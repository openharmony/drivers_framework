/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CODE_EMITTER_H
#define OHOS_HDI_CODE_EMITTER_H

#include "ast/ast.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
class CodeEmitter : public LightRefCountBase {
public:
    virtual ~CodeEmitter() = default;

    bool OutPut(const AutoPtr<AST>& ast, const String& targetDirectory, bool isKernelCode = false);

protected:
    bool Reset(const AutoPtr<AST>& ast, const String& targetDirectory, bool isKernelCode);

    void CleanData();

    virtual bool ResolveDirectory(const String& targetDirectory) = 0;

    virtual void EmitCode() = 0;

    bool isKernelCode_ = false;
    AutoPtr<AST> ast_ = nullptr;
    AutoPtr<ASTInterfaceType> interface_ = nullptr;
    String directory_;

    String interfaceName_;
    String interfaceFullName_;
    String infName_;
    String proxyName_;
    String proxyFullName_;
    String stubName_;
    String stubFullName_;
    String implName_;
    String implFullName_;
};
}
}

#endif // OHOS_HDI_CODE_EMITTER_H
