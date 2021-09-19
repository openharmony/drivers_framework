/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_CODE_GENERATOR_H
#define OHOS_HDI_C_CODE_GENERATOR_H

#include "codegen/c_code_emitter.h"
#include "codegen/code_generator.h"

namespace OHOS {
namespace HDI {
class CCodeGenerator : public CodeGenerator {
public:
    using CCodeEmitMap = std::unordered_map<String, AutoPtr<CCodeEmitter>, StringHashFunc, StringEqualFunc>;

    CCodeGenerator(const AutoPtr<ASTModule>& astModule, const String& targetDirectory)
        : CodeGenerator(astModule, targetDirectory), emitters_() {}

    virtual ~CCodeGenerator() = default;

    bool Generate() override;
private:
    void Initializate();

    CCodeEmitMap emitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_CODE_GENERATOR_H