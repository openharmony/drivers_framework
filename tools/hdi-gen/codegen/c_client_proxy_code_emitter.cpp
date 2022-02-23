/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_client_proxy_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CClientProxyCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = GetFilePath(targetDirectory);
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CClientProxyCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CClientProxyCodeEmitter::EmitCode()
{
    EmitProxySourceFile();
}

void CClientProxyCodeEmitter::EmitProxySourceFile()
{
    String filePath = String::Format("%s/%s.c", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitProxyInclusions(sb);
    sb.Append("\n");
    EmitProxyDefinition(sb);
    sb.Append("\n");
    if (!isKernelCode_) {
        EmitProxyCallMethodImpl(sb);
    } else {
        EmitProxyKernelCallMethodImpl(sb);
    }

    sb.Append("\n");
    EmitProxyMethodImpls(sb);
    sb.Append("\n");
    EmitProxyConstruction(sb);
    if (!isCallbackInterface()) {
        sb.Append("\n");
        EmitProxyGetMethodImpl(sb);
        sb.Append("\n");
        if (isKernelCode_) {
            EmitKernelProxyGetInstanceMethodImpl(sb);
            sb.Append("\n");
            EmitKernelProxyReleaseMethodImpl(sb);
        } else {
            EmitProxyGetInstanceMethodImpl(sb);
            sb.Append("\n");
            EmitProxyReleaseMethodImpl(sb);
        }
    } else {
        sb.Append("\n");
        EmitCbProxyGetMethodImpl(sb);
        sb.Append("\n");
        EmitProxyReleaseMethodImpl(sb);
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CClientProxyCodeEmitter::EmitProxyInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(interfaceName_)));
    GetHeaderOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CClientProxyCodeEmitter::GetHeaderOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_sbuf"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "osal_mem"));

    if (isKernelCode_) {
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_io_service_if"));
    } else {
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "servmgr_hdi"));
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_dlist"));
    }

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_STRING || type->GetTypeKind() == TypeKind::TYPE_UNION) {
            headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "securec"));
            break;
        }
    }
}

void CClientProxyCodeEmitter::EmitProxyDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct %sProxy {\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct %s impl;\n", interfaceName_.string());
    if (isKernelCode_) {
        sb.Append(g_tab).Append("struct HdfIoService *serv;\n");
    } else {
        sb.Append(g_tab).Append("struct HdfRemoteService *remote;\n");
    }

    sb.Append("};\n");
}

void CClientProxyCodeEmitter::EmitProxyCallMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("static int32_t %sProxyCall(struct %s *self, int32_t id, struct HdfSBuf *data,\n",
        infName_.string(), interfaceName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *reply, bool isOneWay)\n");
    sb.Append("{\n");

    String remoteName = "remote";
    sb.Append(g_tab).AppendFormat("struct HdfRemoteService *%s = self->AsObject(self);\n", remoteName.string());
    sb.Append(g_tab).AppendFormat("if (%s == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| %s->dispatcher == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| %s->dispatcher->Dispatch == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| %s->dispatcher->DispatchAsync == NULL) {\n",
        remoteName.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: Invalid HdfRemoteService obj\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return HDF_ERR_INVALID_OBJECT;\n");
    sb.Append(g_tab).Append("}\n");

    sb.Append(g_tab).AppendFormat("if (isOneWay) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("return %s->dispatcher->DispatchAsync(%s, id, data, reply);\n",
        remoteName.string(), remoteName.string());
    sb.Append(g_tab).AppendFormat("} else {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("return %s->dispatcher->Dispatch(%s, id, data, reply);\n",
        remoteName.string(), remoteName.string());
    sb.Append(g_tab).AppendFormat("}\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyKernelCallMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("static int32_t %sProxyCall(struct %s *self, int32_t id, struct HdfSBuf *data,\n",
        infName_.string(), interfaceName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *reply)\n");
    sb.Append("{\n");

    String remoteName = "serv";
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = CONTAINER_OF(self, struct %sProxy, impl);\n",
        infName_.string(), infName_.string(), remoteName.string());
    sb.Append(g_tab).AppendFormat("struct HdfIoService *%s = proxy->%s;\n", remoteName.string(), remoteName.string());

    sb.Append(g_tab).AppendFormat("if (%s == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| %s->dispatcher == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| %s->dispatcher->Dispatch == NULL) {\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: Invalid HdfRemoteService obj\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return HDF_ERR_INVALID_OBJECT;\n");
    sb.Append(g_tab).Append("}\n\n");

    sb.Append(g_tab).AppendFormat("return %s->dispatcher->Dispatch(", remoteName.string());
    sb.AppendFormat("(struct HdfObject *)&(%s->object), id, data, reply);\n", remoteName.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder& sb)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodImpl(method, sb);
        sb.Append("\n");
    }

    EmitProxyMethodImpl(interface_->GetVersionMethod(), sb);

    if (!isKernelCode_) {
        sb.Append("\n");
        EmitProxyAsObjectMethodImpl(sb);
    }
}

void CClientProxyCodeEmitter::EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb)
{
    if (method->GetParameterNumber() == 0) {
        sb.AppendFormat("static int32_t %sProxy%s(struct %s *self)\n",
            infName_.string(), method->GetName().string(), interfaceName_.string());
    } else {
        StringBuilder paramStr;
        paramStr.AppendFormat("static int32_t %sProxy%s(", infName_.string(), method->GetName().string());
        paramStr.AppendFormat("struct %s *self, ", interfaceName_.string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(")");
        sb.Append(SpecificationParam(paramStr, g_tab));
        sb.Append("\n");
    }
    EmitProxyMethodBody(method, sb, "");
}

void CClientProxyCodeEmitter::EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).AppendFormat("int32_t %s = HDF_FAILURE;\n", errorCodeName_.string());

    // Local variable definitions must precede all execution statements.
    EmitInitLoopVar(method, sb, prefix + g_tab);

    sb.Append("\n");
    EmitCreateBuf(dataParcelName_, replyParcelName_, sb, prefix + g_tab);
    sb.Append("\n");

    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_IN) {
            param->EmitCWriteVar(dataParcelName_, errorCodeName_, finishedLabelName_, sb, prefix + g_tab);
            sb.Append("\n");
        } else if (param->EmitCProxyWriteOutVar(dataParcelName_, errorCodeName_, finishedLabelName_, sb,
            prefix + g_tab)) {
            sb.Append("\n");
        }
    }

    EmitStubCallMethod(method, sb, prefix + g_tab);
    sb.Append("\n");

    if (!method->IsOneWay()) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
                EmitReadProxyMethodParameter(param, replyParcelName_, finishedLabelName_, sb, prefix + g_tab);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix).AppendFormat("%s:\n", finishedLabelName_);
    EmitReleaseBuf(dataParcelName_, replyParcelName_, sb, prefix + g_tab);

    sb.Append(prefix + g_tab).AppendFormat("return %s;\n", errorCodeName_.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitCreateBuf(const String& dataBufName, const String& replyBufName,
    StringBuilder& sb, const String& prefix)
{
    if (isKernelCode_) {
        sb.Append(prefix).AppendFormat("struct HdfSBuf *%s = HdfSbufObtainDefaultSize();\n", dataBufName.string());
        sb.Append(prefix).AppendFormat("struct HdfSBuf *%s = HdfSbufObtainDefaultSize();\n", replyBufName.string());
    } else {
        sb.Append(prefix).AppendFormat("struct HdfSBuf *%s = HdfSbufTypedObtain(SBUF_IPC);\n", dataBufName.string());
        sb.Append(prefix).AppendFormat("struct HdfSBuf *%s = HdfSbufTypedObtain(SBUF_IPC);\n", replyBufName.string());
    }

    sb.Append("\n");
    sb.Append(prefix).AppendFormat("if (%s == NULL || %s == NULL) {\n", dataBufName.string(), replyBufName.string());
    sb.Append(prefix + g_tab).Append("HDF_LOGE(\"%{public}s: HdfSubf malloc failed!\", __func__);\n");
    sb.Append(prefix + g_tab).AppendFormat("%s = HDF_ERR_MALLOC_FAIL;\n", errorCodeName_.string());
    sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", finishedLabelName_);
    sb.Append(prefix).Append("}\n");
}

void CClientProxyCodeEmitter::EmitReleaseBuf(const String& dataBufName, const String& replyBufName, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat("if (%s != NULL) {\n", dataBufName.string());
    sb.Append(prefix + g_tab).AppendFormat("HdfSbufRecycle(%s);\n", dataBufName.string());
    sb.Append(prefix).Append("}\n");
    sb.Append(prefix).AppendFormat("if (%s != NULL) {\n", replyBufName.string());
    sb.Append(prefix + g_tab).AppendFormat("HdfSbufRecycle(%s);\n", replyBufName.string());
    sb.Append(prefix).Append("}\n");
}

void CClientProxyCodeEmitter::EmitReadProxyMethodParameter(const AutoPtr<ASTParameter>& param,
    const String& parcelName, const String& gotoLabel, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    if (type->GetTypeKind() == TypeKind::TYPE_STRING) {
        String cloneName = String::Format("%sCopy", param->GetName().string());
        type->EmitCProxyReadVar(parcelName, cloneName, false, errorCodeName_, gotoLabel, sb, prefix);
        sb.Append(prefix).AppendFormat("if (strcpy_s(%s, %sLen, %s) != EOK) {\n",
            param->GetName().string(), param->GetName().string(), cloneName.string());
        sb.Append(prefix + g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n",
            param->GetName().string());
        sb.Append(prefix + g_tab).AppendFormat("%s = HDF_ERR_INVALID_PARAM;\n", errorCodeName_.string());
        sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
        sb.Append(prefix).Append("}\n");
    } else if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        type->EmitCProxyReadVar(parcelName, param->GetName().string(), false, errorCodeName_, gotoLabel, sb, prefix);
    } else if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
        String cpName = String::Format("%sCp", param->GetName().string());
        type->EmitCProxyReadVar(parcelName, cpName, false, errorCodeName_, gotoLabel, sb, prefix);
        sb.Append(prefix).AppendFormat("(void)memcpy_s(%s, sizeof(%s), %s, sizeof(%s));\n",
            param->GetName().string(), type->EmitCType().string(), cpName.string(), type->EmitCType().string());
    } else {
        type->EmitCProxyReadVar(parcelName, param->GetName(), false, errorCodeName_, gotoLabel, sb, prefix);
    }
}

void CClientProxyCodeEmitter::EmitStubCallMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (!isKernelCode_) {
        sb.Append(prefix).AppendFormat("%s = %sCall(self, %s, %s, %s, %s);\n", errorCodeName_.string(),
            proxyName_.string(), EmitMethodCmdID(method).string(), dataParcelName_.string(),
            replyParcelName_.string(), method->IsOneWay() ? "true" : "false");
    } else {
        sb.Append(prefix).AppendFormat("%s = %sCall(self, %s, %s, %s);\n", errorCodeName_.string(),
            proxyName_.string(), EmitMethodCmdID(method).string(), dataParcelName_.string(),
            replyParcelName_.string());
    }
    sb.Append(prefix).AppendFormat("if (%s != HDF_SUCCESS) {\n", errorCodeName_.string());
    sb.Append(prefix + g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: call failed! error code is %%{public}d\", __func__, %s);\n", errorCodeName_.string());
    sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", finishedLabelName_);
    sb.Append(prefix).Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyAsObjectMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("static struct HdfRemoteService *%sProxyAsObject(struct %s *self)\n",
        infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (self == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = CONTAINER_OF(self, struct %sProxy, impl);\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append("return proxy->remote;\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyConstruction(StringBuilder& sb)
{
    String objName = "impl";
    sb.AppendFormat("static void %sProxyConstruct(struct %s *%s)\n",
        infName_.string(), interfaceName_.string(), objName.string());
    sb.Append("{\n");

    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(g_tab).AppendFormat("%s->%s = %sProxy%s;\n",
            objName.string(), method->GetName().string(), infName_.string(), method->GetName().string());
    }

    AutoPtr<ASTMethod> getVerMethod = interface_->GetVersionMethod();
    sb.Append(g_tab).AppendFormat("%s->%s = %sProxy%s;\n", objName.string(), getVerMethod->GetName().string(),
        infName_.string(), getVerMethod->GetName().string());

    if (!isKernelCode_) {
        sb.Append(g_tab).AppendFormat("%s->AsObject = %sProxyAsObject;\n", objName.string(), infName_.string());
    }

    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyGetMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *%sGet()\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("return %sGetInstance(\"%s\");\n", infName_.string(), FileName(implName_).string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyGetInstanceMethodImpl(StringBuilder& sb)
{
    String objName = "client";
    String SerMajorName = "serMajorVer";
    String SerMinorName = "serMinorVer";
    sb.AppendFormat("struct %s *%sGetInstance(const char *instanceName)\n",
        interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();\n");
    sb.Append(g_tab).Append("if (serviceMgr == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: HDIServiceManager not found!\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("struct HdfRemoteService *remote = ");
    sb.Append("serviceMgr->GetService(serviceMgr, instanceName);\n");
    sb.Append(g_tab).Append("HDIServiceManagerRelease(serviceMgr);\n");
    sb.Append(g_tab).Append("if (remote == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: %sService not found!\", __func__);\n",
        infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = ", infName_.string());
    sb.AppendFormat("(struct %sProxy *)OsalMemAlloc(sizeof(struct %sProxy));\n", infName_.string(),
        infName_.string());
    sb.Append(g_tab).Append("if (proxy == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: malloc %s proxy failed!\", __func__);\n", interfaceName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HdfRemoteServiceRecycle(remote);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("proxy->remote = remote;\n");
    sb.Append(g_tab).AppendFormat("struct %s *%s = &proxy->impl;\n", interfaceName_.string(), objName.string());
    sb.Append(g_tab).AppendFormat("%sProxyConstruct(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).AppendFormat("uint32_t %s = 0;\n", SerMajorName.string());
    sb.Append(g_tab).AppendFormat("uint32_t %s = 0;\n", SerMinorName.string());
    sb.Append(g_tab).AppendFormat("int32_t %s = %s->GetVersion(%s, &%s, &%s);\n",
        errorCodeName_.string(), objName.string(), objName.string(), SerMajorName.string(), SerMinorName.string());
    sb.Append(g_tab).AppendFormat("if (%s != HDF_SUCCESS) {\n", errorCodeName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: get version failed!\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("%sRelease(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("if (%s != %s) {\n", SerMajorName.string(), majorVerName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s:check version failed! ");
    sb.Append("version of service:%u.%u, version of client:%u.%u\", __func__,\n");
    sb.Append(g_tab).Append(g_tab).Append(g_tab).AppendFormat("%s, %s, %s, %s);\n", SerMajorName.string(),
        SerMinorName.string(), majorVerName_.string(), minorVerName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("%sRelease(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("return %s;\n", objName.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitKernelProxyGetInstanceMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *%sGetInstance(const char* serviceName)\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct HdfIoService *serv = ");
    sb.Append("HdfIoServiceBind(serviceName);\n");
    sb.Append(g_tab).Append("if (serv == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: %sService not found!\", __func__);\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = (struct %sProxy *)OsalMemAlloc(sizeof(struct %sProxy));\n",
        infName_.string(), infName_.string(), infName_.string());
    sb.Append(g_tab).Append("if (proxy == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: malloc %s proxy failed!\", __func__);\n", interfaceName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HdfIoServiceRecycle(serv);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).Append("proxy->serv = serv;\n");
    sb.Append(g_tab).AppendFormat("%sProxyConstruct(&proxy->impl);\n", infName_.string());
    sb.Append(g_tab).Append("return &proxy->impl;\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitCbProxyGetMethodImpl(StringBuilder& sb)
{
    String objName = "client";
    String SerMajorName = "serMajorVer";
    String SerMinorName = "serMinorVer";
    sb.AppendFormat("struct %s *%sGet(struct HdfRemoteService *remote)\n",
        interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (remote == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: %sService not found!\", __func__);\n",
        infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = ", infName_.string());
    sb.AppendFormat("(struct %sProxy *)OsalMemAlloc(sizeof(struct %sProxy));\n", infName_.string(),
        infName_.string());
    sb.Append(g_tab).Append("if (proxy == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: malloc %s proxy failed!\", __func__);\n", interfaceName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HdfRemoteServiceRecycle(remote);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("proxy->remote = remote;\n");
    sb.Append(g_tab).AppendFormat("struct %s *%s = &proxy->impl;\n", interfaceName_.string(), objName.string());
    sb.Append(g_tab).AppendFormat("%sProxyConstruct(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).AppendFormat("uint32_t %s = 0;\n", SerMajorName.string());
    sb.Append(g_tab).AppendFormat("uint32_t %s = 0;\n", SerMinorName.string());
    sb.Append(g_tab).AppendFormat("int32_t %s = %s->GetVersion(%s, &%s, &%s);\n",
        errorCodeName_.string(), objName.string(), objName.string(), SerMajorName.string(), SerMinorName.string());
    sb.Append(g_tab).AppendFormat("if (%s != HDF_SUCCESS) {\n", errorCodeName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: get version failed!\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("%sRelease(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("if (%s != %s) {\n", SerMajorName.string(), majorVerName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s:check version failed! ");
    sb.Append("version of service:%u.%u, version of client:%u.%u\", __func__,\n");
    sb.Append(g_tab).Append(g_tab).Append(g_tab).AppendFormat("%s, %s, %s, %s);\n", SerMajorName.string(),
        SerMinorName.string(), majorVerName_.string(), minorVerName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("%sRelease(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("return %s;\n", objName.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyReleaseMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (instance == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = CONTAINER_OF(instance, struct %sProxy, impl);\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append("HdfRemoteServiceRecycle(proxy->remote);\n");
    sb.Append(g_tab).Append("OsalMemFree(proxy);\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitKernelProxyReleaseMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (instance == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("struct %sProxy *proxy = CONTAINER_OF(instance, struct %sProxy, impl);\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append("HdfIoServiceRecycle(proxy->serv);\n");
    sb.Append(g_tab).Append("OsalMemFree(proxy);\n");
    sb.Append("}\n");
}
} // namespace HDI
} // namespace OHOS