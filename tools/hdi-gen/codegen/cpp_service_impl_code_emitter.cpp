/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_impl_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CppServiceImplCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE) {
        directory_ = String::Format("%s/%s/server/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string());
    } else if (ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string());
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppServiceImplCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppServiceImplCodeEmitter::EmitCode()
{
    EmitImplHeaderFile();
    EmitImplSourceFile();
}

void CppServiceImplCodeEmitter::EmitImplHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_ + "Service").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, implFullName_);
    sb.Append("\n");
    EmitServiceImplInclusions(sb);
    sb.Append("\n");
    EmitServiceImplDecl(sb);
    sb.Append("\n");
    EmitTailMacro(sb, implFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceImplCodeEmitter::EmitServiceImplInclusions(StringBuilder& sb)
{
    if (!isCallbackInterface()) {
        sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    } else {
        sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
    }
    sb.Append("#include <hdf_base.h>\n");
}

void CppServiceImplCodeEmitter::EmitServiceImplDecl(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        sb.AppendFormat("class %sService : public %s {\n", infName_.string(), interfaceName_.string());
    } else {
        sb.AppendFormat("class %sService : public %s {\n", infName_.string(), stubName_.string());
    }
    sb.Append("public:\n");
    EmitServiceImplBody(sb, g_tab);
    sb.Append("};\n");

    sb.Append("\n");
    EmitEndNamespace(sb);
}

void CppServiceImplCodeEmitter::EmitServiceImplBody(StringBuilder& sb, const String& prefix)
{
    EmitServiceImplDestruction(sb, g_tab);
    sb.Append("\n");
    EmitServiceImplMethodDecls(sb, g_tab);
}

void CppServiceImplCodeEmitter::EmitServiceImplDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%sService() {}\n", infName_.string());
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodDecls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodDecl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s() override;\n", method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s(", method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(") override;");

        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }
}

void CppServiceImplCodeEmitter::EmitImplSourceFile()
{
    String filePath = String::Format("%s%s.cpp", directory_.string(), FileName(infName_ + "Service").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    sb.AppendFormat("#include \"%s_service.h\"\n", FileName(infName_).string());
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitServiceImplMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %sService::%s()\n", infName_.string(), method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %sService::%s(", infName_.string(), method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.AppendFormat(")");

        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }

    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}
} // namespace HDI
} // namespace OHOS