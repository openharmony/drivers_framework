/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_service_impl_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CServiceImplCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CServiceImplCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CServiceImplCodeEmitter::EmitCode()
{
    EmitServiceImplHeaderFile();
    EmitServiceImplSourceFile();
}

void CServiceImplCodeEmitter::EmitServiceImplHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_ + "Service").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, implFullName_);
    sb.Append("\n");
    EmitServiceImplHeaderInclusions(sb);
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitServiceImplConstructDecl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, implFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceImplCodeEmitter::EmitServiceImplHeaderInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(interfaceName_)));

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CServiceImplCodeEmitter::EmitServiceImplConstructDecl(StringBuilder& sb)
{
    sb.AppendFormat("void %sServiceConstruct(struct %s* service);\n", infName_.string(), interfaceName_.string());
}

void CServiceImplCodeEmitter::EmitServiceImplSourceFile()
{
    String filePath = String::Format("%s%s.c", directory_.string(), FileName(infName_ + "Service").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitServiceImplSourceInclusions(sb);
    sb.Append("\n");
    EmitServiceImplMethodImpls(sb, "");
    sb.Append("\n");
    EmitServiceImplConstruct(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceImplCodeEmitter::EmitServiceImplSourceInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, FileName(implName_)));
    GetSourceOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CServiceImplCodeEmitter::GetSourceOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "osal_mem"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "securec"));
}

void CServiceImplCodeEmitter::EmitServiceImplMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CServiceImplCodeEmitter::EmitServiceImplMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s%s(struct %s *self)\n",
            infName_.string(), method->GetName().string(), interfaceName_.string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s%s(struct %s *self, ",
            infName_.string(), method->GetName().string(), interfaceName_.string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(")");
        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }

    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}

void CServiceImplCodeEmitter::EmitServiceImplConstruct(StringBuilder& sb)
{
    String objName("instance");
    sb.AppendFormat("void %sServiceConstruct(struct %s *%s)\n",
        infName_.string(), interfaceName_.string(), objName.string());
    sb.Append("{\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(g_tab).AppendFormat("%s->%s = %s%s;\n",
            objName.string(), method->GetName().string(), infName_.string(), method->GetName().string());
    }
    sb.Append("}");
}
} // namespace HDI
} // namespace OHOS