/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/generator_factory.h"
#include "codegen/c_code_generator.h"
#include "codegen/cpp_code_generator.h"
#include "codegen/java_code_generator.h"

namespace OHOS {
namespace HDI {
GeneratorFactory& GeneratorFactory::GetInstance()
{
    static GeneratorFactory factory;
    return factory;
}

AutoPtr<CodeGenerator> GeneratorFactory::GetCodeGenerator(const AutoPtr<ASTModule>& astModule,
    const String& targetLanuage, const String& targetDirectory)
{
    if (targetLanuage.Equals("c")) {
        return new CCodeGenerator(astModule, targetDirectory);
    } else if (targetLanuage.Equals("cpp")) {
        return new CppCodeGenerator(astModule, targetDirectory);
    } else if (targetLanuage.Equals("java")) {
        return new JavaCodeGenerator(astModule, targetDirectory);
    }

    return nullptr;
}
} // namespace HDI
} // namespace OHOS