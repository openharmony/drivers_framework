/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_SERVICE_STUB_CODE_EMITTER_H
#define OHOS_HDI_CPP_SERVICE_STUB_CODE_EMITTER_H

#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppServiceStubCodeEmitter : public CppCodeEmitter {
public:
    CppServiceStubCodeEmitter() : CppCodeEmitter() {}

    virtual ~CppServiceStubCodeEmitter() = default;
private:
    bool ResolveDirectory(const String& targetDirectory) override;

    void EmitCode() override;

    // ISample.idl -> sample_service_stub.h
    void EmitStubHeaderFile();

    void EmitStubHeaderInclusions(StringBuilder& sb);

    void EmitStubUsingNamespace(StringBuilder& sb);

    void EmitStubDecl(StringBuilder& sb);

    void EmitCbStubDecl(StringBuilder& sb);

    void EmitStubBody(StringBuilder& sb, const String& prefix);

    void EmitCbStubBody(StringBuilder& sb, const String& prefix);

    void EmitStubDestruction(StringBuilder& sb, const String& prefix);

    void EmitCbStubOnRequestDecl(StringBuilder& sb, const String& prefix);

    void EmitStubMethodDecls(StringBuilder& sb, const String& prefix);

    void EmitStubMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitStubOnRequestMethodDecl(StringBuilder& sb, const String& prefix);

    void EmitStubMembers(StringBuilder& sb, const String& prefix);

    void EmitStubExternalsMethodsDel(StringBuilder& sb);

    // ISample.idl -> sample_service_stub.cpp
    void EmitStubSourceFile();

    void EmitStubSourceInclusions(StringBuilder& sb);

    void EmitStubSourceStdlibInclusions(StringBuilder& sb);

    void EmitDriverLibPath(StringBuilder& sb);

    void EmitLibFuncTypeDef(StringBuilder& sb);

    void EmitStubMethodImpls(StringBuilder& sb, const String& prefix);

    void EmitStubMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitStubCallMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix);

    void EmitCbStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix);

    void EmitStubExternalsMethodsImpl(StringBuilder& sb, const String& prefix);

    void EmitStubLinkService(StringBuilder& sb);

    void EmitStubInstanceMethodImpl(StringBuilder& sb);

    void EmitStubReleaseMethodImpl(StringBuilder& sb);

    void EmitServiceOnRemoteRequest(StringBuilder& sb);

    String EmitStubServiceUsings(String nameSpace);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_SERVICE_STUB_CODE_EMITTER_H