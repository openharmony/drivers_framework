/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CODEGENERATOR_H
#define OHOS_HDI_CODEGENERATOR_H

#include "ast/ast_module.h"

namespace OHOS {
namespace HDI {
class CodeGenerator : public LightRefCountBase {
public:
    CodeGenerator(const AutoPtr<ASTModule>& astModule, const String& targetDirectory)
        : LightRefCountBase(), astModule_(astModule), targetDirectory_(targetDirectory) {}

    virtual ~CodeGenerator() = default;

    virtual bool Generate() = 0;
protected:
    AutoPtr<ASTModule> astModule_;
    String targetDirectory_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CODEGENERATOR_H