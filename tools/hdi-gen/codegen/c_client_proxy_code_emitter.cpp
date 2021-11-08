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
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE) {
        directory_ = File::AdapterPath(String::Format("%s/%s/client/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else if (ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
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
    if (isCallbackInterface()) {
        EmitCbProxyHeaderFile();
    }
    EmitProxySourceFile();
}

void CClientProxyCodeEmitter::EmitCbProxyHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, proxyFullName_);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitCbProxyMethodsDcl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, proxyFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CClientProxyCodeEmitter::EmitCbProxyMethodsDcl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *%sProxyObtain(struct HdfRemoteService *remote);\n",
        interfaceName_.string(), infName_.string());
    sb.Append("\n");
    sb.AppendFormat("void %sProxyRelease(struct %s *callback);\n", infName_.string(), interfaceName_.string());
}

void CClientProxyCodeEmitter::EmitProxySourceFile()
{
    String filePath = String::Format("%s%s.c", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitProxyInclusions(sb);
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitProxyCallMethodImpl(sb);
    sb.Append("\n");
    EmitProxyMethodImpls(sb);
    sb.Append("\n");
    EmitProxyConstruction(sb);
    if (!isCallbackInterface()) {
        sb.Append("\n");
        if (isKernelCode_) {
            EmitKernelProxyGetMethodImpl(sb);
        } else {
            EmitProxyGetMethodImpl(sb);
        }
        sb.Append("\n");
        EmitProxyReleaseMethodImpl(sb);
    } else {
        sb.Append("\n");
        EmitCbProxyObtainMethodImpl(sb);
        sb.Append("\n");
        EmitCbProxyReleaseMethodImpl(sb);
    }

    sb.Append("\n");
    EmitTailExternC(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CClientProxyCodeEmitter::EmitProxyInclusions(StringBuilder& sb)
{
    if (!isCallbackInterface()) {
        EmitProxyStdlibInclusions(sb);
        sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    } else {
        sb.AppendFormat("#include \"%s.h\"\n", FileName(proxyName_).string());
        EmitProxyStdlibInclusions(sb);
    }
}

void CClientProxyCodeEmitter::EmitProxyStdlibInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <hdf_sbuf.h>\n");
    sb.Append("#include <osal_mem.h>\n");

    if (isKernelCode_) {
        sb.Append("#include <hdf_io_service_if.h>\n");
    } else {
        sb.Append("#include <servmgr_hdi.h>\n");
    }

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
            sb.Append("#include <securec.h>\n");
            break;
        }
    }
}

void CClientProxyCodeEmitter::EmitProxyCallMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("static int32_t %sProxyCall(struct %s *self, int32_t id, struct HdfSBuf *data,\n",
        infName_.string(), interfaceName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *reply)\n");
    sb.Append("{\n");

    String remoteName = isKernelCode_ ? "serv" : "remote";
    sb.Append(g_tab).AppendFormat("if (self->%s == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| self->%s->dispatcher == NULL\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("|| self->%s->dispatcher->Dispatch == NULL) {\n", remoteName.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: obj is null\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return HDF_ERR_INVALID_OBJECT;\n");
    sb.Append(g_tab).Append("}\n");

    if (isKernelCode_) {
        sb.Append(g_tab).Append("return self->serv->dispatcher->Dispatch");
        sb.Append("((struct HdfObject *)&(self->serv->object), id, data, reply);\n");
    } else {
        sb.Append(g_tab).Append("return self->remote->dispatcher->Dispatch(self->remote, id, data, reply);\n");
    }
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder& sb)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodImpl(method, sb);
        if (i + 1 != interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
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
    sb.Append(prefix + g_tab).Append("int32_t ec = HDF_FAILURE;\n");

    // Local variable definitions must precede all execution statements.
    if (isKernelCode_) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            AutoPtr<ASTType> type = param->GetType();
            if (type->GetTypeKind() == TypeKind::TYPE_ARRAY ||
                type->GetTypeKind() == TypeKind::TYPE_LIST) {
                sb.Append(prefix + g_tab).Append("uint32_t i = 0;\n");
                break;
            }
        }
    }

    sb.Append("\n");
    EmitCreateBuf(sb, prefix + g_tab);
    sb.Append("\n");

    String gotoName = GetGotLabel(method);
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_IN) {
            param->EmitCWriteVar("data", gotoName, sb, prefix + g_tab);
            sb.Append("\n");
        }
    }

    sb.Append(prefix + g_tab).AppendFormat("ec = %sCall(self, CMD_%s, data, reply);\n",
        proxyName_.string(), ConstantName(method->GetName()).string());
    sb.Append(prefix + g_tab).Append("if (ec != HDF_SUCCESS) {\n");
    sb.Append(prefix + g_tab + g_tab).Append(
        "HDF_LOGE(\"%{public}s: call failed! error code is %{public}d\", __func__, ec);\n");
    sb.Append(prefix + g_tab + g_tab).AppendFormat("goto %s;\n", gotoName.string());
    sb.Append(prefix + g_tab).Append("}\n");
    sb.Append("\n");

    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
            EmitReadProxyMethodParameter(param, "reply", gotoName, sb, prefix + g_tab);
            sb.Append("\n");
        }
    }

    EmitErrorHandle(method, "errors", true, sb, prefix);
    sb.Append(prefix).Append("finished:\n");
    EmitReleaseBuf(sb, prefix + g_tab);

    sb.Append(prefix + g_tab).Append("return ec;\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitCreateBuf(StringBuilder& sb, const String& prefix)
{
    if (isKernelCode_) {
        sb.Append(prefix).Append("struct HdfSBuf *data = HdfSBufObtainDefaultSize();\n");
        sb.Append(prefix).Append("struct HdfSBuf *reply = HdfSBufObtainDefaultSize();\n");
    } else {
        sb.Append(prefix).Append("struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);\n");
        sb.Append(prefix).Append("struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);\n");
    }

    sb.Append("\n");
    sb.Append(prefix).Append("if (data == NULL || reply == NULL) {\n");
    sb.Append(prefix + g_tab).Append("HDF_LOGE(\"%{public}s: HdfSubf malloc failed!\", __func__);\n");
    sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
    sb.Append(prefix + g_tab).Append("goto finished;\n");
    sb.Append(prefix).Append("}\n");
}

void CClientProxyCodeEmitter::EmitReleaseBuf(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("if (data != NULL) {\n");
    sb.Append(prefix + g_tab).Append("HdfSBufRecycle(data);\n");
    sb.Append(prefix).Append("}\n");
    sb.Append(prefix).Append("if (reply != NULL) {\n");
    sb.Append(prefix + g_tab).Append("HdfSBufRecycle(reply);\n");
    sb.Append(prefix).Append("}\n");
}

void CClientProxyCodeEmitter::EmitReadProxyMethodParameter(const AutoPtr<ASTParameter>& param,
    const String& parcelName, const String& gotoLabel, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    if (type->GetTypeKind() == TypeKind::TYPE_STRING) {
        String cloneName = String::Format("%sCopy", param->GetName().string());
        type->EmitCProxyReadVar(parcelName, cloneName, false, gotoLabel, sb, prefix);
        String leftVar = String::Format("*%s", param->GetName().string());
        if (isKernelCode_) {
            sb.Append("\n");
            sb.Append(prefix).AppendFormat("%s = (char*)OsalMemCalloc(strlen(%s) + 1);\n",
                leftVar.string(), cloneName.string());
            sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", leftVar.string());
            sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
            sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
            sb.Append(prefix).Append("}\n\n");
            sb.Append(prefix).AppendFormat("if (strcpy_s(%s, (strlen(%s) + 1), %s) != HDF_SUCCESS) {\n",
                leftVar.string(), cloneName.string(), cloneName.string());
            sb.Append(prefix + g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n",
                leftVar.string());
            sb.Append(prefix + g_tab).Append("ec = HDF_ERR_INVALID_PARAM;\n");
            sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
            sb.Append(prefix).Append("}\n");
        } else {
            sb.Append(prefix).AppendFormat("%s = strdup(%s);\n", leftVar.string(), cloneName.string());
        }
    } else if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        String name = String::Format("*%s", param->GetName().string());
        sb.Append(prefix).AppendFormat("%s = (%s*)OsalMemAlloc(sizeof(%s));\n",
            name.string(), type->EmitCType().string(), type->EmitCType().string());
        sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", name.string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
        sb.Append(prefix).Append("}\n");
        type->EmitCProxyReadVar(parcelName, name, false, gotoLabel, sb, prefix);
    } else if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
        String cpName = String::Format("%sCp", param->GetName().string());
        type->EmitCProxyReadVar(parcelName, cpName, false, gotoLabel, sb, prefix);
        sb.Append(prefix).AppendFormat("*%s = (%s*)OsalMemAlloc(sizeof(%s));\n",
            param->GetName().string(), type->EmitCType().string(), type->EmitCType().string());
        sb.Append(prefix).AppendFormat("if (*%s == NULL) {\n", param->GetName().string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
        sb.Append(prefix).Append("}\n");
        sb.Append(prefix).AppendFormat("(void)memcpy_s(*%s, sizeof(%s), %s, sizeof(%s));\n",
            param->GetName().string(), type->EmitCType().string(), cpName.string(), type->EmitCType().string());
    } else {
        type->EmitCProxyReadVar(parcelName, param->GetName(), false, gotoLabel, sb, prefix);
    }
}

String CClientProxyCodeEmitter::GetGotLabel(const AutoPtr<ASTMethod>& method)
{
    String labelName = "finished";
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        AutoPtr<ASTType> paramType = param->GetType();
        if (param->GetAttribute() == ParamAttr::PARAM_OUT &&
            (paramType->GetTypeKind() == TypeKind::TYPE_STRING
            || paramType->GetTypeKind() == TypeKind::TYPE_ARRAY
            || paramType->GetTypeKind() == TypeKind::TYPE_LIST
            || paramType->GetTypeKind() == TypeKind::TYPE_STRUCT
            || paramType->GetTypeKind() == TypeKind::TYPE_UNION)) {
            labelName = "errors";
            break;
        }
    }

    return labelName;
}

void CClientProxyCodeEmitter::EmitProxyConstruction(StringBuilder& sb)
{
    String objName;
    if (!isCallbackInterface()) {
        objName = "inst";
        sb.AppendFormat("static void %sConstruct(struct %s *%s)\n",
            infName_.string(), interfaceName_.string(), objName.string());
    } else {
        objName = "callback";
        sb.AppendFormat("static void %sProxyConstruct(struct %s *%s)\n",
            infName_.string(), interfaceName_.string(), objName.string());
    }
    sb.Append("{\n");

    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(g_tab).AppendFormat("%s->%s = %sProxy%s;\n",
            objName.string(), method->GetName().string(), infName_.string(), method->GetName().string());
    }

    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyGetMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *Hdi%sGet()\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();\n");
    sb.Append(g_tab).Append("if (serviceMgr == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: HDIServiceManager not found!\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).Append("struct HdfRemoteService *remote = ");
    sb.AppendFormat("serviceMgr->GetService(serviceMgr, \"%sService\");\n", infName_.string());
    sb.Append(g_tab).Append("HDIServiceManagerRelease(serviceMgr);\n");
    sb.Append(g_tab).Append("if (remote == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: %sService not found!\", __func__);\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).AppendFormat("struct %s *%sClient = (struct %s *)OsalMemAlloc(sizeof(struct %s));\n",
        interfaceName_.string(), infName_.string(), interfaceName_.string(), interfaceName_.string());
    sb.Append(g_tab).AppendFormat("if (%sClient == NULL) {\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: malloc %s instance failed!\", __func__);\n", interfaceName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HdfRemoteServiceRecycle(remote);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).AppendFormat("%sClient->remote = remote;\n", infName_.string());
    sb.Append(g_tab).AppendFormat("%sConstruct(%sClient);\n", infName_.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("return %sClient;\n", infName_.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitKernelProxyGetMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *Hdi%sGet()\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct HdfIoService *serv = ");
    sb.AppendFormat("HdfIoServiceBind(\"%s\");\n", infName_.string());
    sb.Append(g_tab).Append("if (serv == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: %sService not found!\", __func__);\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).AppendFormat("struct %s *%sClient = (struct %s *)OsalMemAlloc(sizeof(struct %s));\n",
        interfaceName_.string(), infName_.string(), interfaceName_.string(), interfaceName_.string());
    sb.Append(g_tab).AppendFormat("if (%sClient == NULL) {\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: malloc %s instance failed!\", __func__);\n", interfaceName_.string());
    sb.Append(g_tab).Append(g_tab).Append("HdfIoServiceRecycle(serv);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");

    sb.Append(g_tab).AppendFormat("%sClient->serv = serv;\n", infName_.string());
    sb.Append(g_tab).AppendFormat("%sConstruct(%sClient);\n", infName_.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("return %sClient;\n", infName_.string());
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitProxyReleaseMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("void Hdi%sRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (instance == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");

    if (isKernelCode_) {
        sb.Append(g_tab).Append("HdfIoServiceRecycle(instance->serv);\n");
    } else {
        sb.Append(g_tab).Append("HdfRemoteServiceRecycle(instance->remote);\n");
    }
    sb.Append(g_tab).Append("OsalMemFree(instance);\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitCbProxyObtainMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *%sProxyObtain(struct HdfRemoteService *remote)\n",
        interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct %s *callback = (struct %s*)OsalMemAlloc(sizeof(struct %s));\n",
        interfaceName_.string(), interfaceName_.string(), interfaceName_.string());
    sb.Append(g_tab).Append("if (callback == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: OsalMemAlloc failed!\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("callback->remote = remote;\n");
    sb.Append(g_tab).AppendFormat("%sProxyConstruct(callback);\n", infName_.string());
    sb.Append(g_tab).Append("return callback;\n");
    sb.Append("}\n");
}

void CClientProxyCodeEmitter::EmitCbProxyReleaseMethodImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sProxyRelease(struct %s *callback)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (callback == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).Append("OsalMemFree(callback);\n");
    sb.Append("}\n");
}
} // namespace HDI
} // namespace OHOS