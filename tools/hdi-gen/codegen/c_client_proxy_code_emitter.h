/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_CLIENT_PROXY_CODE_EMITTER_H
#define OHOS_HDI_C_CLIENT_PROXY_CODE_EMITTER_H

#include "codegen/c_code_emitter.h"

namespace OHOS {
namespace HDI {
class CClientProxyCodeEmitter : public CCodeEmitter {
public:
    CClientProxyCodeEmitter() : CCodeEmitter() {}

    virtual ~CClientProxyCodeEmitter() = default;
private:
    bool ResolveDirectory(const String& targetDirectory) override;

    void EmitCode() override;

    void EmitProxySourceFile();

    void EmitProxyDefinition(StringBuilder& sb);

    void EmitProxyInclusions(StringBuilder& sb);

    void EmitProxyStdlibInclusions(StringBuilder& sb);

    void EmitProxyCallMethodImpl(StringBuilder& sb);

    void EmitProxyMethodImpls(StringBuilder& sb);

    void EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb);

    void EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitCreateBuf(const String& dataBufName, const String& replyBufName, StringBuilder& sb, const String& prefix);

    void EmitReleaseBuf(const String& dataBufName, const String& replyBufName, StringBuilder& sb,
        const String& prefix);

    void EmitReadProxyMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
        const String& gotoLabel, StringBuilder& sb, const String& prefix);

    String GetGotLabel(const AutoPtr<ASTMethod>& method);

    void EmitProxyConstruction(StringBuilder&);

    void EmitProxyGetMethodImpl(StringBuilder& sb);

    void EmitProxyGetInstanceMethodImpl(StringBuilder& sb);

    void EmitKernelProxyGetInstanceMethodImpl(StringBuilder& sb);

    void EmitCbProxyGetMethodImpl(StringBuilder& sb);

    void EmitProxyReleaseMethodImpl(StringBuilder& sb);

    void EmitKernelProxyReleaseMethodImpl(StringBuilder& sb);

    std::vector<String> freeObjStatements_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_CLIENT_PROXY_CODE_EMITTER_H