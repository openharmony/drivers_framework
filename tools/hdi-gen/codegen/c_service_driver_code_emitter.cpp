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
#include "util/options.h"

namespace OHOS {
namespace HDI {
bool CServiceDriverCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() != ASTFileType::AST_IFACE) {
        return false;
    }

    directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
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
    sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
}

void CServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct Hdf%sHost {\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct IDeviceIoService ioservice;\n");
    sb.Append(g_tab).Append("void *instance;\n");
    sb.Append("};\n");
}

void CServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder& sb)
{
    String hostName = infName_.ToLowerCase() + "Host";
    sb.AppendFormat("static int32_t %sDriverDispatch(struct HdfDeviceIoClient *client, int cmdId,\n",
        infName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(\n",
        infName_.string(), hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("client->device->service, struct Hdf%sHost, ioservice);\n",
        infName_.string());
    sb.Append(g_tab).AppendFormat("return %sServiceOnRemoteRequest(%s->instance, cmdId, data, reply);\n",
        infName_.string(), hostName.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverInit(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter.\");\n", infName_.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverBind(StringBuilder& sb)
{
    String hostName = infName_.ToLowerCase() + "Host";
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter.\");\n", infName_.string());
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = (struct Hdf%sHost *)OsalMemAlloc(\n",
        infName_.string(), hostName.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("sizeof(struct Hdf%sHost));\n", infName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == NULL) {\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"Hdf%sDriverBind OsalMemAlloc Hdf%sHost failed!\");\n", infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("%s->ioservice.Dispatch = %sDriverDispatch;\n",
        hostName.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Open = NULL;\n", hostName.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Release = NULL;\n", hostName.string());
    sb.Append(g_tab).AppendFormat("%s->instance = %sStubGetInstance();\n", hostName.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("if (%s->instance == NULL) {\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(%s);\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("deviceObject->service = &%s->ioservice;\n", hostName.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder& sb)
{
    String hostName = infName_.ToLowerCase() + "Host";
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverRelease enter.\");\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(", infName_.string(), hostName.string());
    sb.AppendFormat("deviceObject->service, struct Hdf%sHost, ioservice);\n",
        infName_.string());
    sb.Append(g_tab).AppendFormat("%sStubRelease(%s->instance);\n", infName_.string(), hostName.string());
    sb.Append(g_tab).AppendFormat("OsalMemFree(%s);\n", hostName.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", infName_.ToLowerCase().string());
    sb.Append(g_tab).Append(".moduleVersion = 1,\n");
    sb.Append(g_tab).AppendFormat(".moduleName = \"%s\",\n",
        Options::GetInstance().GetModeName().string());
    sb.Append(g_tab).AppendFormat(".Bind = Hdf%sDriverBind,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Init = Hdf%sDriverInit,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Release = Hdf%sDriverRelease,\n", infName_.string());
    sb.Append("};\n\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);", infName_.ToLowerCase().string());
}
} // namespace HDI
} // namespace OHOS