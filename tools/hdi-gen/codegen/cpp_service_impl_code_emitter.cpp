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
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
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

    if (!isCallbackInterface()) {
        sb.Append("\n");
        EmitHeadExternC(sb);
        sb.Append("\n");
        EmitExternalGetMethodDecl(sb);
        sb.Append("\n");
        EmitExternalReleaseMethodDecl(sb);
        sb.Append("\n");
        EmitTailExternC(sb);
    }

    sb.Append("\n");
    EmitTailMacro(sb, implFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceImplCodeEmitter::EmitServiceImplInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    if (!isCallbackInterface()) {
        headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(interfaceName_)));
    } else {
        headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, FileName(stubName_)));
    }

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
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

void CppServiceImplCodeEmitter::EmitExternalGetMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("%s *%sServiceConstruct();\n", CppFullName(interfaceFullName_).string(), infName_.string());
}

void CppServiceImplCodeEmitter::EmitExternalReleaseMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("void %sServiceRelease(%s *obj);\n", infName_.string(), CppFullName(interfaceFullName_).string());
}

void CppServiceImplCodeEmitter::EmitImplSourceFile()
{
    String filePath = String::Format("%s%s.cpp", directory_.string(), FileName(infName_ + "Service").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitImplSourceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitServiceImplMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);
    if (!isCallbackInterface()) {
        sb.Append("\n");
        EmitExternalGetMethodImpl(sb);
        sb.Append("\n");
        EmitExternalReleaseMethodImpl(sb);
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceImplCodeEmitter::EmitImplSourceInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, FileName(implName_)));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceImplCodeEmitter::GetSourceOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
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

void CppServiceImplCodeEmitter::EmitExternalGetMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("%s *%sServiceConstruct()\n", CppFullName(interfaceFullName_).string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("using %s;\n", CppFullName(implFullName_).string());
    sb.Append(g_tab).AppendFormat("return new %sService();\n", infName_.string());
    sb.Append("}\n");
}

void CppServiceImplCodeEmitter::EmitExternalReleaseMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sServiceRelease(%s *obj)\n", infName_.string(), CppFullName(interfaceFullName_).string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("if (obj == nullptr) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("delete obj;\n");
    sb.Append("}\n");
}
} // namespace HDI
} // namespace OHOS