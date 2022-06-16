/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_ASTMETHOD_H
#define OHOS_HDI_ASTMETHOD_H

#include <vector>

#include "ast/ast_node.h"
#include "ast/ast_parameter.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
class ASTMethod : public ASTNode {
public:
    inline void SetName(const String &name)
    {
        name_ = name;
    }

    inline String GetName()
    {
        return name_;
    }

    inline void SetAttribute(const AutoPtr<ASTMethodAttr> &attr)
    {
        if (attr != nullptr) {
            attr_ == attr;
        }
    }

    inline bool IsOneWay()
    {
        return attr_->isOneWay_;
    }

    inline bool IsFull()
    {
        return attr_->isFull_;
    }

    inline bool IsLite()
    {
        return attr_->isLite_;
    }

    void AddParameter(const AutoPtr<ASTParameter> &parameter);

    AutoPtr<ASTParameter> GetParameter(size_t index);

    inline size_t GetParameterNumber()
    {
        return parameters_.size();
    }

    String Dump(const String &prefix) override;

private:
    String name_;
    AutoPtr<ASTMethodAttr> attr_ = new ASTMethodAttr();
    std::vector<AutoPtr<ASTParameter>> parameters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_ASTMETHOD_H