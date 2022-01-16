/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_driver_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"
#include "util/options.h"

namespace OHOS {
namespace HDI {
bool CppServiceDriverCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() != ASTFileType::AST_IFACE) {
        return false;
    }

    directory_ = GetFilePath(targetDirectory);
    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppServiceDriverCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppServiceDriverCodeEmitter::EmitCode()
{
    // the callback interface have no driver file.
    if (!isCallbackInterface()) {
        EmitDriverSourceFile();
    }
}

void CppServiceDriverCodeEmitter::EmitDriverSourceFile()
{
    String filePath = String::Format("%s/%s.cpp", directory_.string(), FileName(infName_ + "Driver").string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitDriverIncluions(sb);
    sb.Append("\n");
    EmitDriverUsings(sb);
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

void CppServiceDriverCodeEmitter::EmitDriverIncluions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "osal_mem"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_device_desc"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_sbuf_ipc"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(implName_)));

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceDriverCodeEmitter::EmitDriverUsings(StringBuilder& sb)
{
    String nspace(interface_->GetNamespace()->ToString());
    int index = nspace.LastIndexOf('.');
    if (index > 0) {
        nspace = nspace.Substring(0, index);
    }

    String fullName = CppFullName(nspace);
    sb.AppendFormat("using namespace %s;\n", fullName.string());
}

void CppServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct Hdf%sHost {\n", infName_.string());
    sb.Append(g_tab).Append("struct IDeviceIoService ioservice;\n");
    sb.Append(g_tab).AppendFormat("%s *service;\n", implName_.string());
    sb.Append("};\n");
}

void CppServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder& sb)
{
    String objName = String::Format("hdf%sHost", infName_.string());
    sb.AppendFormat("static int32_t %sDriverDispatch(", infName_.string());
    sb.Append("struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data,\n");
    sb.Append(g_tab).Append("struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(", infName_.string(), objName.string());
    sb.AppendFormat("client->device->service, struct Hdf%sHost, ioservice);\n\n", infName_.string());

    sb.Append(g_tab).Append("OHOS::MessageParcel *dataParcel = nullptr;\n");
    sb.Append(g_tab).Append("OHOS::MessageParcel *replyParcel = nullptr;\n");
    sb.Append(g_tab).Append("OHOS::MessageOption option;\n\n");

    sb.Append(g_tab).Append("(void)SbufToParcel(reply, &replyParcel);\n");
    sb.Append(g_tab).Append("if (SbufToParcel(data, &dataParcel) != HDF_SUCCESS) {\n");
    sb.Append(g_tab).Append(g_tab).Append(
        "HDF_LOGE(\"%{public}s:invalid data sbuf object to dispatch\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(g_tab).Append("}\n\n");

    sb.Append(g_tab).AppendFormat("return %s->service->OnRemoteRequest(cmdId, *dataParcel, *replyParcel, option);\n",
        objName.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverInit(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter\");\n", infName_.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverBind(StringBuilder& sb)
{
    String objName = String::Format("hdf%sHost", infName_.string());
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter\");\n\n", infName_.string());

    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = (struct Hdf%sHost *)OsalMemAlloc(\n",
        infName_.string(), objName.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("sizeof(struct Hdf%sHost));\n", infName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == nullptr) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"Hdf%sDriverBind OsalMemAlloc Hdf%sHost failed!\");\n",
        infName_.string(), infName_.string());
    sb.Append(g_tab).Append(g_tab).Append("return HDF_FAILURE;\n");
    sb.Append(g_tab).Append("}\n\n");

    sb.Append(g_tab).AppendFormat("%s->ioservice.Dispatch = %sDriverDispatch;\n",
        objName.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Open = NULL;\n", objName.string());
    sb.Append(g_tab).AppendFormat("%s->ioservice.Release = NULL;\n", objName.string());
    sb.Append(g_tab).AppendFormat("%s->service = new %s();\n\n", objName.string(), implName_.string());

    sb.Append(g_tab).AppendFormat("deviceObject->service = &%s->ioservice;\n", objName.string());
    sb.Append(g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder& sb)
{
    String objName = String::Format("hdf%sHost", infName_.string());
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)", infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("HDF_LOGI(\"Hdf%sDriverRelease enter\");\n\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct Hdf%sHost *%s = CONTAINER_OF(", infName_.string(), objName.string());
    sb.AppendFormat("deviceObject->service, struct Hdf%sHost, ioservice);\n", infName_.string());
    sb.Append(g_tab).AppendFormat("delete %s->service;\n", objName.string());
    sb.Append(g_tab).AppendFormat("OsalMemFree(%s);\n", objName.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", infName_.ToLowerCase().string());
    sb.Append(g_tab).Append(".moduleVersion = 1,\n");
    sb.Append(g_tab).AppendFormat(".moduleName = \"%s\",\n",
        Options::GetInstance().GetModuleName().string());
    sb.Append(g_tab).AppendFormat(".Bind = Hdf%sDriverBind,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Init = Hdf%sDriverInit,\n", infName_.string());
    sb.Append(g_tab).AppendFormat(".Release = Hdf%sDriverRelease,\n", infName_.string());
    sb.Append("};\n");
    sb.Append("\n");
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("extern \"C\" {\n");
    sb.Append("#endif\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);\n", infName_.ToLowerCase().string());
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("}\n");
    sb.Append("#endif\n");
}
} // namespace HDI
} // namespace OHOS