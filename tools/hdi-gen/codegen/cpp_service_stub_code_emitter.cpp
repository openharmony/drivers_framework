/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_stub_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CppServiceStubCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppServiceStubCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CppServiceStubCodeEmitter::EmitCode()
{
    EmitStubHeaderFile();
    EmitStubSourceFile();
}

void CppServiceStubCodeEmitter::EmitStubHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, stubFullName_);
    sb.Append("\n");
    EmitStubHeaderInclusions(sb);
    sb.Append("\n");

    if (!isCallbackInterface()) {
        EmitStubDecl(sb);
    } else {
        EmitCbStubDecl(sb);
    }
    sb.Append("\n");
    EmitTailMacro(sb, stubFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceStubCodeEmitter::EmitStubHeaderInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;

    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_MODULE_HEADER_FILE, FileName(interfaceName_)));
    GetHeaderOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceStubCodeEmitter::GetHeaderOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "message_parcel"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "message_option"));
    if (isCallbackInterface()) {
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "iremote_stub"));
    }
}

void CppServiceStubCodeEmitter::EmitStubDecl(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitStubUsingNamespace(sb);
    sb.Append("\n");
    sb.AppendFormat("class %s {\n", stubName_.string());
    EmitStubBody(sb, g_tab);
    sb.Append("};\n");

    sb.Append("\n");
    EmitEndNamespace(sb);

    sb.Append("\n");
    EmitStubExternalsMethodsDel(sb);
}

void CppServiceStubCodeEmitter::EmitCbStubDecl(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitStubUsingNamespace(sb);
    sb.Append("\n");
    sb.AppendFormat("class %s : public IRemoteStub<%s> {\n", stubName_.string(), interfaceName_.string());
    EmitCbStubBody(sb, g_tab);
    sb.Append("};\n");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
}

void CppServiceStubCodeEmitter::EmitStubUsingNamespace(StringBuilder& sb)
{
    sb.Append("using namespace OHOS;\n");
}

void CppServiceStubCodeEmitter::EmitStubBody(StringBuilder& sb, const String& prefix)
{
    sb.Append("public:\n");
    EmitStubDestruction(sb, prefix);
    sb.Append("\n");
    EmitStubMethodDecls(sb, prefix);
    sb.Append("\n");
    EmitStubOnRequestMethodDecl(sb, prefix);
    sb.Append("\n");
    EmitStubMembers(sb, prefix);
}

void CppServiceStubCodeEmitter::EmitCbStubBody(StringBuilder& sb, const String& prefix)
{
    sb.Append("public:\n");
    EmitStubDestruction(sb, prefix);
    sb.Append("\n");
    EmitCbStubOnRequestDecl(sb, prefix);
    EmitStubMethodDecls(sb, prefix);
}

void CppServiceStubCodeEmitter::EmitStubDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() {}\n", stubName_.string());
}

void CppServiceStubCodeEmitter::EmitCbStubOnRequestDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,\n");
    sb.Append(prefix + g_tab).Append("MessageOption &option) override;\n");
}

void CppServiceStubCodeEmitter::EmitStubMethodDecls(StringBuilder& sb, const String& prefix)
{
    if (interface_->GetMethodNumber() > 0) {
        if (isCallbackInterface()) {
            sb.Append("private:\n");
        }

        for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
            AutoPtr<ASTMethod> method = interface_->GetMethod(i);
            EmitStubMethodDecl(method, sb, prefix);
            if (i + 1 < interface_->GetMethodNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CppServiceStubCodeEmitter::EmitStubMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    String dataName = "data_";
    String replyName = "reply_";
    String optionName = "option_";
    sb.Append(prefix).AppendFormat("int32_t %s%s(MessageParcel& %s, MessageParcel& %s, MessageOption& %s);\n",
        stubName_.string(), method->GetName().string(), dataName.string(), replyName.string(), optionName.string());
}

void CppServiceStubCodeEmitter::EmitStubOnRequestMethodDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %sOnRemoteRequest(int cmdId, MessageParcel& data, MessageParcel& reply,\n",
        stubName_.string());
    sb.Append(prefix).Append(g_tab).Append("MessageOption& option);\n");
}

void CppServiceStubCodeEmitter::EmitStubMembers(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("void *dlHandler;\n");
    sb.Append(prefix).AppendFormat("%s *service;\n", interfaceName_.string());
}

void CppServiceStubCodeEmitter::EmitStubExternalsMethodsDel(StringBuilder& sb)
{
    sb.AppendFormat("void *%sInstance();\n", stubName_.string());
    sb.Append("\n");
    sb.AppendFormat("void %sRelease(void *obj);\n", stubName_.string());
    sb.Append("\n");
    sb.AppendFormat(
        "int32_t %sServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf* data, struct HdfSBuf* reply);\n",
        infName_.string());
}

void CppServiceStubCodeEmitter::EmitStubSourceFile()
{
    String filePath = String::Format("%s%s.cpp", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitStubSourceInclusions(sb);
    
    if (!isCallbackInterface()) {
        sb.Append("\n");
        EmitDriverLibPath(sb);
        sb.Append("\n");
        EmitHeadExternC(sb);
        sb.Append("\n");
        EmitLibFuncTypeDef(sb);
        sb.Append("\n");
        EmitTailExternC(sb);
    }

    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        EmitStubOnRequestMethodImpl(sb, "");
    } else {
        EmitCbStubOnRequestMethodImpl(sb, "");
    }
    sb.Append("\n");
    EmitStubMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");

    if (!isCallbackInterface()) {
        EmitStubExternalsMethodsImpl(sb, "");
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceStubCodeEmitter::EmitStubSourceInclusions(StringBuilder& sb)
{
    HeaderFile::HeaderFileSet headerFiles;
    headerFiles.emplace(HeaderFile(HeaderFileType::OWN_HEADER_FILE, FileName(stubName_)));
    GetSourceOtherLibInclusions(headerFiles);

    for (const auto& file : headerFiles) {
        sb.AppendFormat("%s\n", file.ToString().string());
    }
}

void CppServiceStubCodeEmitter::GetSourceOtherLibInclusions(HeaderFile::HeaderFileSet& headerFiles)
{
    if (!isCallbackInterface()) {
        headerFiles.emplace(HeaderFile(HeaderFileType::SYSTEM_HEADER_FILE, "dlfcn"));
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_sbuf_ipc"));
        headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "securec"));
    } else {
        const AST::TypeStringMap& types = ast_->GetTypes();
        for (const auto& pair : types) {
            AutoPtr<ASTType> type = pair.second;
            if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
                headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "securec"));
                break;
            }
        }
    }

    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_base"));
    headerFiles.emplace(HeaderFile(HeaderFileType::OTHER_MODULES_HEADER_FILE, "hdf_log"));
}

void CppServiceStubCodeEmitter::EmitDriverLibPath(StringBuilder& sb)
{
    sb.Append("#ifdef __ARM64__\n");
    sb.Append("#define DRIVER_PATH \"system/lib64\"\n");
    sb.Append("#else\n");
    sb.Append("#define DRIVER_PATH \"system/lib\"\n");
    sb.Append("#endif\n");
}

void CppServiceStubCodeEmitter::EmitLibFuncTypeDef(StringBuilder& sb)
{
    sb.AppendFormat("typedef %s* (*SERVICE_CONSTRUCT_FUNC)();\n", CppFullName(interface_->GetFullName()).string());
    sb.AppendFormat("typedef void (*SERVICE_RELEASE_FUNC)(%s *obj);\n",
        CppFullName(interface_->GetFullName()).string());
}

void CppServiceStubCodeEmitter::EmitStubMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitStubMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceStubCodeEmitter::EmitStubMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    String dataName = "data_";
    String replyName = "reply_";
    String optionName = "option_";
    sb.Append(prefix).AppendFormat(
        "int32_t %s::%s%s(MessageParcel& %s, MessageParcel& %s, MessageOption& %s)\n",
        stubName_.string(), stubName_.string(), method->GetName().string(),
        dataName.string(), replyName.string(), optionName.string());
    sb.Append(prefix).Append("{\n");

    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_IN) {
            EmitReadMethodParameter(param, dataName, true, sb, prefix + g_tab);
            sb.Append("\n");
        } else {
            EmitLocalVariable(param, sb, prefix + g_tab);
            sb.Append("\n");
        }
    }

    EmitStubCallMethod(method, sb, prefix + g_tab);
    sb.Append("\n");

    if (!method->IsOneWay()) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
                EmitWriteMethodParameter(param, replyName, sb, prefix + g_tab);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix + g_tab).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubCallMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (!isCallbackInterface()) {
        sb.Append(prefix).AppendFormat("int32_t ec = service->%s(", method->GetName().string());
    } else {
        sb.Append(prefix).AppendFormat("int32_t ec = %s(", method->GetName().string());
    }
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        sb.Append(param->GetName());
        if (i + 1 < method->GetParameterNumber()) {
            sb.Append(", ");
        }
    }
    sb.Append(");\n");

    sb.Append(prefix).Append("if (ec != HDF_SUCCESS) {\n");
    sb.Append(prefix + g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s failed, error code is %%d\", __func__, ec);\n", method->GetName().string());
    sb.Append(prefix + g_tab).Append("return ec;\n");
    sb.Append(prefix).Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %s::%sOnRemoteRequest(int cmdId,\n",
        stubName_.string(), stubName_.string());
    sb.Append(prefix + g_tab).Append("MessageParcel& data, MessageParcel& reply, MessageOption& option)\n");
    sb.Append(prefix).Append("{\n");

    sb.Append(prefix + g_tab).Append("switch (cmdId) {\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + g_tab + g_tab).AppendFormat("case CMD_%s:\n", ConstantName(method->GetName()).string());
        sb.Append(prefix + g_tab + g_tab + g_tab).AppendFormat("return %sStub%s(data, reply, option);\n",
            infName_.string(), method->GetName().string());
    }

    sb.Append(prefix + g_tab + g_tab).Append("default: {\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).Append(
        "HDF_LOGE(\"%{public}s: not support cmd %{public}d\", __func__, cmdId);\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + g_tab + g_tab).Append("}\n");
    sb.Append(prefix + g_tab).Append("}\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitCbStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %s::OnRemoteRequest(uint32_t code,\n", stubName_.string());
    sb.Append(prefix + g_tab).Append("MessageParcel& data, MessageParcel& reply, MessageOption& option)\n");
    sb.Append(prefix).Append("{\n");

    sb.Append(prefix + g_tab).Append("switch (code) {\n");

    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + g_tab + g_tab).AppendFormat("case CMD_%s:\n", ConstantName(method->GetName()).string());
        sb.Append(prefix + g_tab + g_tab + g_tab).AppendFormat("return %sStub%s(data, reply, option);\n",
            infName_.string(), method->GetName().string());
    }

    sb.Append(prefix + g_tab + g_tab).Append("default: {\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).Append(
        "HDF_LOGE(\"%{public}s: not support cmd %{public}d\", __func__, code);\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).Append(
        "return IPCObjectStub::OnRemoteRequest(code, data, reply, option);\n");
    sb.Append(prefix + g_tab + g_tab).Append("}\n");
    sb.Append(prefix + g_tab).Append("}\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubExternalsMethodsImpl(StringBuilder& sb, const String& prefix)
{
    EmitStubLinkService(sb);
    sb.Append("\n");
    EmitStubInstanceMethodImpl(sb);
    sb.Append("\n");
    EmitStubReleaseMethodImpl(sb);
    sb.Append("\n");
    EmitServiceOnRemoteRequest(sb);
}

void CppServiceStubCodeEmitter::EmitStubLinkService(StringBuilder& sb)
{
    sb.Append("static void *LoadServiceHandler(const char* libFileName)\n");
    sb.Append("{\n");
    sb.Append(g_tab).Append("char path[PATH_MAX + 1] = {0};\n");
    sb.Append(g_tab).Append("char libPath[PATH_MAX + 1] = {0};\n");
    sb.Append(g_tab).Append("void *handler = NULL;\n");
    sb.Append("\n");
    sb.Append(g_tab).AppendFormat("if (snprintf_s(path, sizeof(path), sizeof(path) - 1, \"%%s/%%s\", ");
    sb.Append("DRIVER_PATH, libFileName) < 0) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: snprintf_s failed\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).Append("if (realpath(path, libPath) == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s file name invalid\", __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).Append("handler = dlopen(libPath, RTLD_LAZY);\n");
    sb.Append(g_tab).Append("if (handler == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: dlopen failed %{public}s\", ");
    sb.AppendFormat("__func__, dlerror());\n");
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append("\n");
    sb.Append(g_tab).Append("return handler;\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubInstanceMethodImpl(StringBuilder& sb)
{
    String objName = "stub";
    String libName = String::Format("lib%s.z.so", FileName(implName_).string());
    sb.AppendFormat("void *%sInstance()\n", stubName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(g_tab).Append("SERVICE_CONSTRUCT_FUNC serviceConstructFunc = nullptr;\n");
    sb.Append(g_tab).AppendFormat("%sStub *%s = new %sStub();\n", infName_.string(), objName.string(),
        infName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == nullptr) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: OsalMemAlloc %s failed!\", __func__);\n",
        objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return nullptr;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("%s->dlHandler = LoadServiceHandler(\"%s\");\n", objName.string(), libName.string());
    sb.Append(g_tab).AppendFormat("if (%s->dlHandler == nullptr) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: %s->dlHanlder is null\", __func__);\n",
        objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("delete %s;\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return nullptr;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("serviceConstructFunc = ");
    sb.AppendFormat("(SERVICE_CONSTRUCT_FUNC)dlsym(%s->dlHandler, \"%sServiceConstruct\");\n",
        objName.string(), infName_.string());
    sb.Append(g_tab).Append("if (serviceConstructFunc == nullptr) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: dlsym failed %{public}s\", __func__, dlerror());\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("dlclose(%s->dlHandler);\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("delete %s;\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return nullptr;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("%s->service = serviceConstructFunc();\n", objName.string());
    sb.Append(g_tab).AppendFormat("if (%s->service == nullptr) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: get service failed %{public}s\", ");
    sb.Append("__func__, dlerror());\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("dlclose(%s->dlHandler);\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("delete %s;\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return nullptr;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("return reinterpret_cast<void *>(stub);\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubReleaseMethodImpl(StringBuilder& sb)
{
    String objName = "stub";
    sb.AppendFormat("void %sRelease(void *obj)\n", stubName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(g_tab).Append("if (obj == nullptr) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("%sStub *%s = reinterpret_cast<%sStub *>(obj);\n", infName_.string(),
        objName.string(), infName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == nullptr) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("SERVICE_RELEASE_FUNC serviceReleaseFunc = ");
    sb.AppendFormat("(SERVICE_RELEASE_FUNC)dlsym(%s->dlHandler, \"SampleServiceRelease\");\n", objName.string());
    sb.Append(g_tab).Append("if (serviceReleaseFunc == nullptr) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: dlsym failed %{public}s\", __func__, dlerror());\n");
    sb.Append(g_tab).Append("} else {\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("serviceReleaseFunc(%s->service);\n", objName.string());
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("dlclose(%s->dlHandler);\n", objName.string());
    sb.Append(g_tab).AppendFormat("delete %s;\n", objName.string());
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitServiceOnRemoteRequest(StringBuilder& sb)
{
    String stubObjName = String::Format("%sStub", infName_.ToLowerCase().string());
    sb.AppendFormat(
        "int32_t %sServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)\n",
        infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(g_tab).AppendFormat("%s *%s = reinterpret_cast<%s *>(stub);\n",
        stubName_.string(), stubObjName.string(), stubName_.string());
    sb.Append(g_tab).Append("OHOS::MessageParcel *dataParcel = nullptr;\n");
    sb.Append(g_tab).Append("OHOS::MessageParcel *replyParcel = nullptr;\n");
    sb.Append("\n");

    sb.Append(g_tab).Append("(void)SbufToParcel(reply, &replyParcel);\n");
    sb.Append(g_tab).Append("if (SbufToParcel(data, &dataParcel) != HDF_SUCCESS) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s:invalid data sbuf object to dispatch\",");
    sb.Append(" __func__);\n");
    sb.Append(g_tab).Append(g_tab).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).Append("OHOS::MessageOption option;\n");
    sb.Append(g_tab).AppendFormat("return %s->%sOnRemoteRequest(cmdId, *dataParcel, *replyParcel, option);\n",
        stubObjName.string(), stubName_.string());
    sb.Append("}\n");
}

String CppServiceStubCodeEmitter::EmitStubServiceUsings(String nameSpace)
{
    int index = nameSpace.LastIndexOf('.');
    if (index > 0) {
        nameSpace = nameSpace.Substring(0, index);
    }
    return CppFullName(nameSpace);
}
} // namespace HDI
} // namespace OHOS