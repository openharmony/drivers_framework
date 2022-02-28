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

    directory_ = GetFilePath(targetDirectory);
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
    String filePath = String::Format("%s/%s.c", directory_.string(), FileName(baseName_ + "Driver").string());
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
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, EmitVersionHeaderName(stubName_)));
    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, EmitVersionHeaderName(implName_)));
    GetDriverSourceOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CServiceDriverCodeEmitter::GetDriverSourceOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "osal_mem"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_device_desc"));
}

void CServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct Hdf%sHost {\n", baseName_.string());
    sb.Append(g_tab).AppendFormat("struct IDeviceIoService ioservice;\n");
    sb.Append(g_tab).AppendFormat("struct %s *service;\n", interfaceName_.string());
    sb.Append("};\n");
}

void CServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder& sb)
{
    String hostName = baseName_.ToLowerCase() + "Host";
    sb.AppendFormat("static int32_t %sDriverDispatch(struct HdfDeviceIoClient *client, int cmdId,\n",
        baseName_.string());
    sb.Append(g_tab).Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(\n",
        baseName_.string(), hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("client->device->service, struct Hdf%sHost, ioservice);\n",
        baseName_.string());
    sb.Append(g_tab).AppendFormat("return %sServiceOnRemoteRequest(%s->service, cmdId, data, reply);\n",
        baseName_.string(), hostName.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverInit(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", baseName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter.\");\n", baseName_.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverBind(StringBuilder& sb)
{
    String hostName = baseName_.ToLowerCase() + "Host";
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", baseName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter.\");\n", baseName_.string());
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = (struct Hdf%sHost *)OsalMemAlloc(\n",
        baseName_.string(), hostName.string(), baseName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("sizeof(struct Hdf%sHost));\n", baseName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == NULL) {\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"Hdf%sDriverBind OsalMemAlloc Hdf%sHost failed!\");\n", baseName_.string(), baseName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("%s->ioservice.Dispatch = %sDriverDispatch;\n",
        hostName.string(), baseName_.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Open = NULL;\n", hostName.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Release = NULL;\n", hostName.string());
    sb.Append(g_tab).AppendFormat("%s->service = %sStubGetInstance();\n", hostName.string(), baseName_.string());
    sb.Append(g_tab).AppendFormat("if (%s->service == NULL) {\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(%s);\n", hostName.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("%sServiceConstruct(%s->service);\n", baseName_.string(),  hostName.string());
    sb.Append(g_tab).AppendFormat("deviceObject->service = &%s->ioservice;\n", hostName.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder& sb)
{
    String hostName = baseName_.ToLowerCase() + "Host";
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)\n", baseName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverRelease enter.\");\n", baseName_.string());
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(", baseName_.string(), hostName.string());
    sb.AppendFormat("deviceObject->service, struct Hdf%sHost, ioservice);\n",
        baseName_.string());
    sb.Append(g_tab).AppendFormat("%sStubRelease(%s->service);\n", baseName_.string(), hostName.string());
    sb.Append(g_tab).AppendFormat("OsalMemFree(%s);\n", hostName.string());
    sb.Append("}\n");
}

void CServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", baseName_.ToLowerCase().string());
    sb.Append(g_tab).Append(".moduleVersion = 1,\n");
    sb.Append(g_tab).AppendFormat(".moduleName = \"%s\",\n",
        Options::GetInstance().GetModuleName().string());
    sb.Append(g_tab).AppendFormat(".Bind = Hdf%sDriverBind,\n", baseName_.string());
    sb.Append(g_tab).AppendFormat(".Init = Hdf%sDriverInit,\n", baseName_.string());
    sb.Append(g_tab).AppendFormat(".Release = Hdf%sDriverRelease,\n", baseName_.string());
    sb.Append("};\n\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);", baseName_.ToLowerCase().string());
}
} // namespace HDI
} // namespace OHOS