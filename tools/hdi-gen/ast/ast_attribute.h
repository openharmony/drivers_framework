/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_AST_ATTRIBUTE_H
#define OHOS_HDI_AST_ATTRIBUTE_H

#include "ast/ast_node.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
class ASTTypeAttr : public ASTNode {
public:
    String ToString() override;

    String Dump(const String &prefix) override;

public:
    bool isFull_ = false;
    bool isLite_ = false;
};

class ASTInfAttr : public ASTNode {
public:
    String ToString() override;

    String Dump(const String &prefix) override;

public:
    bool isFull_ = false;
    bool isLite_ = false;
    bool isCallback_ = false;
    bool isOneWay_ = false;
};

class ASTMethodAttr : public ASTNode {
public:
    String ToString() override;

    String Dump(const String &prefix) override;

public:
    bool isFull_ = false;
    bool isLite_ = false;
    bool isOneWay_ = false;
};

enum class ParamAttr {
    PARAM_IN,
    PARAM_OUT,
};

class ASTParamAttr : public ASTNode {
public:
    ASTParamAttr(ParamAttr value) : ASTNode(), value_(value) {}

    String ToString() override;

    String Dump(const String &prefix) override;

public:
    ParamAttr value_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_AST_ATTRIBUTE_H