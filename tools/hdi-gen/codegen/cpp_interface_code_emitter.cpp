/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_interface_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CppInterfaceCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppInterfaceCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppInterfaceCodeEmitter::EmitCode()
{
    EmitInterfaceHeaderFile();
}

void CppInterfaceCodeEmitter::EmitInterfaceHeaderFile()
{
    String filePath = String::Format("%s/%s.h", directory_.string(), FileName(interfaceName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, interfaceFullName_);
    sb.Append("\n");
    EmitInterfaceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitUsingNamespace(sb);
    sb.Append("\n");
    EmitInterfaceMethodCommands(sb, "");
    sb.Append("\n");
    EmitInterfaceDefinition(sb);
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitTailMacro(sb, interfaceFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppInterfaceCodeEmitter::EmitInterfaceInclusions(StringBuilder& sb)
{
    EmitInterfaceStdlibInclusions(sb);
    EmitInterfaceDBinderInclusions(sb);
    EmitInterfaceSelfDefinedTypeInclusions(sb);
}

void CppInterfaceCodeEmitter::EmitInterfaceStdlibInclusions(StringBuilder& sb)
{
    bool includeString = false;
    bool includeList = false;
    bool includeMap = false;

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_STRING: {
                if (!includeString) {
                    sb.Append("#include <string>\n");
                    includeString = true;
                }
                break;
            }
            case TypeKind::TYPE_ARRAY:
            case TypeKind::TYPE_LIST: {
                if (!includeList) {
                    sb.Append("#include <vector>\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::TYPE_MAP: {
                if (!includeMap) {
                    sb.Append("#include <map>\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceDBinderInclusions(StringBuilder& sb)
{
    sb.Append("#include <stdint.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <iservmgr_hdi.h>\n");
}

void CppInterfaceCodeEmitter::EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;
        sb.Append("#include ").AppendFormat("\"%s.h\"\n", FileName(importAst->GetFullName()).string());
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceDefinition(StringBuilder& sb)
{
    if (!isCallbackInterface()) {
        sb.Append("#ifndef __HDI_SERVER__\n");
        sb.AppendFormat("class %s : public IRemoteBroker {\n", interfaceName_.string());
        sb.Append("public:\n");
        EmitInterfaceDescriptor(sb, g_tab);
        sb.Append("\n");
        EmitGetMethodDecl(sb, g_tab);
        sb.Append("\n");
        EmitGetInstanceMethodDecl(sb, g_tab);
        sb.Append("#else\n");
        sb.AppendFormat("class %s {\n", interfaceName_.string());
        sb.Append("public:\n");
        sb.Append("#endif\n");


        EmitInterfaceDestruction(sb, g_tab);
        sb.Append("\n");
        EmitInterfaceMethodsDecl(sb, g_tab);
        sb.Append("};\n\n");
    } else {
        sb.AppendFormat("class %s : public IRemoteBroker {\n", interfaceName_.string());
        sb.Append("public:\n");
        EmitInterfaceDescriptor(sb, g_tab);
        sb.Append("\n");
        EmitInterfaceDestruction(sb, g_tab);
        sb.Append("\n");
        EmitInterfaceMethodsDecl(sb, g_tab);
        sb.Append("};\n\n");
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceDescriptor(StringBuilder& sb, const String& prefix)
{
    sb.Append(g_tab).AppendFormat("DECLARE_INTERFACE_DESCRIPTOR(u\"%s\");\n", interfaceFullName_.string());
}

void CppInterfaceCodeEmitter::EmitGetMethodDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("static sptr<%s> Get();\n", interface_->GetName().string());
}

void CppInterfaceCodeEmitter::EmitGetInstanceMethodDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("static sptr<%s> GetInstance(const std::string& serviceName);\n",
        interface_->GetName().string());
}

void CppInterfaceCodeEmitter::EmitInterfaceDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() = default;\n", interface_->GetName().string());
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodsDecl(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethodDecl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("virtual int32_t %s() = 0;\n", method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("virtual int32_t %s(", method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(") = 0;");
        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppParameter());
}
} // namespace HDI
} // namespace OHOS