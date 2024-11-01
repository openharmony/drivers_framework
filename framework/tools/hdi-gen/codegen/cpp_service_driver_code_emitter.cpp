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
bool CppServiceDriverCodeEmitter::ResolveDirectory(const String &targetDirectory)
{
    if (ast_->GetASTFileType() != ASTFileType::AST_IFACE) {
        return false;
    }

    directory_ = GetFileParentPath(targetDirectory);
    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppServiceDriverCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppServiceDriverCodeEmitter::EmitCode()
{
    // the callback interface have no driver file.
    if (!interface_->IsSerializable()) {
        EmitDriverSourceFile();
    }
}

void CppServiceDriverCodeEmitter::EmitDriverSourceFile()
{
    String filePath = File::AdapterPath(String::Format("%s/%s.cpp", directory_.string(),
        FileName(baseName_ + "Driver").string()));
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitDriverInclusions(sb);
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

void CppServiceDriverCodeEmitter::EmitDriverInclusions(StringBuilder &sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base");
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log");
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_device_desc");
    headerFiles.emplace(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_sbuf_ipc");
    headerFiles.emplace(HeaderFileType::OWN_MODULE_HEADER_FILE, EmitVersionHeaderName(stubName_));

    for (const auto &file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceDriverCodeEmitter::EmitDriverUsings(StringBuilder &sb)
{
    String nspace = EmitPackageToNameSpace(interface_->GetNamespace()->ToString());
    sb.AppendFormat("using namespace %s;\n", nspace.string());
}

void CppServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder &sb)
{
    sb.AppendFormat("struct Hdf%sHost {\n", baseName_.string());
    sb.Append(TAB).Append("struct IDeviceIoService ioService;\n");
    sb.Append(TAB).Append("OHOS::sptr<OHOS::IRemoteObject> stub;\n");
    sb.Append("};\n");
}

void CppServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder &sb)
{
    String objName = String::Format("hdf%sHost", baseName_.string());
    sb.AppendFormat("static int32_t %sDriverDispatch(", baseName_.string());
    sb.Append("struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data,\n");
    sb.Append(TAB).Append("struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("auto *%s = CONTAINER_OF(", objName.string());
    sb.AppendFormat("client->device->service, struct Hdf%sHost, ioService);\n\n", baseName_.string());

    sb.Append(TAB).Append("OHOS::MessageParcel *dataParcel = nullptr;\n");
    sb.Append(TAB).Append("OHOS::MessageParcel *replyParcel = nullptr;\n");
    sb.Append(TAB).Append("OHOS::MessageOption option;\n\n");

    sb.Append(TAB).Append("if (SbufToParcel(data, &dataParcel) != HDF_SUCCESS) {\n");
    sb.Append(TAB).Append(TAB).Append("HDF_LOGE(\"%{public}s:invalid data sbuf object to dispatch\", __func__);\n");
    sb.Append(TAB).Append(TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(TAB).Append("}\n");
    sb.Append(TAB).Append("if (SbufToParcel(reply, &replyParcel) != HDF_SUCCESS) {\n");
    sb.Append(TAB).Append(TAB).Append("HDF_LOGE(\"%{public}s:invalid reply sbuf object to dispatch\", __func__);\n");
    sb.Append(TAB).Append(TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(TAB).Append("}\n\n");

    sb.Append(TAB).AppendFormat(
        "return %s->stub->SendRequest(cmdId, *dataParcel, *replyParcel, option);\n", objName.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverInit(StringBuilder &sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", baseName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter\");\n", baseName_.string());
    sb.Append(TAB).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverBind(StringBuilder &sb)
{
    String objName = String::Format("hdf%sHost", baseName_.string());
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", baseName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter\");\n\n", baseName_.string());

    sb.Append(TAB).AppendFormat("auto *%s = new (std::nothrow) Hdf%sHost;\n", objName.string(), baseName_.string());
    sb.Append(TAB).AppendFormat("if (%s == nullptr) {\n", objName.string());
    sb.Append(TAB).Append(TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: failed to create create Hdf%sHost object\", __func__);\n", baseName_.string());
    sb.Append(TAB).Append(TAB).Append("return HDF_FAILURE;\n");
    sb.Append(TAB).Append("}\n\n");

    sb.Append(TAB).AppendFormat("%s->ioService.Dispatch = %sDriverDispatch;\n", objName.string(), baseName_.string());
    sb.Append(TAB).AppendFormat("%s->ioService.Open = NULL;\n", objName.string());
    sb.Append(TAB).AppendFormat("%s->ioService.Release = NULL;\n\n", objName.string());

    sb.Append(TAB).AppendFormat("auto serviceImpl = %s::Get(true);\n", interfaceName_.string());
    sb.Append(TAB).Append("if (serviceImpl == nullptr) {\n");
    sb.Append(TAB).Append(TAB).Append("HDF_LOGE(\"%{public}s: failed to get of implement service\", __func__);\n");
    sb.Append(TAB).Append(TAB).AppendFormat("delete %s;\n", objName.string());
    sb.Append(TAB).Append(TAB).Append("return HDF_FAILURE;\n");
    sb.Append(TAB).Append("}\n\n");

    sb.Append(TAB).AppendFormat("%s->stub = OHOS::HDI::ObjectCollector::GetInstance().", objName.string());
    sb.Append("GetOrNewObject(serviceImpl,\n");
    sb.Append(TAB).Append(TAB).AppendFormat("%s::GetDescriptor());\n", interfaceName_.string());
    sb.Append(TAB).AppendFormat("if (%s->stub == nullptr) {\n", objName.string());
    sb.Append(TAB).Append(TAB).Append("HDF_LOGE(\"%{public}s: failed to get stub object\", __func__);\n");
    sb.Append(TAB).Append(TAB).AppendFormat("delete %s;\n", objName.string());
    sb.Append(TAB).Append(TAB).AppendFormat("return HDF_FAILURE;\n");
    sb.Append(TAB).Append("}\n\n");

    sb.Append(TAB).AppendFormat("deviceObject->service = &%s->ioService;\n", objName.string());
    sb.Append(TAB).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder &sb)
{
    String objName = String::Format("hdf%sHost", baseName_.string());
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)", baseName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverRelease enter\");\n", baseName_.string());
    sb.Append(TAB).AppendFormat("if (deviceObject->service == nullptr) {\n");
    sb.Append(TAB).Append(TAB).AppendFormat("HDF_LOGE(\"Hdf%sDriverRelease not initted\");\n", baseName_.string());
    sb.Append(TAB).Append(TAB).AppendFormat("return;\n");
    sb.Append(TAB).Append("}\n\n");
    sb.Append(TAB).AppendFormat("auto *%s = CONTAINER_OF(", objName.string());
    sb.AppendFormat("deviceObject->service, struct Hdf%sHost, ioService);\n", baseName_.string());
    sb.Append(TAB).AppendFormat("delete %s;\n", objName.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder &sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", baseName_.ToLowerCase().string());
    sb.Append(TAB).Append(".moduleVersion = 1,\n");
    sb.Append(TAB).AppendFormat(".moduleName = \"%s\",\n", Options::GetInstance().GetModuleName().string());
    sb.Append(TAB).AppendFormat(".Bind = Hdf%sDriverBind,\n", baseName_.string());
    sb.Append(TAB).AppendFormat(".Init = Hdf%sDriverInit,\n", baseName_.string());
    sb.Append(TAB).AppendFormat(".Release = Hdf%sDriverRelease,\n", baseName_.string());
    sb.Append("};\n");
    sb.Append("\n");
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("extern \"C\" {\n");
    sb.Append("#endif\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);\n", baseName_.ToLowerCase().string());
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("}\n");
    sb.Append("#endif\n");
}
} // namespace HDI
} // namespace OHOS