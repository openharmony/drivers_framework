/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_interface_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CInterfaceCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CInterfaceCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CInterfaceCodeEmitter::EmitCode()
{
    EmitInterfaceHeaderFile();
}

void CInterfaceCodeEmitter::EmitInterfaceHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(interfaceName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, interfaceFullName_);
    sb.Append("\n");
    EmitImportInclusions(sb);
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitInterfaceMethodCommands(sb);
    sb.Append("\n");
    EmitInterfaceDefinition(sb);
    sb.Append("\n");
    EmitInterfaceGetMethodDecl(sb);
    sb.Append("\n");
    EmitInterfaceReleaseMethodDecl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, interfaceFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CInterfaceCodeEmitter::EmitImportInclusions(StringBuilder& sb)
{
    sb.Append("#include <stdint.h>\n");
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;
        sb.AppendFormat("#include \"%s.h\"\n", FileName(importAst->GetFullName()).string());
    }
}

void CInterfaceCodeEmitter::EmitInterfaceDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct %s {\n", interfaceName_.string());
    if (isCallbackInterface()) {
        sb.Append(g_tab).Append("struct HdfRemoteService *remote;\n");
        sb.Append("\n");
    }
    EmitInterfaceMethods(sb, g_tab);
    sb.Append("};\n");
}

void CInterfaceCodeEmitter::EmitInterfaceMethods(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethod(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CInterfaceCodeEmitter::EmitInterfaceMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t (*%s)(struct %s *self);\n",
            method->GetName().string(), interfaceName_.string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t (*%s)(struct %s *self, ",
            method->GetName().string(), interfaceName_.string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(");");
        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }
}

void CInterfaceCodeEmitter::EmitInterfaceGetMethodDecl(StringBuilder& sb)
{
    String methodParamStr = isCallbackInterface() ? "struct HdfRemoteService *remote" : "void";
    sb.AppendFormat("struct %s *%sGet(%s);\n", interfaceName_.string(), infName_.string(), methodParamStr.string());
    if (!isCallbackInterface()) {
        sb.Append("\n");
        sb.AppendFormat("struct %s *%sGetInstance(const char *instanceName);\n",
            interfaceName_.string(), infName_.string());
    }
}

void CInterfaceCodeEmitter::EmitInterfaceReleaseMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("void %sRelease(struct %s *instance);\n", infName_.string(), interfaceName_.string());
}

} // namespace HDI
} // namespace OHOS