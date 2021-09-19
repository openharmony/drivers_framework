/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_GENERATORFACTORY_H
#define OHOS_HDI_GENERATORFACTORY_H

#include "ast/ast_module.h"
#include "codegen/code_generator.h"

namespace OHOS {
namespace HDI {
class GeneratorFactory {
public:
    GeneratorFactory(const GeneratorFactory&) = default;

    GeneratorFactory& operator=(const GeneratorFactory&) = default;

    ~GeneratorFactory() = default;

    static GeneratorFactory& GetInstance();

    AutoPtr<CodeGenerator> GetCodeGenerator(const AutoPtr<ASTModule>& astModule, const String& targetLanuage,
        const String& targetDirectory);
private:
    GeneratorFactory() = default;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_GENERATORFACTORY_H