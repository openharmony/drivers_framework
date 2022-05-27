/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_method.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
void ASTMethod::AddParameter(const AutoPtr<ASTParameter> &parameter)
{
    if (parameter == nullptr) {
        return;
    }
    parameters_.push_back(parameter);
}

AutoPtr<ASTParameter> ASTMethod::GetParameter(size_t index)
{
    if (index >= parameters_.size()) {
        return nullptr;
    }

    return parameters_[index];
}

String ASTMethod::Dump(const String &prefix)
{
    StringBuilder sb;

    sb.Append(prefix).Append(attr_->Dump(prefix)).Append(" ");
    sb.Append(name_).Append('(');
    if (parameters_.size() != 0) {
        sb.Append('\n');
        for (auto parameter : parameters_) {
            String info = parameter->Dump(prefix + TAB);
            sb.Append(info);
            if (parameter != parameters_[parameters_.size() - 1]) {
                sb.Append(",\n");
            }
        }
    }
    sb.Append(");\n");

    return sb.ToString();
}
} // namespace HDI
} // namespace OHOS