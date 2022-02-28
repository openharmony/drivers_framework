/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_client_proxy_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CppClientProxyCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = GetFilePath(targetDirectory);
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppClientProxyCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppClientProxyCodeEmitter::EmitCode()
{
    EmitProxyHeaderFile();
    EmitProxySourceFile();
}

void CppClientProxyCodeEmitter::EmitProxyHeaderFile()
{
    String filePath = String::Format("%s/%s.h", directory_.string(), FileName(baseName_ + "Proxy").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, proxyFullName_);
    sb.Append("\n");
    EmitProxyHeaderInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitProxyDecl(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitTailMacro(sb, proxyFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppClientProxyCodeEmitter::EmitProxyHeaderInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, EmitVersionHeaderName(interfaceName_)));
    GetHeaderOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppClientProxyCodeEmitter::GetHeaderOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "iremote_proxy"));
}

void CppClientProxyCodeEmitter::EmitProxyDecl(StringBuilder& sb, const String& prefix)
{
    sb.AppendFormat("class %s : public IRemoteProxy<%s> {\n",
        proxyName_.string(), interfaceName_.string());
    sb.Append("public:\n");
    EmitProxyConstructor(sb, g_tab);
    sb.Append("\n");
    EmitProxyMethodDecls(sb, g_tab);
    sb.Append("\n");
    sb.Append("private:\n");
    EmitProxyConstants(sb, g_tab);
    sb.Append("};\n");
}

void CppClientProxyCodeEmitter::EmitProxyConstructor(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("explicit %s(const sptr<IRemoteObject>& remote)", proxyName_.string());
    sb.AppendFormat(" : IRemoteProxy<%s>(remote) {}\n\n", interfaceName_.string());
    sb.Append(prefix).AppendFormat("virtual ~%s() {}\n", proxyName_.string());
}

void CppClientProxyCodeEmitter::EmitProxyMethodDecls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodDecl(method, sb, prefix);
        sb.Append("\n");
    }

    EmitProxyMethodDecl(interface_->GetVersionMethod(), sb, prefix);
}

void CppClientProxyCodeEmitter::EmitProxyMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s() override;\n", method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s(", method->GetName().string());

        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitProxyMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(") override;");

        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }
}

void CppClientProxyCodeEmitter::EmitProxyConstants(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("static inline BrokerDelegator<%s> delegator_;\n", proxyName_.string());
}

void CppClientProxyCodeEmitter::EmitProxyMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppParameter());
}

void CppClientProxyCodeEmitter::EmitProxySourceFile()
{
    String filePath = String::Format("%s/%s.cpp", directory_.string(), FileName(baseName_ + "Proxy").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitProxySourceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        EmitGetMethodImpl(sb, "");
        sb.Append("\n");
        EmitGetInstanceMethodImpl(sb, "");
        sb.Append("\n");
    }
    EmitProxyMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppClientProxyCodeEmitter::EmitProxySourceInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, EmitVersionHeaderName(proxyName_)));
    GetSourceOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppClientProxyCodeEmitter::GetSourceOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    if (!isCallbackInterface()) {
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "iservmgr_hdi"));
    }
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "message_option"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "message_parcel"));

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
            headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "securec"));
            break;
        }
    }
}

void CppClientProxyCodeEmitter::EmitGetMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("sptr<%s> %s::Get()\n",
        interface_->GetName().string(), interface_->GetName().string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).AppendFormat("return %s::GetInstance(\"%s\");\n",
        interfaceName_.string(), FileName(implName_).string());
    sb.Append(prefix).Append("}\n");
}

void CppClientProxyCodeEmitter::EmitGetInstanceMethodImpl(StringBuilder& sb, const String& prefix)
{
    String objName = "proxy";
    String SerMajorName = "serMajorVer";
    String SerMinorName = "serMinorVer";
    sb.Append(prefix).AppendFormat("sptr<%s> %s::GetInstance(const std::string& serviceName)\n",
        interface_->GetName().string(), interface_->GetName().string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).Append("using namespace OHOS::HDI::ServiceManager::V1_0;\n");
    sb.Append(prefix + g_tab).Append("auto servMgr = IServiceManager::Get();\n");
    sb.Append(prefix + g_tab).Append("if (servMgr == nullptr) {\n");
    sb.Append(prefix + g_tab + g_tab).Append(
        "HDF_LOGE(\"%{public}s:get IServiceManager failed!\", __func__);\n");
    sb.Append(prefix + g_tab + g_tab).Append("return nullptr;\n");
    sb.Append(prefix + g_tab).Append("}\n\n");

    sb.Append(prefix + g_tab).Append("sptr<IRemoteObject> remote = ");
    sb.Append("servMgr->GetService(serviceName.c_str());\n");
    sb.Append(prefix + g_tab).Append("if (remote == nullptr) {\n");
    sb.Append(prefix + g_tab + g_tab).Append(
        "HDF_LOGE(\"%{public}s:get remote object failed!\", __func__);\n");
    sb.Append(prefix + g_tab + g_tab).Append("return nullptr;\n");
    sb.Append(prefix + g_tab).Append("}\n\n");

    sb.Append(prefix + g_tab).AppendFormat("sptr<%s> %s = iface_cast<%s>(remote);\n",
        interfaceName_.string(), objName.string(), interfaceName_.string());
    sb.Append(prefix + g_tab).AppendFormat("if (%s == nullptr) {\n", objName.string());
    sb.Append(prefix + g_tab + g_tab).Append("HDF_LOGE(\"%{public}s:iface_cast failed!\", __func__);\n");
    sb.Append(prefix + g_tab + g_tab).Append("return nullptr;\n");
    sb.Append(prefix + g_tab).Append("}\n\n");

    sb.Append(prefix + g_tab).AppendFormat("uint32_t %s = 0;\n", SerMajorName.string());
    sb.Append(prefix + g_tab).AppendFormat("uint32_t %s = 0;\n", SerMinorName.string());
    sb.Append(prefix + g_tab).AppendFormat("int32_t %s = %s->GetVersion(%s, %s);\n", errorCodeName_.string(),
        objName.string(), SerMajorName.string(), SerMinorName.string());
    sb.Append(prefix + g_tab).AppendFormat("if (%s != HDF_SUCCESS) {\n", errorCodeName_.string());
    sb.Append(prefix + g_tab + g_tab).Append("HDF_LOGE(\"%{public}s:get version failed!\", __func__);\n");
    sb.Append(prefix + g_tab + g_tab).Append("return nullptr;\n");
    sb.Append(prefix + g_tab).Append("}\n\n");

    sb.Append(prefix + g_tab).AppendFormat("if (%s != %s) {\n", SerMajorName.string(), majorVerName_.string());
    sb.Append(prefix + g_tab + g_tab).Append("HDF_LOGE(\"%{public}s:check version failed! ");
    sb.Append("version of service:%u.%u, version of client:%u.%u\", __func__,\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).AppendFormat("%s, %s, %s, %s);\n",
        SerMajorName.string(), SerMinorName.string(), majorVerName_.string(), minorVerName_.string());
    sb.Append(prefix + g_tab + g_tab).Append("return nullptr;\n");
    sb.Append(prefix + g_tab).Append("}\n\n");
    sb.Append(prefix + g_tab).AppendFormat("return %s;\n", objName.string());
    sb.Append(prefix).Append("}\n");
}

void CppClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodImpl(method, sb, prefix);
        sb.Append("\n");
    }

    EmitProxyMethodImpl(interface_->GetVersionMethod(), sb, prefix);
}

void CppClientProxyCodeEmitter::EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s::%s()\n", proxyName_.string(), method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s::%s(", proxyName_.string(), method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitProxyMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(")");

        sb.Append(SpecificationParam(paramStr, prefix + g_tab));
        sb.Append("\n");
    }
    EmitProxyMethodBody(method, sb, prefix);
}

void CppClientProxyCodeEmitter::EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    String option = method->IsOneWay() ? "MessageOption::TF_ASYNC" : "MessageOption::TF_SYNC";
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).AppendFormat("MessageParcel %s;\n", dataParcelName_.string());
    sb.Append(prefix + g_tab).AppendFormat("MessageParcel %s;\n", replyParcelName_.string());
    sb.Append(prefix + g_tab).AppendFormat("MessageOption %s(%s);\n", optionName_.string(), option.string());
    sb.Append("\n");

    if (method->GetParameterNumber() > 0) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_IN) {
                EmitWriteMethodParameter(param, dataParcelName_, sb, prefix + g_tab);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix + g_tab).AppendFormat("int32_t %s = Remote()->SendRequest(%s, %s, %s, %s);\n",
        errorCodeName_.string(), EmitMethodCmdID(method).string(), dataParcelName_.string(),
        replyParcelName_.string(), optionName_.string());
    sb.Append(prefix + g_tab).AppendFormat("if (%s != HDF_SUCCESS) {\n", errorCodeName_.string());
    sb.Append(prefix + g_tab + g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s failed, error code is %%{public}d\", __func__, %s);\n", errorCodeName_.string());
    sb.Append(prefix + g_tab + g_tab).AppendFormat("return %s;\n", errorCodeName_.string());
    sb.Append(prefix + g_tab).Append("}\n");

    if (!method->IsOneWay()) {
        sb.Append("\n");
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
                EmitReadMethodParameter(param, replyParcelName_, false, sb, prefix + g_tab);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix + g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}
} // namespace HDI
} // namespace OHOS