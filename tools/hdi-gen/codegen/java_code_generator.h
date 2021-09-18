/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_JAVA_CODE_GENERATOR_H
#define OHOS_HDI_JAVA_CODE_GENERATOR_H

#include "codegen/code_generator.h"
#include "codegen/java_code_emitter.h"

namespace OHOS {
namespace HDI {
class JavaCodeGenerator : public CodeGenerator {
public:
    using JavaCodeEmitMap = std::unordered_map<String, AutoPtr<JavaCodeEmitter>, StringHashFunc, StringEqualFunc>;

    JavaCodeGenerator(const AutoPtr<ASTModule>& astModule, const String& targetDirectory)
        : CodeGenerator(astModule, targetDirectory), emitters_() {}

    ~JavaCodeGenerator() override {};

    bool Generate() override;
private:
    void Initializate();

    JavaCodeEmitMap emitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_JAVA_CODE_GENERATOR_H