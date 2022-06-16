/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
bool CppServiceImplCodeEmitter::ResolveDirectory(const String &targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE || ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = GetFileParentPath(targetDirectory);
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
    String filePath =
        File::AdapterPath(String::Format("%s/%s.h", directory_.string(), FileName(baseName_ + "Service").string()));
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

void CppServiceImplCodeEmitter::EmitServiceImplInclusions(StringBuilder &sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    headerFiles.emplace(HeaderFileType::OWN_HEADER_FILE, EmitVersionHeaderName(interfaceName_));

    for (const auto &file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceImplCodeEmitter::EmitServiceImplDecl(StringBuilder &sb)
{
    EmitBeginNamespace(sb);
    sb.AppendFormat("class %sService : public %s {\n", baseName_.string(), interfaceName_.string());
    sb.Append("public:\n");
    EmitServiceImplBody(sb, TAB);
    sb.Append("};\n");
    EmitEndNamespace(sb);
}

void CppServiceImplCodeEmitter::EmitServiceImplBody(StringBuilder &sb, const String &prefix)
{
    EmitServiceImplConstructor(sb, TAB);
    sb.Append("\n");
    EmitServiceImplMethodDecls(sb, TAB);
}

void CppServiceImplCodeEmitter::EmitServiceImplConstructor(StringBuilder &sb, const String &prefix)
{
    sb.Append(prefix).AppendFormat("%s() = default;\n", implName_.string());
    sb.Append(prefix).AppendFormat("virtual ~%s() = default;\n", implName_.string());
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodDecls(StringBuilder &sb, const String &prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodDecl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodDecl(
    const AutoPtr<ASTMethod> &method, StringBuilder &sb, const String &prefix)
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

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
}

void CppServiceImplCodeEmitter::EmitImplSourceFile()
{
    String filePath =
        File::AdapterPath(String::Format("%s/%s.cpp", directory_.string(), FileName(baseName_ + "Service").string()));
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitImplSourceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    EmitServiceImplGetMethodImpl(sb, "");
    EmitServiceImplMethodImpls(sb, "");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceImplCodeEmitter::EmitImplSourceInclusions(StringBuilder &sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    headerFiles.emplace(HeaderFileType::OWN_HEADER_FILE, EmitVersionHeaderName(implName_));
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base");

    for (const auto &file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceImplCodeEmitter::GetSourceOtherLibInclusions(HeaderFile::HeaderFileSet &headerFiles)
{
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base");
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodImpls(StringBuilder &sb, const String &prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceImplCodeEmitter::EmitServiceImplMethodImpl(
    const AutoPtr<ASTMethod> &method, StringBuilder &sb, const String &prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %sService::%s()\n", baseName_.string(), method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %sService::%s(", baseName_.string(), method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.AppendFormat(")");

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }

    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}

void CppServiceImplCodeEmitter::EmitServiceImplGetMethodImpl(StringBuilder &sb, const String &prefix)
{
    if (!interface_->IsSerializable()) {
        sb.Append(prefix).AppendFormat(
            "extern \"C\" %s *%sImplGetInstance(void)\n", interfaceName_.string(), baseName_.string());
        sb.Append(prefix).Append("{\n");
        sb.Append(prefix + TAB).AppendFormat("return new (std::nothrow) %s();\n", implName_.string());
        sb.Append(prefix).Append("}\n\n");
    }
}
} // namespace HDI
} // namespace OHOS