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
bool CppInterfaceCodeEmitter::ResolveDirectory(const String &targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE || ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = GetFileParentPath(targetDirectory);
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
    String filePath = File::AdapterPath(String::Format("%s/%s.h", directory_.string(),
        FileName(interfaceName_).string()));
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, interfaceFullName_);
    sb.Append("\n");
    EmitInterfaceInclusions(sb);
    sb.Append("\n");
    EmitInterfaceVersionMacro(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    EmitUsingNamespace(sb);
    sb.Append("\n");
    EmitInterfaceMethodCommands(sb, "");
    sb.Append("\n");
    EmitInterfaceDefinition(sb);
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitTailMacro(sb, interfaceFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppInterfaceCodeEmitter::EmitInterfaceInclusions(StringBuilder &sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    GetStdlibInclusions(headerFiles);
    GetImportInclusions(headerFiles);
    GetHeaderOtherLibInclusions(headerFiles);

    for (const auto &file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppInterfaceCodeEmitter::GetHeaderOtherLibInclusions(HeaderFile::HeaderFileSet &headerFiles)
{
    headerFiles.emplace(HeaderFileType::C_STD_HEADER_FILE, "stdint");
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdi_base");
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base");
}

void CppInterfaceCodeEmitter::EmitInterfaceVersionMacro(StringBuilder &sb)
{
    sb.AppendFormat("#define %s %u\n", majorVerName_.string(), ast_->GetMajorVer());
    sb.AppendFormat("#define %s %u\n", minorVerName_.string(), ast_->GetMinorVer());
}

void CppInterfaceCodeEmitter::EmitInterfaceDefinition(StringBuilder &sb)
{
    if (!interface_->IsSerializable()) {
        sb.AppendFormat("class %s : public HdiBase {\n", interfaceName_.string());
        sb.Append("public:\n");
        EmitInterfaceDescriptor(sb, TAB);
        sb.Append("\n");
        EmitInterfaceDestruction(sb, TAB);
        sb.Append("\n");
        EmitGetMethodDecl(sb, TAB);
        sb.Append("\n");
        EmitInterfaceMethodsDecl(sb, TAB);
        sb.Append("};\n");
    } else {
        sb.AppendFormat("class %s : public HdiBase {\n", interfaceName_.string());
        sb.Append("public:\n");
        EmitInterfaceDescriptor(sb, TAB);
        sb.Append("\n");
        EmitInterfaceDestruction(sb, TAB);
        sb.Append("\n");
        EmitInterfaceMethodsDecl(sb, TAB);
        sb.Append("};\n");
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceDescriptor(StringBuilder &sb, const String &prefix)
{
    sb.Append(TAB).AppendFormat("DECLARE_HDI_DESCRIPTOR(u\"%s\");\n", interfaceFullName_.string());
}

void CppInterfaceCodeEmitter::EmitGetMethodDecl(StringBuilder &sb, const String &prefix)
{
    sb.Append(prefix).AppendFormat("static sptr<%s> Get(bool isStub = false);\n",
        interface_->GetName().string());
    sb.Append(prefix).
        AppendFormat("static sptr<%s> Get(const std::string &serviceName, bool isStub = false);\n",
            interface_->GetName().string());
}

void CppInterfaceCodeEmitter::EmitInterfaceDestruction(StringBuilder &sb, const String &prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() = default;\n", interface_->GetName().string());
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodsDecl(StringBuilder &sb, const String &prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethodDecl(method, sb, prefix);
        sb.Append("\n");
    }

    EmitInterfaceGetVersionMethod(sb, prefix);
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodDecl(
    const AutoPtr<ASTMethod> &method, StringBuilder &sb, const String &prefix)
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
        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
}

void CppInterfaceCodeEmitter::EmitInterfaceGetVersionMethod(StringBuilder &sb, const String &prefix)
{
    AutoPtr<ASTMethod> method = interface_->GetVersionMethod();
    sb.Append(prefix).AppendFormat("virtual int32_t %s(", method->GetName().string());
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        EmitInterfaceMethodParameter(param, sb, "");
        if (i + 1 < method->GetParameterNumber()) {
            sb.Append(", ");
        }
    }

    sb.AppendFormat(")\n");
    sb.Append(prefix).Append("{\n");

    AutoPtr<ASTParameter> majorParam = method->GetParameter(0);
    sb.Append(prefix + TAB).AppendFormat("%s = %s;\n", majorParam->GetName().string(), majorVerName_.string());
    AutoPtr<ASTParameter> minorParam = method->GetParameter(1);
    sb.Append(prefix + TAB).AppendFormat("%s = %s;\n", minorParam->GetName().string(), minorVerName_.string());

    sb.Append(prefix + TAB).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}

void CppInterfaceCodeEmitter::EmitInterfaceMethodParameter(
    const AutoPtr<ASTParameter> &param, StringBuilder &sb, const String &prefix)
{
    sb.Append(prefix).Append(param->EmitCppParameter());
}
} // namespace HDI
} // namespace OHOS