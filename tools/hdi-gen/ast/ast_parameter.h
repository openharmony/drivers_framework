/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_ASTPARAMETER_H
#define OHOS_HDI_ASTPARAMETER_H

#include "ast/ast_attribute.h"
#include "ast/ast_node.h"
#include "ast/ast_type.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
class ASTParameter : public ASTNode {
public:
    ASTParameter(const String &name, ParamAttr attribute, const AutoPtr<ASTType> &type)
        : ASTNode(), name_(name), attr_(new ASTParamAttr(attribute)), type_(type)
    {
    }

    ASTParameter(const String &name, const AutoPtr<ASTParamAttr> &attribute, const AutoPtr<ASTType> &type)
        : ASTNode(), name_(name), attr_(attribute), type_(type)
    {
    }

    inline String GetName()
    {
        return name_;
    }

    inline AutoPtr<ASTType> GetType()
    {
        return type_;
    }

    inline ParamAttr GetAttribute()
    {
        return attr_->value_;
    }

    String Dump(const String &prefix) override;

    String EmitCParameter();

    String EmitCppParameter();

    String EmitJavaParameter();

    String EmitCLocalVar();

    String EmitCppLocalVar();

    String EmitJavaLocalVar();

    void EmitCWriteVar(const String &parcelName, const String &ecName, const String &gotoLabel, StringBuilder &sb,
        const String &prefix) const;

    bool EmitCProxyWriteOutVar(const String &parcelName, const String &ecName, const String &gotoLabel,
        StringBuilder &sb, const String &prefix) const;

    void EmitCStubReadOutVar(const String &buffSizeName, const String &memFlagName, const String &parcelName,
        const String &ecName, const String &gotoLabel, StringBuilder &sb, const String &prefix) const;

    void EmitJavaWriteVar(const String &parcelName, StringBuilder &sb, const String &prefix) const;

    void EmitJavaReadVar(const String &parcelName, StringBuilder &sb, const String &prefix) const;

private:
    String name_;
    AutoPtr<ASTType> type_ = nullptr;
    AutoPtr<ASTParamAttr> attr_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_ASTPARAMETER_H