/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_SERVICE_INTERFACE_CODE_EMITTER_H
#define OHOS_HDI_CPP_SERVICE_INTERFACE_CODE_EMITTER_H

#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppServiceInterfaceCodeEmitter : public CppCodeEmitter {
public:
    CppServiceInterfaceCodeEmitter() : CppCodeEmitter() {}

    virtual ~CppServiceInterfaceCodeEmitter() = default;
private:
    bool ResolveDirectory(const String& targetDirectory) override;

    void EmitCode() override;

    void EmitInterfaceHeaderFile();

    void EmitServiceInfHeadMacro(StringBuilder& sb);

    void EmitServiceInfTailMacro(StringBuilder& sb);

    void EmitInterfaceInclusions(StringBuilder& sb);

    void EmitInterfaceStdlibInclusions(StringBuilder& sb);

    void EmitInterfaceDBinderInclusions(StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb);

    void EmitInterfaceUsings(StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeUsings(StringBuilder& sb);

    void EmitInterfaceDecl(StringBuilder& sb);

    void EmitInterfaceBody(StringBuilder& sb, const String& prefix);

    void EmitInterfaceDestruction(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethods(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_SERVICE_INTERFACE_CODE_EMITTER_H