/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_service_stub_code_emitter.h"

#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CServiceStubCodeEmitter::ResolveDirectory(const String& targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE ||
        ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = File::AdapterPath(String::Format("%s/%s/", targetDirectory.string(),
            FileName(ast_->GetPackageName()).string()));
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CServiceStubCodeEmitter", "Create '%s' failed!", directory_.string());
        return false;
    }

    return true;
}

void CServiceStubCodeEmitter::EmitCode()
{

    EmitCbServiceStubHeaderFile();
    EmitServiceStubSourceFile();
}

void CServiceStubCodeEmitter::EmitCbServiceStubHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, stubFullName_);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitCbServiceStubMethodsDcl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, stubFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceStubCodeEmitter::EmitCbServiceStubMethodsDcl(StringBuilder& sb)
{
    if (!isCallbackInterface()) {
        sb.AppendFormat("int32_t %sServiceOnRemoteRequest(void *service, int cmdId, ", infName_.string());
        sb.Append("struct HdfSBuf *data, struct HdfSBuf *reply);\n");
        sb.Append("\n");
    }
    sb.AppendFormat("struct %s* %sStubGetInstance(void);\n", interfaceName_.string(), infName_.string());
    sb.Append("\n");
    sb.AppendFormat("void %sStubRelease(struct %s *instance);\n", infName_.string(), interfaceName_.string());
}

void CServiceStubCodeEmitter::EmitServiceStubSourceFile()
{
    String filePath = String::Format("%s%s.c", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitServiceStubInclusions(sb);
    sb.Append("\n");

    if (!isKernelCode_) {
        EmitDriverLibPath(sb);
        sb.Append("\n");
        EmitServConstructTypedef(sb);
        sb.Append("\n");
        EmitCbStubDefinitions(sb);
        sb.Append("\n");
    }

    EmitServiceStubMethodImpls(sb, "");
    sb.Append("\n");
    EmitServiceStubOnRequestMethodImpl(sb, "");
    sb.Append("\n");
    if (isKernelCode_) {
        EmitKernelStubGetMethodImpl(sb);
    } else {
        EmitStubLinkService(sb);
        sb.Append("\n");
        EmitStubGetMethodImpl(sb);
    }

    sb.Append("\n");
    if (isKernelCode_) {
        EmitKernelStubReleaseImpl(sb);
    } else {
        EmitStubReleaseImpl(sb);
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceStubCodeEmitter::EmitServiceStubInclusions(StringBuilder& sb)
{
    if (!isCallbackInterface()) {
        EmitServiceStubStdlibInclusions(sb);
        sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
        if (isKernelCode_) {
            sb.AppendFormat("#include \"%s.h\"\n", FileName(implName_).string());
        } else {
            for (const auto& importPair : ast_->GetImports()) {
                AutoPtr<AST> importAst = importPair.second;
                if (importAst->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
                    String fileName = FileName(importAst->GetInterfaceDef()->GetFullName());
                    sb.AppendFormat("#include \"%s.h\"\n", fileName.string());
                }
            }
        }
    } else {
        sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
        EmitServiceStubStdlibInclusions(sb);
        sb.Append("#include <hdf_remote_service.h>\n");
        sb.AppendFormat("#include \"%s.h\"\n", FileName(implName_).string());
    }
}

void CServiceStubCodeEmitter::EmitServiceStubStdlibInclusions(StringBuilder& sb)
{
    if (!isKernelCode_) {
        sb.Append("#include <dlfcn.h>\n");
    }

    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <hdf_device_desc.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <hdf_sbuf.h>\n");
    sb.Append("#include <osal_mem.h>\n");

    if (isKernelCode_) {
        const AST::TypeStringMap& types = ast_->GetTypes();
        for (const auto& pair : types) {
            AutoPtr<ASTType> type = pair.second;
            if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
                sb.Append("#include <securec.h>\n");
                break;
            }
        }
    } else {
        sb.Append("#include <securec.h>\n");
    }
}

void CServiceStubCodeEmitter::EmitDriverLibPath(StringBuilder& sb)
{
    sb.Append("#ifdef __ARM64__\n");
    sb.Append("#define DRIVER_PATH \"system/lib64\"\n");
    sb.Append("#else\n");
    sb.Append("#define DRIVER_PATH \"system/lib\"\n");
    sb.Append("#endif\n");
}

void CServiceStubCodeEmitter::EmitServConstructTypedef(StringBuilder& sb)
{
    typedef void (*SERVICE_CONSTRUCT_FUNC)(struct callbacks *);
    sb.AppendFormat("typedef void (*SERVICE_CONSTRUCT_FUNC)(struct %s *);\n", interfaceName_.string());
}

void CServiceStubCodeEmitter::EmitCbStubDefinitions(StringBuilder& sb)
{
    sb.AppendFormat("struct %sStub {\n", infName_.string());
    sb.Append(g_tab).AppendFormat("struct %s service;\n", interfaceName_.string());
    if (isCallbackInterface()) {
        sb.Append(g_tab).Append("struct HdfRemoteDispatcher dispatcher;\n");
    }
    sb.Append(g_tab).Append("void *dlHandler;\n");
    sb.Append("};\n");
}

void CServiceStubCodeEmitter::EmitServiceStubMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceStubMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CServiceStubCodeEmitter::EmitServiceStubMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat(
        "static int32_t SerStub%s(struct %s *serviceImpl, struct HdfSBuf *data, struct HdfSBuf *reply)\n",
        method->GetName().string(), interfaceName_.string());
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

    String gotoName = "errors";
    if (method->GetParameterNumber() > 0) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitStubLocalVariable(param, sb, prefix + g_tab);
        }

        sb.Append("\n");
        for (int i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_IN) {
                EmitReadStubMethodParameter(param, "data", sb, prefix + g_tab);
                sb.Append("\n");
            }
        }
    }

    EmitStubCallMethod(method, gotoName, sb, prefix + g_tab);
    sb.Append("\n");

    for (int i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
            param->EmitCWriteVar("reply", gotoName, sb, prefix + g_tab);
            sb.Append("\n");
        }
    }

    EmitErrorHandle(method, gotoName, false, sb, prefix);
    sb.Append(prefix + g_tab).Append("return ec;\n");
    sb.Append(prefix).Append("}\n");
}

void CServiceStubCodeEmitter::EmitStubLocalVariable(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    sb.Append(prefix).Append(param->EmitCLocalVar()).Append("\n");
    if (type->GetTypeKind() == TypeKind::TYPE_ARRAY || type->GetTypeKind() == TypeKind::TYPE_LIST) {
        sb.Append(prefix).AppendFormat("uint32_t %sLen = 0;\n", param->GetName().string());
    }
}

void CServiceStubCodeEmitter::EmitReadStubMethodParameter(const AutoPtr<ASTParameter>& param,
    const String& parcelName, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();

    if (type->GetTypeKind() == TypeKind::TYPE_STRING) {
        EmitReadCStringStubMethodParameter(param, parcelName, sb, prefix, type);
    } else if (type->GetTypeKind() == TypeKind::TYPE_INTERFACE) {
        type->EmitCStubReadVar(parcelName, param->GetName(), sb, prefix);
    } else if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        sb.Append(prefix).AppendFormat("%s = (%s*)OsalMemAlloc(sizeof(%s));\n", param->GetName().string(),
            type->EmitCType(TypeMode::NO_MODE).string(), type->EmitCType(TypeMode::NO_MODE).string());
        sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", param->GetName().string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + g_tab).Append("goto errors;\n");
        sb.Append(prefix).Append("}\n");
        type->EmitCStubReadVar(parcelName, param->GetName(), sb, prefix);
    } else if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
        String cpName = String::Format("%sCp", param->GetName().string());
        type->EmitCStubReadVar(parcelName, cpName, sb, prefix);
        sb.Append(prefix).AppendFormat("%s = (%s*)OsalMemAlloc(sizeof(%s));\n", param->GetName().string(),
            type->EmitCType(TypeMode::NO_MODE).string(), type->EmitCType(TypeMode::NO_MODE).string());
        sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", param->GetName().string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + g_tab).Append("goto errors;\n");
        sb.Append(prefix).Append("}\n");
        sb.Append(prefix).AppendFormat("(void)memcpy_s(%s, sizeof(%s), %s, sizeof(%s));\n", param->GetName().string(),
            type->EmitCType(TypeMode::NO_MODE).string(), cpName.string(), type->EmitCType(TypeMode::NO_MODE).string());
    } else if (type->GetTypeKind() == TypeKind::TYPE_ARRAY || type->GetTypeKind() == TypeKind::TYPE_LIST) {
        type->EmitCStubReadVar(parcelName, param->GetName(), sb, prefix);
    } else if (type->GetTypeKind() == TypeKind::TYPE_FILEDESCRIPTOR) {
        type->EmitCStubReadVar(parcelName, param->GetName(), sb, prefix);
    } else {
        String name = String::Format("&%s", param->GetName().string());
        type->EmitCStubReadVar(parcelName, name, sb, prefix);
    }
}

void CServiceStubCodeEmitter::EmitReadCStringStubMethodParameter(const AutoPtr<ASTParameter>& param,
    const String& parcelName, StringBuilder& sb, const String& prefix,  AutoPtr<ASTType>& type)
{
    String cloneName = String::Format("%sCp", param->GetName().string());
    type->EmitCStubReadVar(parcelName, cloneName, sb, prefix);
    if (isKernelCode_) {
        sb.Append("\n");
        sb.Append(prefix).AppendFormat("%s = (char*)OsalMemCalloc(strlen(%s) + 1);\n",
            param->GetName().string(), cloneName.string());
        sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", param->GetName().string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + g_tab).Append("goto errors;\n");
        sb.Append(prefix).Append("}\n\n");
        sb.Append(prefix).AppendFormat("if (strcpy_s(%s, (strlen(%s) + 1), %s) != HDF_SUCCESS) {\n",
            param->GetName().string(), cloneName.string(), cloneName.string());
        sb.Append(prefix + g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n",
            param->GetName().string());
        sb.Append(prefix + g_tab).Append("ec = HDF_ERR_INVALID_PARAM;\n");
        sb.Append(prefix + g_tab).Append("goto errors;\n");
        sb.Append(prefix).Append("}\n");
    } else {
        sb.Append(prefix).AppendFormat("%s = strdup(%s);\n", param->GetName().string(), cloneName.string());
    }
}

void CServiceStubCodeEmitter::EmitStubCallMethod(const AutoPtr<ASTMethod>& method, const String& gotoLabel,
    StringBuilder& sb, const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("ec = serviceImpl->%s(serviceImpl);\n", method->GetName().string());
    } else {
        sb.Append(prefix).AppendFormat("ec = serviceImpl->%s(serviceImpl, ", method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitCallParameter(sb, param->GetType(), param->GetAttribute(), param->GetName());
            if (i + 1 < method->GetParameterNumber()) {
                sb.Append(", ");
            }
        }
        sb.AppendFormat(");\n", method->GetName().string());
    }

    sb.Append(prefix).Append("if (ec != HDF_SUCCESS) {\n");
    sb.Append(prefix + g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: call %s function failed!\", __func__);\n", method->GetName().string());
    sb.Append(prefix + g_tab).AppendFormat("goto %s;\n", gotoLabel.string());
    sb.Append(prefix).Append("}\n");
}

void CServiceStubCodeEmitter::EmitCallParameter(StringBuilder& sb, const AutoPtr<ASTType>& type, ParamAttr attribute,
    const String& name)
{
    if (attribute == ParamAttr::PARAM_OUT) {
        sb.AppendFormat("&%s", name.string());
        if (type->GetTypeKind() == TypeKind::TYPE_ARRAY || type->GetTypeKind() == TypeKind::TYPE_LIST) {
            sb.AppendFormat(", &%sLen", name.string());
        }
    } else {
        sb.AppendFormat("%s", name.string());
        if (type->GetTypeKind() == TypeKind::TYPE_ARRAY || type->GetTypeKind() == TypeKind::TYPE_LIST) {
            sb.AppendFormat(", %sLen", name.string());
        }
    }
}

void CServiceStubCodeEmitter::EmitServiceStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix)
{
    String codeName;
    if (!isCallbackInterface()) {
        codeName = "cmdId";
        sb.Append(prefix).AppendFormat("int32_t %sServiceOnRemoteRequest(void *service, int %s, ",
            infName_.string(), codeName.string());
        sb.Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    } else {
        codeName = "code";
        sb.Append(prefix).AppendFormat("int32_t %sServiceOnRemoteRequest(struct HdfRemoteService *service, int %s,\n",
            infName_.string(), codeName.string());
        sb.Append(prefix + g_tab).Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    }
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + g_tab).AppendFormat("struct %s *serviceImpl = (struct %s*)service;\n",
        interfaceName_.string(), interfaceName_.string());
    sb.Append(prefix + g_tab).AppendFormat("switch (%s) {\n", codeName.string());

    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + g_tab + g_tab).AppendFormat("case CMD_%s:\n", ConstantName(method->GetName()).string());
        sb.Append(prefix + g_tab + g_tab + g_tab).AppendFormat("return SerStub%s(serviceImpl, data, reply);\n",
            method->GetName().string());
    }

    sb.Append(prefix + g_tab + g_tab).Append("default: {\n");
    sb.Append(prefix + g_tab + g_tab + g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: not support cmd %%{public}d\", __func__, %s);\n", codeName.string());
    sb.Append(prefix + g_tab + g_tab + g_tab).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + g_tab + g_tab).Append("}\n");
    sb.Append(prefix + g_tab).Append("}\n");
    sb.Append("}\n");
}

void CServiceStubCodeEmitter::EmitStubLinkService(StringBuilder& sb)
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

void CServiceStubCodeEmitter::EmitStubGetMethodImpl(StringBuilder& sb)
{
    String libName = String::Format("lib%s.z.so", FileName(implName_).string());
    String stubTypeName = String::Format("%sStub", infName_.string());
    String objName = "stub";
    sb.AppendFormat("struct %s *%sStubGetInstance(void)\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("SERVICE_CONSTRUCT_FUNC serviceConstructFunc = NULL;\n", libName.string());
    sb.Append(g_tab).AppendFormat("struct %s *%s = (struct %s *)OsalMemAlloc(sizeof(struct %s));\n",
        stubTypeName.string(), objName.string(), stubTypeName.string(), stubTypeName.string());
    sb.Append(g_tab).AppendFormat("if (%s == NULL) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: OsalMemAlloc obj failed!\", __func__);\n",
        stubTypeName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    if (isCallbackInterface()) {
        sb.Append(g_tab).AppendFormat("%s->dispatcher.Dispatch = %sServiceOnRemoteRequest;\n",
            objName.string(), infName_.string());
        sb.Append(g_tab).AppendFormat(
            "%s->service.remote = HdfRemoteServiceObtain((struct HdfObject*)%s, &(%s->dispatcher));\n",
            objName.string(), objName.string(), objName.string());
        sb.Append(g_tab).AppendFormat("if (%s->service.remote == NULL) {\n", objName.string());
        sb.Append(g_tab).Append(g_tab).AppendFormat(
            "HDF_LOGE(\"%%{public}s: %s->service.remote is null\", __func__);\n", objName.string());
        sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(%s);\n", objName.string());
        sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
        sb.Append(g_tab).Append("}\n\n");
    }
    sb.Append(g_tab).AppendFormat("%s->dlHandler = LoadServiceHandler(\"lib%s.z.so\");\n",
        objName.string(), FileName(implName_).string());
    sb.Append(g_tab).AppendFormat("if (%s->dlHandler == NULL) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("HDF_LOGE(\"%%{public}s: %s->dlHanlder is null\", __func__);\n",
        objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(%s);\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("serviceConstructFunc = ");
    sb.AppendFormat("(SERVICE_CONSTRUCT_FUNC)dlsym(%s->dlHandler, \"%sServiceConstruct\");\n",
        objName.string(), infName_.string());
    sb.Append(g_tab).Append("if (serviceConstructFunc == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("HDF_LOGE(\"%{public}s: dlsym failed %{public}s\", __func__, dlerror());\n");
    sb.Append(g_tab).Append(g_tab).AppendFormat("dlclose(%s->dlHandler);\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat("OsalMemFree(%s);\n", objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n\n");
    sb.Append(g_tab).AppendFormat("serviceConstructFunc(&%s->service);\n", objName.string());
    sb.Append(g_tab).AppendFormat("return &%s->service;\n", objName.string());
    sb.Append("}\n");
}

void CServiceStubCodeEmitter::EmitKernelStubGetMethodImpl(StringBuilder& sb)
{
    String objName("instance");
    sb.AppendFormat("struct %s *%sStubGetInstance(void)\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).AppendFormat("struct %s *%s = (struct %s*)OsalMemAlloc(sizeof(struct %s));\n",
        interfaceName_.string(), objName.string(), interfaceName_.string(), interfaceName_.string());
    sb.Append(g_tab).AppendFormat("if (%s == NULL) {\n", objName.string());
    sb.Append(g_tab).Append(g_tab).AppendFormat(
        "HDF_LOGE(\"%%{public}s: OsalMemAlloc struct %s %s failed!\", __func__);\n",
        interfaceName_.string(), objName.string());
    sb.Append(g_tab).Append(g_tab).Append("return NULL;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("%sServiceConstruct(%s);\n", infName_.string(), objName.string());
    sb.Append(g_tab).AppendFormat("return %s;\n", objName.string());
    sb.Append("}\n");
}

void CServiceStubCodeEmitter::EmitStubReleaseImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sStubRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (instance == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).AppendFormat("struct %s *stub = CONTAINER_OF(instance, struct %s, service);\n",
        stubName_.string(), stubName_.string());
    sb.Append(g_tab).Append("dlclose(stub->dlHandler);\n");
    sb.Append(g_tab).Append("OsalMemFree(stub);\n");
    sb.Append("}");
}

void CServiceStubCodeEmitter::EmitKernelStubReleaseImpl(StringBuilder& sb)
{
    sb.AppendFormat("void %sStubRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(g_tab).Append("if (instance == NULL) {\n");
    sb.Append(g_tab).Append(g_tab).Append("return;\n");
    sb.Append(g_tab).Append("}\n");
    sb.Append(g_tab).Append("OsalMemFree(instance);\n");
    sb.Append("}");
}

} // namespace HDI
} // namespace OHOS