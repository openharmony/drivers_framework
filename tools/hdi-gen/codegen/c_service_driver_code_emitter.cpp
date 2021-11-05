/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_service_driver_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CServiceDriverCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() != ASTFileType::AST_IFACE) {
        return false;
    }

    directory_ = File::AdapterPath(String::Format("%s/%s/server/", targetDirectory.string(),
        FileName(ast_->GetPackageName()).string()));
    if (!File::CreateParentDir(directory_)) {
        Logger::E("CServiceDriverCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CServiceDriverCodeEmitter::EmitCode()
{
    // the callback interface have no driver file.
    if (!isCallbackInterface()) {
        EmitDriverSourceFile();
    }
}

void CServiceDriverCodeEmitter::EmitDriverSourceFile()
{
    String filePath = String::Format("%s%s.c", directory_.string(), FileName(infName_ + "Driver").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitDriverIncluions(sb);
    sb.Append("\n");
    EmitDriverServiceDecl(sb);
    sb.Append("\n");
    EmitDriverDispatch(sb);
    sb.Append("\n");
    EmitDriverInit(sb);
    sb.Append("\n");
    EmitDriverBind(sb);
    sb.Append("\n");
    EmitDriverRelease(sb);
    sb.Append("\n");
    EmitDriverEntryDefinition(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceDriverCodeEmitter::EmitDriverIncluions(StringBuilder& sb)
{
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <osal_mem.h>\n");
    sb.Append("#include <hdf_device_desc.h>\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
}

void CServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct Hdf%sService {\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct IDeviceIoService ioservice;\n");
    sb.Append(g_tab).Append("void *instance;\n");
    sb.Append("};\n");
}

void CServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder& sb)
{
    sb.AppendFormat("static int32_t %sServiceDispatch(struct HdfDeviceIoClient *client, int cmdId,\n",
        infName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sService *hdf%sService = CONTAINER_OF(\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("client->device->service, struct Hdf%sService, ioservice);\n",
        infName_.string());
    sb.Append(g_tab).AppendFormat("return %sServiceOnRemoteRequest(hdf%sService->instance, cmdId, data, reply);\n",
        infName_.string(), infName_.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverInit(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter, new hdi impl.\");\n", infName_.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverBind(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter.\");\n", infName_.string());
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sService *hdf%sService = (struct Hdf%sService *)OsalMemAlloc(\n",
        infName_.string(), infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("sizeof(struct Hdf%sService));\n", infName_.string());
    sb.Append(g_tab).AppendFormat("if (hdf%sService == NULL) {\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"Hdf%sDriverBind OsalMemAlloc Hdf%sService failed!\");\n", infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("hdf%sService->ioservice.Dispatch = %sServiceDispatch;\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("hdf%sService->ioservice.Open = NULL;\n", infName_.string());
    sb.Append(g_tab).AppendFormat("hdf%sService->ioservice.Release = NULL;\n", infName_.string());
    sb.Append(g_tab).AppendFormat("hdf%sService->instance = Hdi%sInstance();\n", infName_.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("if (hdf%sService->instance == NULL) {\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(hdf%sService);\n", infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("deviceObject->service = &hdf%sService->ioservice;\n", infName_.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder& sb)
{
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sService *hdf%sService = CONTAINER_OF(\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("deviceObject->service, struct Hdf%sService, ioservice);\n",
        infName_.string());
    sb.Append(g_tab).AppendFormat("Hdi%sRelease(hdf%sService->instance);\n", infName_.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("OsalMemFree(hdf%sService);\n", infName_.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", infName_.ToLowerCase().string());
    sb.Append(g_tab).Append(".moduleVersion = 1,\n");
    sb.Append(g_tab).Append(".moduleName = \"sample\", /* please change the moduleName */\n");
    sb.Append(g_tab).AppendFormat(".Bind = Hdf%sDriverBind,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Init = Hdf%sDriverInit,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Release = Hdf%sDriverRelease,\n", infName_.string());
    sb.Append("};\n\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);", infName_.ToLowerCase().string());
}
} // namespace HDI
} // namespace OHOS