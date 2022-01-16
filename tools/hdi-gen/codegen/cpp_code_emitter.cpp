/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_code_emitter.h"
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>
#include "util/options.h"

namespace OHOS {
namespace HDI {
String CppCodeEmitter::FileName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String subName = Options::GetInstance().GetSubPackage(name);
    StringBuilder sb;
    for (int i = 0; i < subName.GetLength(); i++) {
        char c = subName[i];
        if (isupper(c) != 0) {
            // 2->Index of the last char array.
            if (i > 1 && subName[i - 1] != '.' && subName[i - 2] != '.') {
                sb.Append('_');
            }
            sb.Append(tolower(c));
        } else {
            sb.Append(c);
        }
    }

    return sb.ToString().Replace('.', '/');
}

String CppCodeEmitter::EmitMethodCmdID(const AutoPtr<ASTMethod>& method)
{
    return String::Format("CMD_%s_%s", infName_.ToUnderLineUpper().string(),
        method->GetName().ToUnderLineUpper().string());
}

void CppCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("enum {\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + g_tab).Append(EmitMethodCmdID(method)).Append(",\n");
    }

    sb.Append(g_tab).Append(EmitMethodCmdID(interface_->GetVersionMethod())).Append(",\n");
    sb.Append(prefix).Append("};\n");
}

void CppCodeEmitter::GetStdlibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    bool includeString = false;
    bool includeList = false;
    bool includeMap = false;
    bool includeSmq = false;

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_STRING: {
                if (!includeString) {
                    headerFiles.emplace(HeaderFile(HeaderFileType::CPP_STD_HEADER_FILE, "string"));
                    includeString = true;
                }
                break;
            }
            case TypeKind::TYPE_ARRAY:
            case TypeKind::TYPE_LIST: {
                if (!includeList) {
                    headerFiles.emplace(HeaderFile(HeaderFileType::CPP_STD_HEADER_FILE, "vector"));
                    includeList = true;
                }
                break;
            }
            case TypeKind::TYPE_MAP: {
                if (!includeMap) {
                    headerFiles.emplace(HeaderFile(HeaderFileType::CPP_STD_HEADER_FILE, "map"));
                    includeMap = true;
                }
                break;
            }
            case TypeKind::TYPE_SMQ: {
                if (!includeSmq) {
                    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdi_smq"));
                    includeSmq = true;
                }
            }
            default:
                break;
        }
    }
}

void CppCodeEmitter::GetImportInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;
        String fileName = FileName(importAst->GetFullName());
        headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(importAst->GetFullName())));
    }
}

void CppCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat(param->EmitCppParameter());
}

void CppCodeEmitter::EmitLicense(StringBuilder& sb)
{
    if (ast_->GetLicense().IsEmpty()) {
        return;
    }
    sb.Append(ast_->GetLicense()).Append("\n\n");
}

void CppCodeEmitter::EmitHeadMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#ifndef ").Append(macroName).Append("\n");
    sb.Append("#define ").Append(macroName).Append("\n");
}

void CppCodeEmitter::EmitTailMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#endif // ").Append(macroName);
}

void CppCodeEmitter::EmitHeadExternC(StringBuilder& sb)
{
    sb.Append("#ifdef __cplusplus\n");
    sb.Append("extern \"C\" {\n");
    sb.Append("#endif /* __cplusplus */\n");
}

void CppCodeEmitter::EmitTailExternC(StringBuilder& sb)
{
    sb.Append("#ifdef __cplusplus\n");
    sb.Append("}\n");
    sb.Append("#endif /* __cplusplus */\n");
}

void CppCodeEmitter::EmitBeginNamespace(StringBuilder& sb)
{
    String nspace = interface_->GetNamespace()->ToString();
    int index = nspace.IndexOf('.');
    while (index != -1) {
        sb.AppendFormat("namespace %s {\n", nspace.Substring(0, index).string());
        nspace = nspace.Substring(index + 1);
        index = nspace.IndexOf('.');
    }
}

void CppCodeEmitter::EmitEndNamespace(StringBuilder& sb)
{
    String nspace = interface_->GetNamespace()->ToString();
    nspace = nspace.Substring(0, nspace.GetLength() - 1);
    while (!nspace.IsEmpty()) {
        int index = nspace.LastIndexOf('.');
        sb.AppendFormat("} // %s\n",
            (index != -1) ? nspace.Substring(index + 1, nspace.GetLength()).string() : nspace.string());
        nspace = nspace.Substring(0, index);
    }
}

void CppCodeEmitter::EmitUsingNamespace(StringBuilder& sb)
{
    sb.Append("using namespace OHOS;\n");
    EmitImportUsingNamespace(sb);
}

void CppCodeEmitter::EmitImportUsingNamespace(StringBuilder& sb)
{
    using StringSet = std::unordered_set<String, StringHashFunc, StringEqualFunc>;
    StringSet namespaceSet;
    String selfNameSpace = CppNameSpace(ast_->GetFullName());

    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> import = importPair.second;
        String nameSpace = CppNameSpace(import->GetFullName());
        if (nameSpace.Equals(selfNameSpace)) {
            continue;
        }
        namespaceSet.emplace(nameSpace);
    }

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_SMQ) {
            namespaceSet.emplace("OHOS::HDI::Base");
            break;
        }
    }

    for (const auto& nspace : namespaceSet) {
        sb.Append("using namespace ").AppendFormat("%s;\n", nspace.string());
    }
}

void CppCodeEmitter::EmitWriteMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    type->EmitCppWriteVar(parcelName, param->GetName(), sb, prefix);
}

void CppCodeEmitter::EmitReadMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    bool initVariable, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    type->EmitCppReadVar(parcelName, param->GetName(), sb, prefix, initVariable);
}

void CppCodeEmitter::EmitLocalVariable(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppLocalVar()).Append("\n");
}

String CppCodeEmitter::MacroName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String macro = name.Replace('.', '_').ToUpperCase() + "_H";
    return macro;
}

String CppCodeEmitter::CppFullName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    return name.Replace(".", "::");
}

String CppCodeEmitter::ConstantName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            if (i > 1) {
                sb.Append('_');
            }
            sb.Append(c);
        } else {
            sb.Append(toupper(c));
        }
    }

    return sb.ToString();
}

String CppCodeEmitter::CppNameSpace(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String space;
    int lastIndex = name.LastIndexOf('.');
    if (lastIndex != -1) {
        space = name.Substring(0, lastIndex);
    } else {
        space = name;
    }

    return CppFullName(space);
}

String CppCodeEmitter::SpecificationParam(StringBuilder& paramSb, const String& prefix)
{
    int maxLineLen = 120;
    int replaceLen = 2;
    String paramStr = paramSb.ToString();
    int preIndex = 0;
    int curIndex = 0;

    String insertStr = String::Format("\n%s", prefix.string());
    for (; curIndex < paramStr.GetLength(); curIndex++) {
        if (curIndex == maxLineLen && preIndex > 0) {
            paramStr.Replace(preIndex, replaceLen, ",");
            paramStr.insert(preIndex + 1, insertStr);
        } else {
            if (paramStr[curIndex] == ',') {
                preIndex = curIndex;
            }
        }
    }
    return paramStr;
}
} // namespace HDI
} // namespace OHOS