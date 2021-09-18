/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_CODE_GENERATOR_H
#define OHOS_HDI_CPP_CODE_GENERATOR_H

#include "codegen/code_generator.h"
#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppCodeGenerator : public CodeGenerator {
public:
    using CppCodeEmitMap = std::unordered_map<String, AutoPtr<CppCodeEmitter>, StringHashFunc, StringEqualFunc>;

    CppCodeGenerator(const AutoPtr<ASTModule>& astModule, const String& targetDirectory)
        : CodeGenerator(astModule, targetDirectory), emitters_() {}

    ~CppCodeGenerator() = default;

    bool Generate() override;
private:
    void Initializate();

    CppCodeEmitMap emitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_CODE_GENERATOR_H