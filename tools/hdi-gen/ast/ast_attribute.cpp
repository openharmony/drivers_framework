/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_attribute.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
String ASTTypeAttr::ToString()
{
    StringBuilder sb;
    std::vector<String> attrVec;
    if (isFull_) {
        attrVec.push_back("full");
    }

    if (isLite_) {
        attrVec.push_back("lite");
    }

    if (attrVec.size() > 0) {
        sb.Append("[");
        for (size_t i = 0; i < attrVec.size(); i++) {
            sb.Append(attrVec[i]);
            if (i + 1 < attrVec.size()) {
                sb.Append(", ");
            }
        }
        sb.Append("]");
    }
    return sb.ToString();
}

String ASTTypeAttr::Dump(const String &prefix)
{
    return prefix + ToString();
}

String ASTInfAttr::ToString()
{
    StringBuilder sb;
    std::vector<String> attrVec;
    if (isFull_) {
        attrVec.push_back("full");
    }

    if (isLite_) {
        attrVec.push_back("lite");
    }

    if (isCallback_) {
        attrVec.push_back("callback");
    }

    if (isOneWay_) {
        attrVec.push_back("oneway");
    }

    if (attrVec.size() > 0) {
        sb.Append("[");
        for (size_t i = 0; i < attrVec.size(); i++) {
            sb.Append(attrVec[i]);
            if (i + 1 < attrVec.size()) {
                sb.Append(", ");
            }
        }
        sb.Append("]");
    }
    return sb.ToString();
}

String ASTInfAttr::Dump(const String &prefix)
{
    return prefix + ToString();
}

String ASTMethodAttr::ToString()
{
    StringBuilder sb;
    std::vector<String> attrVec;
    if (isFull_) {
        attrVec.push_back("full");
    }

    if (isLite_) {
        attrVec.push_back("lite");
    }

    if (isOneWay_) {
        attrVec.push_back("oneway");
    }

    if (attrVec.size() > 0) {
        sb.Append("[");
        for (size_t i = 0; i < attrVec.size(); i++) {
            sb.Append(attrVec[i]);
            if (i + 1 < attrVec.size()) {
                sb.Append(", ");
            }
        }
        sb.Append("]");
    }
    return sb.ToString();
}

String ASTMethodAttr::Dump(const String &prefix)
{
    return prefix + ToString();
}

String ASTParamAttr::ToString()
{
    return String::Format("[%s]", (value_ == ParamAttr::PARAM_IN) ? "in" : "out");
}

String ASTParamAttr::Dump(const String &prefix)
{
    return prefix + ToString();
}
} // namespace HDI
} // namespace OHOS