/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_PARSER_H
#define OHOS_HDI_PARSER_H

#include <memory>
#include <set>
#include <vector>

#include "ast/ast.h"
#include "ast/ast_attribute.h"
#include "ast/ast_interface_type.h"
#include "ast/ast_method.h"
#include "ast/ast_type.h"
#include "lexer/lexer.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/options.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
using AttrSet = std::set<Token, TokenTypeCompare>;

class Parser {
public:
    Parser() = default;

    ~Parser() = default;

    bool Parse(const std::vector<String> &sourceFiles);

    using StrAstMap = std::unordered_map<String, AutoPtr<AST>, StringHashFunc, StringEqualFunc>;
    inline const StrAstMap &GetAllAst() const
    {
        return allAsts_;
    }

private:
    class Attribute : public LightRefCountBase {
    public:
        bool isOneWay = false;
        bool isCallback = false;
        bool isFull = false;
        bool isLite = false;
    };

    bool ParseOne(const String &sourceFile);

    bool Reset(const String &sourceFile);

    bool ParseFile();

    String ParseLicense();

    bool ParsePackage();

    bool ParserPackageInfo(const String &packageFullName);

    bool ParseImports();

    void ParseImportInfo();

    void ParseSequenceableInfo();

    bool ParseTypeDecls();

    // parse attributes of type
    void ParseAttribute();

    AttrSet ParseAttributeInfo();

    bool AprseAttrUnit(AttrSet &attrs);

    // parse interface type
    void ParseInterface(const AttrSet &attrs = {});

    AutoPtr<ASTInfAttr> ParseInfAttrInfo(const AttrSet &attrs);

    void ParseInterfaceBody(const AutoPtr<ASTInterfaceType> &interface);

    AutoPtr<ASTMethod> ParseMethod();

    AutoPtr<ASTMethodAttr> ParseMethodAttr();

    AutoPtr<ASTMethod> CreateGetVersionMethod();

    void ParseMethodParamList(const AutoPtr<ASTMethod> &method);

    AutoPtr<ASTParameter> ParseParam();

    AutoPtr<ASTParamAttr> ParseParamAttr();

    // parse type
    AutoPtr<ASTType> ParseType();

    AutoPtr<ASTType> ParseUnsignedType();

    AutoPtr<ASTType> ParseArrayType(const AutoPtr<ASTType> &elementType);

    AutoPtr<ASTType> ParseListType();

    AutoPtr<ASTType> ParseMapType();

    AutoPtr<ASTType> ParseSmqType();

    AutoPtr<ASTType> ParseUserDefType();

    // parse declaration of enum
    void ParseEnumDeclaration(const AttrSet &attrs = {});

    AutoPtr<ASTType> ParseEnumBaseType();

    void ParserEnumMember(const AutoPtr<ASTEnumType> &enumType);

    // parse declaration of struct
    void ParseStructDeclaration(const AttrSet &attrs = {});

    void ParseStructMember(const AutoPtr<ASTStructType> &structType);

    // parse declaration of union
    void ParseUnionDeclaration(const AttrSet &attrs = {});

    void ParseUnionMember(const AutoPtr<ASTUnionType> &unionType);

    bool AddUnionMember(const AutoPtr<ASTUnionType> &unionType, const AutoPtr<ASTType> &type, const String &name);

    AutoPtr<ASTTypeAttr> ParseUserDefTypeAttr(const AttrSet &attrs);

    // parse expression
    AutoPtr<ASTExpr> ParseExpr();

    AutoPtr<ASTExpr> ParseAndExpr();

    AutoPtr<ASTExpr> ParseXorExpr();

    AutoPtr<ASTExpr> ParseOrExpr();

    AutoPtr<ASTExpr> ParseShiftExpr();

    AutoPtr<ASTExpr> ParseAddExpr();

    AutoPtr<ASTExpr> ParseMulExpr();

    AutoPtr<ASTExpr> ParseUnaryExpr();

    AutoPtr<ASTExpr> ParsePrimaryExpr();

    AutoPtr<ASTExpr> ParseNumExpr();

    bool CheckType(const Token &token, const AutoPtr<ASTType> &type);

    void SetAstFileType();

    bool CheckIntegrity();

    bool CheckInterfaceAst();

    bool CheckCallbackAst();

    bool CheckPackageName(const String &filePath, const String &packageName);

    bool CheckImport(const String &importName);

    inline static bool IsPrimitiveType(Token token)
    {
        return token.kind_ >= TokenType::BOOLEAN && token.kind_ <= TokenType::ASHMEM;
    }

    bool AddAst(const AutoPtr<AST> &ast);

    void LogError(const String &message);

    void LogError(const Token &token, const String &message);

    void ShowError();

    static constexpr char *TAG = "Parser";

    Lexer lexer_;
    std::vector<String> errors_;
    AutoPtr<AST> ast_;
    StrAstMap allAsts_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_PARSER_H