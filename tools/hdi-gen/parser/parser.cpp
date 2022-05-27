/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "parser/parser.h"
#include <regex>
#include "ast/ast_array_type.h"
#include "ast/ast_enum_type.h"
#include "ast/ast_list_type.h"
#include "ast/ast_map_type.h"
#include "ast/ast_parameter.h"
#include "ast/ast_sequenceable_type.h"
#include "ast/ast_smq_type.h"
#include "ast/ast_struct_type.h"
#include "ast/ast_union_type.h"
#include "util/logger.h"
#include "util/string_builder.h"

#define RE_DIGIT      "[0-9]+"
#define RE_IDENTIFIER "[a-zA-Z_][a-zA-Z0-9_]*"

#define RE_PACKAGE_NUM             3
#define RE_PACKAGE_INDEX           0
#define RE_PACKAGE_MAJOR_VER_INDEX 1
#define RE_PACKAGE_MINOR_VER_INDEX 2

namespace OHOS {
namespace HDI {
static const std::regex rePackage(RE_IDENTIFIER "(?:\\." RE_IDENTIFIER ")*\\.[V|v]"
                                                "(" RE_DIGIT ")_(" RE_DIGIT ")");
static const std::regex reImport(
    RE_IDENTIFIER "(?:\\." RE_IDENTIFIER ")*\\.[V|v]" RE_DIGIT "_" RE_DIGIT "." RE_IDENTIFIER);

bool Parser::Parse(const std::vector<String> &sourceFiles)
{
    for (const auto &file : sourceFiles) {
        if (!ParseOne(file)) {
            return false;
        }
    }

    return true;
}

bool Parser::ParseOne(const String &sourceFile)
{
    if (!Reset(sourceFile)) {
        return false;
    }

    bool ret = ParseFile();
    ret = CheckIntegrity() && ret;
    ret = AddAst(ast_) && ret;
    if (!ret || !errors_.empty()) {
        ShowError();
        return false;
    }

    return true;
}

bool Parser::Reset(const String &sourceFile)
{
    bool ret = lexer_.Reset(sourceFile);
    if (!ret) {
        Logger::E(TAG, "Fail to open file '%s'.", sourceFile.string());
        return false;
    }

    errors_.clear();
    ast_ = nullptr;
    return true;
}

bool Parser::ParseFile()
{
    ast_ = new AST();
    ast_->SetIdlFile(lexer_.GetFilePath());
    ast_->SetLicense(ParseLicense());

    if (!ParsePackage()) {
        return false;
    }

    if (!ParseImports()) {
        return false;
    }

    if (!ParseTypeDecls()) {
        return false;
    }

    SetAstFileType();
    return true;
}

String Parser::ParseLicense()
{
    Token token = lexer_.PeekToken(false);
    if (token.kind_ == TokenType::COMMENT_BLOCK) {
        lexer_.GetToken(false);
        return token.value_;
    }

    return String("");
}

bool Parser::ParsePackage()
{
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::PACKAGE) {
        LogError(token, String::Format("expected 'package' before '%s' token", token.value_.string()));
        return false;
    }
    lexer_.GetToken();

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected name of package before '%s' token", token.value_.string()));
        lexer_.SkipToken(TokenType::SEMICOLON);
        return false;
    }
    String packageName = token.value_;
    lexer_.GetToken();

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::SEMICOLON) {
        LogError(token, String::Format("expected ';' before '%s' token", token.value_.string()));
        return false;
    }
    lexer_.GetToken();

    if (packageName.IsEmpty()) {
        LogError(String("package name is not expected."));
        return false;
    } else if (!CheckPackageName(lexer_.GetFilePath(), packageName)) {
        LogError(String::Format(
            "package name '%s' does not match file apth '%s'.", packageName.string(), lexer_.GetFilePath().string()));
        return false;
    }

    if (!ParserPackageInfo(packageName)) {
        LogError(String::Format("parse package '%s' infomation failed.", packageName.string()));
        return false;
    }

    return true;
}

bool Parser::ParserPackageInfo(const String &packageName)
{
    std::cmatch result;
    if (!std::regex_match(packageName.string(), result, rePackage)) {
        return false;
    }

    if (result.size() < RE_PACKAGE_NUM) {
        return false;
    }

    ast_->SetPackageName(result.str(RE_PACKAGE_INDEX).c_str());
    size_t majorVersion = std::atoi(result.str(RE_PACKAGE_MAJOR_VER_INDEX).c_str());
    size_t minorVersion = std::atoi(result.str(RE_PACKAGE_MINOR_VER_INDEX).c_str());
    ast_->SetVersion(majorVersion, minorVersion);
    return true;
}

bool Parser::ParseImports()
{
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::IMPORT || token.kind_ == TokenType::SEQ) {
        TokenType kind = token.kind_;
        lexer_.GetToken();

        token = lexer_.PeekToken();
        if (token.kind_ != TokenType::ID) {
            LogError(token, String::Format("expected identifier before '%s' token", token.value_.string()));
            lexer_.SkipToken(TokenType::SEMICOLON);
            token = lexer_.PeekToken();
            continue;
        }

        if (kind == TokenType::IMPORT) {
            ParseImportInfo();
        } else {
            ParseSequenceableInfo();
        }
        lexer_.GetToken();

        token = lexer_.PeekToken();
        if (token.kind_ != TokenType::SEMICOLON) {
            LogError(token, String::Format("expected ';' before '%s'.", token.value_.string()));
            return false;
        }
        lexer_.GetToken();

        token = lexer_.PeekToken();
    }

    return true;
}

void Parser::ParseImportInfo()
{
    Token token = lexer_.PeekToken();
    String importName = token.value_;
    if (importName.IsEmpty()) {
        LogError(token, String::Format("import name is empty"));
        return;
    }

    if (!CheckImport(importName)) {
        LogError(token, String::Format("import name is illegal"));
        return;
    }

    auto iter = allAsts_.find(importName);
    AutoPtr<AST> importAst = (iter != allAsts_.end()) ? iter->second : nullptr;
    if (importAst == nullptr) {
        LogError(token, String::Format("can not find idl file from import name '%s'", importName.string()));
        return;
    }

    AutoPtr<ASTInterfaceType> interfaceType = importAst->GetInterfaceDef();
    if (interfaceType != nullptr) {
        interfaceType->SetSerializable(true);
    }

    if (!ast_->AddImport(importAst)) {
        LogError(token, String::Format("multiple import of '%s'", importName.string()));
        return;
    }
}

void Parser::ParseSequenceableInfo()
{
    Token token = lexer_.PeekToken();
    String seqName = token.value_;
    if (seqName.IsEmpty()) {
        LogError(token, String::Format("sequenceable name is empty"));
        return;
    }

    AutoPtr<ASTSequenceableType> seqType = new ASTSequenceableType();
    int index = seqName.LastIndexOf('.');
    if (index != -1) {
        seqType->SetName(seqName.Substring(index + 1));
        seqType->SetNamespace(ast_->ParseNamespace(seqName.Substring(0, index + 1)));
    } else {
        seqType->SetName(seqName);
    }

    AutoPtr<AST> seqAst = new AST();
    seqAst->SetFullName(seqName);
    seqAst->AddSequenceableDef(seqType);
    seqAst->SetAStFileType(ASTFileType::AST_SEQUENCEABLE);
    ast_->AddImport(seqAst);
    AddAst(seqAst);
}

bool Parser::ParseTypeDecls()
{
    Token token = lexer_.PeekToken();
    while (token.kind_ != TokenType::END_OF_FILE) {
        switch (token.kind_) {
            case TokenType::BRACKETS_LEFT:
                ParseAttribute();
                break;
            case TokenType::INTERFACE:
                ParseInterface();
                break;
            case TokenType::ENUM:
                ParseEnumDeclaration();
                break;
            case TokenType::STRUCT:
                ParseStructDeclaration();
                break;
            case TokenType::UNION:
                ParseUnionDeclaration();
                break;
            default:
                LogError(token, String::Format("'%s' is not expected", token.value_.string()));
                lexer_.SkipToken(TokenType::SEMICOLON);
                break;
        }
        token = lexer_.PeekToken();
    }
    return true;
}

void Parser::ParseAttribute()
{
    AttrSet attrs = ParseAttributeInfo();
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::INTERFACE:
            ParseInterface(attrs);
            break;
        case TokenType::ENUM:
            ParseEnumDeclaration(attrs);
            break;
        case TokenType::STRUCT:
            ParseStructDeclaration(attrs);
            break;
        case TokenType::UNION:
            ParseUnionDeclaration(attrs);
            break;
        default:
            LogError(token, String::Format("'%s' is not expected", token.value_.string()));
            lexer_.SkipToken(token.kind_);
            break;
    }
}

AttrSet Parser::ParseAttributeInfo()
{
    AttrSet attrs;
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACKETS_LEFT) {
        LogError(token, String::Format("expected '[' before '%s' token", token.value_.string()));
        lexer_.SkipToken(token.kind_);
        return attrs;
    }
    lexer_.GetToken();

    token = lexer_.PeekToken();
    while (token.kind_ != TokenType::BRACKETS_RIGHT && token.kind_ != TokenType::END_OF_FILE) {
        if (!AprseAttrUnit(attrs)) {
            return attrs;
        }
        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::COMMA) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            continue;
        }

        if (token.kind_ == TokenType::BRACKETS_RIGHT) {
            lexer_.GetToken();
            break;
        } else {
            LogError(token, String::Format("expected ',' or ']' before '%s' token", token.value_.string()));
            lexer_.SkipToken(TokenType::BRACKETS_RIGHT);
            break;
        }
    }

    return attrs;
}

bool Parser::AprseAttrUnit(AttrSet &attrs)
{
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::FULL:
        case TokenType::LITE:
        case TokenType::CALLBACK:
        case TokenType::ONEWAY: {
            if (attrs.find(token) != attrs.end()) {
                LogError(token, String::Format("Duplicate declared attributes '%s'", token.value_.string()));
            } else {
                attrs.insert(token);
            }
            lexer_.GetToken();
            break;
        }
        default:
            LogError(token, String::Format("'%s' is a illegal attribute", token.value_.string()));
            lexer_.SkipToken(TokenType::BRACKETS_RIGHT);
            return false;
    }
    return true;
}

void Parser::ParseInterface(const AttrSet &attrs)
{
    AutoPtr<ASTInterfaceType> interfaceType = new ASTInterfaceType;
    AutoPtr<ASTInfAttr> astAttr = ParseInfAttrInfo(attrs);
    interfaceType->SetAttribute(astAttr);

    lexer_.GetToken();
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected interface name before '%s' token", token.value_.string()));
    } else {
        interfaceType->SetName(token.value_);
        interfaceType->SetNamespace(ast_->ParseNamespace(ast_->GetFullName()));
        interfaceType->SetLicense(ast_->GetLicense());
        if (!token.value_.Equals(ast_->GetName())) {
            LogError(token, String::Format("interface name '%s' is not equal idl file name", token.value_.string()));
        }
        lexer_.GetToken();
    }

    ParseInterfaceBody(interfaceType);
    ast_->AddInterfaceDef(interfaceType);
}

AutoPtr<ASTInfAttr> Parser::ParseInfAttrInfo(const AttrSet &attrs)
{
    AutoPtr<ASTInfAttr> infAttr = new ASTInfAttr;

    for (const auto &attr : attrs) {
        switch (attr.kind_) {
            case TokenType::FULL:
                infAttr->isFull_ = true;
                break;
            case TokenType::LITE:
                infAttr->isLite_ = true;
                break;
            case TokenType::CALLBACK:
                infAttr->isCallback_ = true;
                break;
            case TokenType::ONEWAY:
                infAttr->isOneWay_ = true;
                break;
            default:
                LogError(attr, String::Format("illegal attribute of interface"));
                break;
        }
    }

    return infAttr;
}

void Parser::ParseInterfaceBody(const AutoPtr<ASTInterfaceType> &interface)
{
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_LEFT) {
        LogError(token, String::Format("expected '{' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    while (token.kind_ != TokenType::BRACES_RIGHT && token.kind_ != TokenType::END_OF_FILE) {
        interface->AddMethod(ParseMethod());
        token = lexer_.PeekToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_RIGHT) {
        LogError(token, String::Format("expected '{' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    interface->AddVersionMethod(CreateGetVersionMethod());
}

AutoPtr<ASTMethod> Parser::ParseMethod()
{
    AutoPtr<ASTMethod> method = new ASTMethod();
    method->SetAttribute(ParseMethodAttr());

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected method name before '%s' token", token.value_.string()));
    } else {
        method->SetName(token.value_);
        lexer_.GetToken();
    }

    ParseMethodParamList(method);

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::SEMICOLON) {
        LogError(token, String::Format("expected ';' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    return method;
}

AutoPtr<ASTMethodAttr> Parser::ParseMethodAttr()
{
    AutoPtr<ASTMethodAttr> attr = new ASTMethodAttr();
    Token token = lexer_.PeekToken();
    if (token.kind_ == TokenType::ID) {
        return attr;
    }

    if (token.kind_ != TokenType::BRACKETS_LEFT) {
        LogError(token, String::Format("expected '[' before '%s' token", token.value_.string()));
        lexer_.SkipUntilToken(TokenType::ID);
        return attr;
    }

    lexer_.GetToken();
    token = lexer_.PeekToken();
    while (token.kind_ != TokenType::BRACKETS_RIGHT) {
        switch (token.kind_) {
            case TokenType::FULL:
                attr->isFull_ = true;
                break;
            case TokenType::LITE:
                attr->isLite_ = true;
                break;
            case TokenType::ONEWAY:
                attr->isOneWay_ = true;
                break;
            default:
                LogError(token, String::Format("expected attribute before '%s' token", token.value_.string()));
                lexer_.SkipUntilToken(TokenType::BRACKETS_RIGHT);
                return attr;
        }

        lexer_.GetToken();
        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::BRACKETS_RIGHT) {
            lexer_.GetToken();
            break;
        }

        if (token.kind_ != TokenType::COMMA) {
            LogError(token, String::Format("expected ',' before '%s' token", token.value_.string()));
            lexer_.SkipUntilToken(TokenType::BRACKETS_RIGHT);
            return attr;
        }
        lexer_.GetToken();
        token = lexer_.PeekToken();
    }
    return attr;
}

AutoPtr<ASTMethod> Parser::CreateGetVersionMethod()
{
    AutoPtr<ASTMethod> method = new ASTMethod();
    method->SetName("GetVersion");

    AutoPtr<ASTType> type = ast_->FindType("unsigned int");
    if (type == nullptr) {
        type = new ASTUintType();
    }
    AutoPtr<ASTParameter> majorParam = new ASTParameter("majorVer", ParamAttr::PARAM_OUT, type);
    AutoPtr<ASTParameter> minorParam = new ASTParameter("minorVer", ParamAttr::PARAM_OUT, type);

    method->AddParameter(majorParam);
    method->AddParameter(minorParam);
    return method;
}

void Parser::ParseMethodParamList(const AutoPtr<ASTMethod> &method)
{
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::PARENTHESES_LEFT) {
        LogError(token, String::Format("expected '(' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ == TokenType::PARENTHESES_RIGHT) {
        lexer_.GetToken();
        return;
    }

    while (token.kind_ != TokenType::PARENTHESES_RIGHT && token.kind_ != TokenType::END_OF_FILE) {
        method->AddParameter(ParseParam());
        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::COMMA) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            if (token.kind_ == TokenType::PARENTHESES_RIGHT) {
                LogError(token, String::Format(""));
            }
            continue;
        }

        if (token.kind_ == TokenType::PARENTHESES_RIGHT) {
            lexer_.GetToken();
            break;
        } else {
            LogError(token, String::Format("expected ',' or ')' before '%s' token", token.value_.string()));
            lexer_.SkipToken(TokenType::PARENTHESES_RIGHT);
            break;
        }
    }
}

AutoPtr<ASTParameter> Parser::ParseParam()
{
    AutoPtr<ASTParamAttr> paramAttr = ParseParamAttr();
    AutoPtr<ASTType> paramType = ParseType();
    String paramName = "";

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected param name before '%s' token", token.value_.string()));
    } else {
        paramName = token.value_;
        lexer_.GetToken();
    }

    return new ASTParameter(paramName, paramAttr, paramType);
}

AutoPtr<ASTParamAttr> Parser::ParseParamAttr()
{
    AutoPtr<ASTParamAttr> attr = new ASTParamAttr(ParamAttr::PARAM_IN);
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACKETS_LEFT) {
        LogError(token, String::Format("expected '[' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ == TokenType::IN) {
        attr->value_ = ParamAttr::PARAM_IN;
        lexer_.GetToken();
    } else if (token.kind_ == TokenType::OUT) {
        attr->value_ = ParamAttr::PARAM_OUT;
        lexer_.GetToken();
    } else {
        LogError(token, String::Format("expected 'in' or 'out' attribute before '%s' token", token.value_.string()));
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACKETS_RIGHT) {
        LogError(token, String::Format("expected ']' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    return attr;
}

AutoPtr<ASTType> Parser::ParseType()
{
    AutoPtr<ASTType> type = nullptr;
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::BOOLEAN:
        case TokenType::BYTE:
        case TokenType::SHORT:
        case TokenType::INT:
        case TokenType::LONG:
        case TokenType::STRING:
        case TokenType::FLOAT:
        case TokenType::DOUBLE:
        case TokenType::FD:
            type = ast_->FindType(token.value_);
            lexer_.GetToken();
            break;
        case TokenType::UNSIGNED:
            type = ParseUnsignedType();
            break;
        case TokenType::LIST:
            type = ParseListType();
            break;
        case TokenType::MAP:
            type = ParseMapType();
            break;
        case TokenType::SMQ:
            type = ParseSmqType();
        case TokenType::ENUM:
        case TokenType::STRUCT:
        case TokenType::UNION:
        case TokenType::ID:
        case TokenType::SEQ:
            type = ParseUserDefType();
            break;
        default:
            LogError(token, String::Format("'%s' of type is illegal", token.value_.string()));
            return nullptr;
    }
    if (type == nullptr) {
        LogError(token, String::Format("this type was note declared in this scope"));
    }
    if (!CheckType(token, type)) {
        return nullptr;
    }
    if (lexer_.PeekToken().kind_ == TokenType::BRACKETS_LEFT) {
        type = ParseArrayType(type);
    }
    return type;
}

AutoPtr<ASTType> Parser::ParseUnsignedType()
{
    AutoPtr<ASTType> type = nullptr;
    String namePrefix = lexer_.GetToken().value_;
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::CHAR:
        case TokenType::SHORT:
        case TokenType::INT:
        case TokenType::LONG:
            type = ast_->FindType(namePrefix + " " + token.value_);
            lexer_.GetToken();
            break;
        default:
            LogError(token, String::Format("'unsigned %s' was not declared in the idl file", token.value_.string()));
            break;
    }

    return type;
}

AutoPtr<ASTType> Parser::ParseArrayType(const AutoPtr<ASTType> &elementType)
{
    lexer_.GetToken(); // '['

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACKETS_RIGHT) {
        LogError(token, String::Format("expected ']' before '%s' token", token.value_.string()));
        return nullptr;
    }
    lexer_.GetToken(); // ']'

    if (elementType == nullptr) {
        return nullptr;
    }

    AutoPtr<ASTArrayType> arrayType = new ASTArrayType();
    arrayType->SetElementType(elementType);
    AutoPtr<ASTType> type = ast_->FindType(arrayType->ToString());

    if (type == nullptr) {
        ast_->AddType(arrayType.Get());
        type = arrayType.Get();
    }

    return type;
}

AutoPtr<ASTType> Parser::ParseListType()
{
    lexer_.GetToken(); // List

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_LEFT) {
        LogError(token, String::Format("expected '<' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // '<'
    }

    AutoPtr<ASTType> type = ParseType(); // element type
    if (type == nullptr) {
        lexer_.SkipToken(TokenType::ANGLE_BRACKETS_RIGHT);
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_RIGHT) {
        LogError(token, String::Format("expected '>' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // '>'
    }

    AutoPtr<ASTListType> list = new ASTListType();
    list->SetElementType(type);

    AutoPtr<ASTType> ret = ast_->FindType(list->ToString());
    if (ret == nullptr) {
        ast_->AddType(list.Get());
        ret = list.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseMapType()
{
    lexer_.GetToken(); // 'Map'

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_LEFT) {
        LogError(token, String::Format("expected '<' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // '<'
    }

    AutoPtr<ASTType> keyType = ParseType(); // key type
    if (keyType == nullptr) {
        LogError(token, String::Format("key type '%s' is illegal", token.value_.string()));
        lexer_.SkipToken(TokenType::ANGLE_BRACKETS_RIGHT);
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::COMMA) {
        LogError(token, String::Format("expected ',' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // ','
    }

    AutoPtr<ASTType> valueType = ParseType();
    if (valueType == nullptr) {
        LogError(token, String::Format("key type '%s' is illegal", token.value_.string()));
        lexer_.SkipToken(TokenType::ANGLE_BRACKETS_RIGHT);
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_RIGHT) {
        LogError(token, String::Format("expected '>' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    AutoPtr<ASTMapType> map = new ASTMapType();
    map->SetKeyType(keyType);
    map->SetValueType(valueType);

    AutoPtr<ASTType> ret = ast_->FindType(map->ToString());
    if (ret == nullptr) {
        ast_->AddType(map.Get());
        ret = map.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseSmqType()
{
    lexer_.GetToken(); // 'SharedMemQueue'

    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_LEFT) {
        LogError(token, String::Format("expected '<' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // '<'
    }

    AutoPtr<ASTType> InnerType = ParseType();
    if (InnerType == nullptr) {
        lexer_.SkipToken(TokenType::ANGLE_BRACKETS_RIGHT);
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ANGLE_BRACKETS_RIGHT) {
        LogError(token, String::Format("expected '>' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken(); // '>'
    }

    AutoPtr<ASTSmqType> type = new ASTSmqType();
    type->SetInnerType(InnerType);
    AutoPtr<ASTType> ret = ast_->FindType(type->ToString());
    if (ret == nullptr) {
        ast_->AddType(type.Get());
        ret = type.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseUserDefType()
{
    Token token = lexer_.GetToken();
    if (token.kind_ == TokenType::ID) {
        return ast_->FindType(token.value_);
    }

    String typePrefix = token.value_;

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected identifier before '%s' token", token.value_.string()));
        return nullptr;
    } else {
        lexer_.GetToken();
    }

    String typeName = typePrefix + " " + token.value_;
    AutoPtr<ASTType> type = ast_->FindType(typeName);
    if (type != nullptr) {
        ast_->AddType(type);
    }
    return type;
}

void Parser::ParseEnumDeclaration(const AttrSet &attrs)
{
    AutoPtr<ASTEnumType> enumType = new ASTEnumType;
    enumType->SetAttribute(ParseUserDefTypeAttr(attrs));

    lexer_.GetToken();
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected enum type name before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
        enumType->SetName(token.value_);
    }

    token = lexer_.PeekToken();
    if (token.kind_ == TokenType::COLON || token.kind_ == TokenType::BRACES_LEFT) {
        enumType->SetBaseType(ParseEnumBaseType());
    } else {
        LogError(token, String::Format("expected ':' or '{' before '%s' token", token.value_.string()));
    }

    ParserEnumMember(enumType);
    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_RIGHT) {
        LogError(token, String::Format("expected '}' before '%s' token", token.value_.string()));
        return;
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::SEMICOLON) {
        LogError(token, String::Format("expected ';' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    ast_->AddTypeDefinition(enumType.Get());
}

AutoPtr<ASTType> Parser::ParseEnumBaseType()
{
    AutoPtr<ASTType> baseType = nullptr;
    Token token = lexer_.PeekToken();
    if (token.kind_ == TokenType::COLON) {
        lexer_.GetToken();

        token = lexer_.PeekToken();
        baseType = ParseType();
        if (baseType == nullptr) {
            return nullptr;
        }

        switch (baseType->GetTypeKind()) {
            case TypeKind::TYPE_BYTE:
            case TypeKind::TYPE_SHORT:
            case TypeKind::TYPE_INT:
            case TypeKind::TYPE_LONG:
            case TypeKind::TYPE_UCHAR:
            case TypeKind::TYPE_USHORT:
            case TypeKind::TYPE_UINT:
            case TypeKind::TYPE_ULONG:
                break;
            default:
                LogError(token, String::Format("illegal base type of enum", baseType->ToString().string()));
                return nullptr;
        }

        token = lexer_.PeekToken();
        if (token.kind_ != TokenType::BRACES_LEFT) {
            LogError(token, String::Format("expected '{' before '%s' token", token.value_.string()));
        }
        lexer_.GetToken();
    } else {
        lexer_.GetToken();
        AutoPtr<ASTType> baseType = ast_->FindType("int");
    }
    return baseType;
}

void Parser::ParserEnumMember(const AutoPtr<ASTEnumType> &enumType)
{
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::ID) {
        AutoPtr<ASTEnumValue> enumValue = new ASTEnumValue(token.value_);
        lexer_.GetToken();

        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::ASSIGN) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            enumValue->SetExprValue(ParseExpr());
        }

        enumValue->SetType(enumType->GetBaseType());
        enumType->AddMember(enumValue);

        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::COMMA) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            continue;
        }

        if (token.kind_ != TokenType::BRACES_RIGHT) {
            LogError(token, String::Format("expected ',' or '}' before '%s' token", token.value_.string()));
        }
    }
}

void Parser::ParseStructDeclaration(const AttrSet &attrs)
{
    AutoPtr<ASTStructType> structType = new ASTStructType;
    structType->SetAttribute(ParseUserDefTypeAttr(attrs));

    lexer_.GetToken();
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected struct name before '%s' token", token.value_.string()));
    } else {
        structType->SetName(token.value_);
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_LEFT) {
        LogError(token, String::Format("expected '{' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    ParseStructMember(structType);

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_RIGHT) {
        LogError(token, String::Format("expected '}' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::SEMICOLON) {
        LogError(token, String::Format("expected ';' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    ast_->AddTypeDefinition(structType.Get());
}

void Parser::ParseStructMember(const AutoPtr<ASTStructType> &structType)
{
    Token token = lexer_.PeekToken();
    while (token.kind_ != TokenType::BRACES_RIGHT && token.kind_ != TokenType::END_OF_FILE) {
        AutoPtr<ASTType> memberType = ParseType();
        if (memberType == nullptr) {
            lexer_.SkipToken(TokenType::SEMICOLON);
            token = lexer_.PeekToken();
            continue;
        }

        String typeName = memberType->ToString();
        token = lexer_.PeekToken();
        if (token.kind_ != TokenType::ID) {
            LogError(token, String::Format("expected member name before '%s' token", token.value_.string()));
            lexer_.SkipToken(TokenType::SEMICOLON);
            token = lexer_.PeekToken();
            continue;
        }

        lexer_.GetToken();
        String memberName = token.value_;
        structType->AddMember(memberType, memberName);

        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::SEMICOLON) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            continue;
        }

        if (token.kind_ != TokenType::BRACES_RIGHT) {
            LogError(token, String::Format("expected ',' or '}' before '%s' token", token.value_.string()));
        }
    }
}

void Parser::ParseUnionDeclaration(const AttrSet &attrs)
{
    AutoPtr<ASTUnionType> unionType = new ASTUnionType;
    unionType->SetAttribute(ParseUserDefTypeAttr(attrs));

    lexer_.GetToken();
    Token token = lexer_.PeekToken();
    if (token.kind_ != TokenType::ID) {
        LogError(token, String::Format("expected struct name before '%s' token", token.value_.string()));
    } else {
        unionType->SetName(token.value_);
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_LEFT) {
        LogError(token, String::Format("expected '{' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    ParseUnionMember(unionType);

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::BRACES_RIGHT) {
        LogError(token, String::Format("expected '}' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    token = lexer_.PeekToken();
    if (token.kind_ != TokenType::SEMICOLON) {
        LogError(token, String::Format("expected ';' before '%s' token", token.value_.string()));
    } else {
        lexer_.GetToken();
    }

    ast_->AddTypeDefinition(unionType.Get());
}

void Parser::ParseUnionMember(const AutoPtr<ASTUnionType> &unionType)
{
    Token token = lexer_.PeekToken();
    while (token.kind_ != TokenType::BRACES_RIGHT && token.kind_ != TokenType::END_OF_FILE) {
        AutoPtr<ASTType> memberType = ParseType();
        if (memberType == nullptr) {
            lexer_.SkipToken(TokenType::SEMICOLON);
            token = lexer_.PeekToken();
            continue;
        }

        String typeName = memberType->ToString();
        token = lexer_.PeekToken();
        if (token.kind_ != TokenType::ID) {
            LogError(token, String::Format("expected member name before '%s' token", token.value_.string()));
            lexer_.SkipToken(TokenType::SEMICOLON);
            token = lexer_.PeekToken();
            continue;
        }
        lexer_.GetToken();

        String memberName = token.value_;
        if (!AddUnionMember(unionType, memberType, memberName)) {
            LogError(token,
                String::Format("union not support this type or name of member duplicate '%s'", token.value_.string()));
        }

        token = lexer_.PeekToken();
        if (token.kind_ == TokenType::SEMICOLON) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            continue;
        }

        if (token.kind_ != TokenType::BRACES_RIGHT) {
            LogError(token, String::Format("expected ',' or '}' before '%s' token", token.value_.string()));
        }
    }
}

bool Parser::AddUnionMember(const AutoPtr<ASTUnionType> &unionType, const AutoPtr<ASTType> &type, const String &name)
{
    for (size_t i = 0; i < unionType->GetMemberNumber(); i++) {
        String memberName = unionType->GetMemberName(i);
        if (name.Equals(memberName)) {
            return false;
        }
    }

    if ((type->GetTypeKind() < TypeKind::TYPE_BOOLEAN || type->GetTypeKind() > TypeKind::TYPE_ULONG) &&
        (type->GetTypeKind() < TypeKind::TYPE_ENUM || type->GetTypeKind() > TypeKind::TYPE_UNION)) {
        return false;
    }

    unionType->AddMember(type, name);
    return true;
}

AutoPtr<ASTTypeAttr> Parser::ParseUserDefTypeAttr(const AttrSet &attrs)
{
    AutoPtr<ASTTypeAttr> attribute = new ASTTypeAttr();
    for (const auto &token : attrs) {
        switch (token.kind_) {
            case TokenType::FULL:
                attribute->isFull_ = true;
                break;
            case TokenType::LITE:
                attribute->isLite_ = true;
                break;
            default:
                LogError(token, String::Format("invalid attribute '%s' for type decl", token.value_.string()));
                break;
        }
    }

    return attribute;
}

AutoPtr<ASTExpr> Parser::ParseExpr()
{
    return ParseAndExpr();
}

AutoPtr<ASTExpr> Parser::ParseAndExpr()
{
    AutoPtr<ASTExpr> left = ParseXorExpr();
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::AND) {
        lexer_.GetToken();
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = BinaryOpKind::AND;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseXorExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseXorExpr()
{
    AutoPtr<ASTExpr> left = ParseOrExpr();
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::XOR) {
        lexer_.GetToken();
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = BinaryOpKind::XOR;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseOrExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseOrExpr()
{
    AutoPtr<ASTExpr> left = ParseShiftExpr();
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::OR) {
        lexer_.GetToken();
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = BinaryOpKind::OR;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseShiftExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseShiftExpr()
{
    AutoPtr<ASTExpr> left = ParseAddExpr();
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::LEFT_SHIFT || token.kind_ == TokenType::RIGHT_SHIFT) {
        lexer_.GetToken();
        BinaryOpKind op = (token.kind_ == TokenType::LEFT_SHIFT) ? BinaryOpKind::LSHIFT : BinaryOpKind::RSHIFT;
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = op;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseAddExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseAddExpr()
{
    AutoPtr<ASTExpr> left = ParseMulExpr();
    Token token = lexer_.PeekToken();
    while (token.kind_ == TokenType::ADD || token.kind_ == TokenType::SUB) {
        lexer_.GetToken();
        BinaryOpKind op = (token.kind_ == TokenType::ADD) ? BinaryOpKind::ADD : BinaryOpKind::SUB;
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = op;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseMulExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseMulExpr()
{
    AutoPtr<ASTExpr> left = ParseUnaryExpr();
    Token token = lexer_.PeekToken();
    while (
        token.kind_ == TokenType::STAR || token.kind_ == TokenType::SLASH || token.kind_ == TokenType::PERCENT_SIGN) {
        lexer_.GetToken();
        BinaryOpKind op = BinaryOpKind::MUL;
        if (token.kind_ == TokenType::SLASH) {
            op = BinaryOpKind::DIV;
        } else if (token.kind_ == TokenType::PERCENT_SIGN) {
            op = BinaryOpKind::MOD;
        }
        AutoPtr<ASTBinaryExpr> expr = new ASTBinaryExpr;
        expr->op_ = op;
        expr->lExpr_ = left;
        expr->rExpr_ = ParseUnaryExpr();

        left = expr.Get();
        token = lexer_.PeekToken();
    }
    return left;
}

AutoPtr<ASTExpr> Parser::ParseUnaryExpr()
{
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::ADD:
        case TokenType::SUB:
        case TokenType::TILDE: {
            lexer_.GetToken();
            AutoPtr<ASTUnaryExpr> expr = new ASTUnaryExpr;
            expr->op_ = UnaryOpKind::PLUS;
            if (token.kind_ == TokenType::SUB) {
                expr->op_ = UnaryOpKind::MINUS;
            } else if (token.kind_ == TokenType::TILDE) {
                expr->op_ = UnaryOpKind::TILDE;
            }

            expr->expr_ = ParseUnaryExpr();
            return expr.Get();
        }
        default:
            return ParsePrimaryExpr();
    }
}

AutoPtr<ASTExpr> Parser::ParsePrimaryExpr()
{
    Token token = lexer_.PeekToken();
    switch (token.kind_) {
        case TokenType::PARENTHESES_LEFT: {
            lexer_.GetToken();
            AutoPtr<ASTExpr> expr = ParseExpr();
            token = lexer_.PeekToken();
            if (token.kind_ != TokenType::PARENTHESES_RIGHT) {
                LogError(token, String::Format("expected ')' before %s token", token.value_.string()));
            } else {
                lexer_.GetToken();
                expr->isParenExpr = true;
            }
            return expr;
        }
        case TokenType::NUM:
            return ParseNumExpr();
        default:
            LogError(token, String::Format("this expression is not supported"));
            lexer_.SkipUntilToken(TokenType::COMMA);
            return nullptr;
    }
}

AutoPtr<ASTExpr> Parser::ParseNumExpr()
{
    Token token = lexer_.GetToken();
    AutoPtr<ASTNumExpr> expr = new ASTNumExpr;
    expr->value_ = token.value_;
    return expr.Get();
}

bool Parser::CheckType(const Token &token, const AutoPtr<ASTType> &type)
{
    if (type == nullptr) {
        return false;
    }

    if (Options::GetInstance().GetTargetLanguage().Equals("c")) {
        if (type->IsSequenceableType()) {
            LogError(token, String::Format("The sequenceable type is not supported by c language."));
            return false;
        }

        if (type->IsSmqType()) {
            LogError(token, String::Format("The smq type is not supported by c language."));
            return false;
        }

        if (Options::GetInstance().DoGenerateKernelCode()) {
            switch (type->GetTypeKind()) {
                case TypeKind::TYPE_FLOAT:
                case TypeKind::TYPE_DOUBLE:
                case TypeKind::TYPE_FILEDESCRIPTOR:
                case TypeKind::TYPE_INTERFACE:
                    LogError(token,
                        String::Format("The '%s' type is not supported by c language.", type->ToString().string()));
                    break;
                default:
                    break;
            }
        }
    } else if (Options::GetInstance().GetTargetLanguage().Equals("java")) {
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_UCHAR:
            case TypeKind::TYPE_USHORT:
            case TypeKind::TYPE_UINT:
            case TypeKind::TYPE_ULONG:
            case TypeKind::TYPE_ENUM:
            case TypeKind::TYPE_STRUCT:
            case TypeKind::TYPE_UNION:
            case TypeKind::TYPE_SMQ:
            case TypeKind::TYPE_UNKNOWN:
                LogError(token,
                    String::Format("The '%s' type is not supported by java language.", type->ToString().string()));
                return false;
            default:
                break;
        }
    }

    return true;
}

void Parser::SetAstFileType()
{
    if (ast_->GetInterfaceDef() != nullptr) {
        if (ast_->GetInterfaceDef()->IsCallback()) {
            ast_->SetAStFileType(ASTFileType::AST_ICALLBACK);
        } else {
            ast_->SetAStFileType(ASTFileType::AST_IFACE);
        }
    } else {
        ast_->SetAStFileType(ASTFileType::AST_TYPES);
    }
}

bool Parser::CheckIntegrity()
{
    if (ast_ == nullptr) {
        LogError(String("ast is nullptr."));
        return false;
    }

    if (ast_->GetName().IsEmpty()) {
        LogError(String("ast's name is empty."));
        return false;
    }

    if (ast_->GetPackageName().IsEmpty()) {
        LogError(String("ast's package name is empty."));
        return false;
    }

    switch (ast_->GetASTFileType()) {
        case ASTFileType::AST_IFACE: {
            return CheckInterfaceAst();
        }
        case ASTFileType::AST_ICALLBACK: {
            return CheckCallbackAst();
        }
        case ASTFileType::AST_SEQUENCEABLE: {
            LogError(String("it's impossible that ast is sequenceable."));
            return false;
        }
        case ASTFileType::AST_TYPES: {
            if (ast_->GetInterfaceDef() != nullptr) {
                LogError(String("custom ast cannot has interface."));
                return false;
            }
            break;
        }
        default:
            break;
    }

    return true;
}

bool Parser::CheckInterfaceAst()
{
    AutoPtr<ASTInterfaceType> interface = ast_->GetInterfaceDef();
    if (interface == nullptr) {
        LogError(String("ast's interface is empty."));
        return false;
    }

    if (ast_->GetTypeDefinitionNumber() > 0) {
        LogError(String("interface ast cannot has custom types."));
        return false;
    }

    if (interface->GetMethodNumber() == 0) {
        LogError(String("interface ast has no method."));
        return false;
    }
    return true;
}

bool Parser::CheckCallbackAst()
{
    AutoPtr<ASTInterfaceType> interface = ast_->GetInterfaceDef();
    if (interface == nullptr) {
        LogError(String("ast's interface is empty."));
        return false;
    }

    if (!interface->IsCallback()) {
        LogError(String("ast is callback, but ast's interface is not callback."));
        return false;
    }
    return true;
}

/*
 * For example
 * filePath: ./ohos/interface/foo/v1_0/IFoo.idl
 * package OHOS.Hdi.foo.v1_0;
 */
bool Parser::CheckPackageName(const String &filePath, const String &packageName)
{
    String pkgToPath = Options::GetInstance().GetPackagePath(packageName);

    int index = filePath.LastIndexOf(File::separator);
    if (index == -1) {
        return false;
    }

    String parentDir = filePath.Substring(0, index);
    return parentDir.Equals(pkgToPath);
}

bool Parser::CheckImport(const String &importName)
{
    if (!std::regex_match(importName.string(), reImport)) {
        LogError(String::Format("invalid impirt name '%s'", importName.string()));
        return false;
    }

    String idlFilePath = Options::GetInstance().GetImportFilePath(importName);
    if (!File::CheckValid(idlFilePath)) {
        LogError(String::Format("can not import '%s'", idlFilePath.string()));
        return false;
    }
    return true;
}

bool Parser::AddAst(const AutoPtr<AST> &ast)
{
    if (ast == nullptr) {
        LogError(String("ast is nullptr."));
        return false;
    }

    allAsts_[ast->GetFullName()] = ast;
    return true;
}

void Parser::LogError(const String &message)
{
    errors_.push_back(message);
}

void Parser::LogError(const Token &token, const String &message)
{
    errors_.push_back(String::Format("[%s] error:%s", LocInfo(token).string(), message.string()));
}

void Parser::ShowError()
{
    for (const auto &errMsg : errors_) {
        Logger::E(TAG, "%s", errMsg.string());
    }
}
} // namespace HDI
} // namespace OHOS