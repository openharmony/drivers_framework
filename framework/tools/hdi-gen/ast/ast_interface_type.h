/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_ASTINTERFACETYPE_H
#define OHOS_HDI_ASTINTERFACETYPE_H

#include "ast/ast_attribute.h"
#include "ast/ast_method.h"

#include <vector>

#include "ast/ast_type.h"
#include "util/autoptr.h"

namespace OHOS {
namespace HDI {
class ASTInterfaceType : public ASTType {
public:
    void SetNamespace(const AutoPtr<ASTNamespace> &nspace) override;

    inline void SetLicense(const String &license)
    {
        license_ = license;
    }

    inline String GetLicense() const
    {
        return license_;
    }

    void SetAttribute(const AutoPtr<ASTInfAttr> &attr)
    {
        if (attr != nullptr) {
            attr_ = attr;
            if (attr_->isCallback_) {
                isSerializable_ = true;
            }
        }
    }

    inline bool IsOneWay()
    {
        return attr_->isOneWay_;
    }

    inline bool IsCallback()
    {
        return attr_->isCallback_;
    }

    inline void SetSerializable(bool isSerializable)
    {
        isSerializable_ = isSerializable;
    }

    inline bool IsSerializable()
    {
        return isSerializable_;
    }

    inline bool IsFull()
    {
        return attr_->isFull_;
    }

    inline bool IsLite()
    {
        return attr_->isLite_;
    }

    void AddMethod(const AutoPtr<ASTMethod> &method);

    AutoPtr<ASTMethod> GetMethod(size_t index);

    inline size_t GetMethodNumber()
    {
        return methods_.size();
    }

    void AddVersionMethod(const AutoPtr<ASTMethod> &method)
    {
        getVerMethod_ = method;
    }

    AutoPtr<ASTMethod> GetVersionMethod()
    {
        return getVerMethod_;
    }

    bool IsInterfaceType() override;

    String ToString() override;

    String Dump(const String &prefix) override;

    TypeKind GetTypeKind() override;

    String GetFullName() const;

    String EmitCType(TypeMode mode = TypeMode::NO_MODE) const override;

    String EmitCppType(TypeMode mode = TypeMode::NO_MODE) const override;

    String EmitJavaType(TypeMode mode, bool isInnerType = false) const override;

    void EmitCWriteVar(const String &parcelName, const String &name, const String &ecName, const String &gotoLabel,
        StringBuilder &sb, const String &prefix) const override;

    void EmitCProxyReadVar(const String &parcelName, const String &name, bool isInnerType, const String &ecName,
        const String &gotoLabel, StringBuilder &sb, const String &prefix) const override;

    void EmitCStubReadVar(const String &parcelName, const String &name, const String &ecName, const String &gotoLabel,
        StringBuilder &sb, const String &prefix) const override;

    void EmitCppWriteVar(const String &parcelName, const String &name, StringBuilder &sb, const String &prefix,
        unsigned int innerLevel = 0) const override;

    void EmitCppReadVar(const String &parcelName, const String &name, StringBuilder &sb, const String &prefix,
        bool initVariable, unsigned int innerLevel = 0) const override;

    void EmitJavaWriteVar(
        const String &parcelName, const String &name, StringBuilder &sb, const String &prefix) const override;

    void EmitJavaReadVar(
        const String &parcelName, const String &name, StringBuilder &sb, const String &prefix) const override;

    void EmitJavaReadInnerVar(const String &parcelName, const String &name, bool isInner, StringBuilder &sb,
        const String &prefix) const override;

private:
    String license_;

    AutoPtr<ASTInfAttr> attr_ = new ASTInfAttr();
    bool isSerializable_ = false;
    std::vector<AutoPtr<ASTMethod>> methods_;
    AutoPtr<ASTMethod> getVerMethod_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_ASTINTERFACETYPE_H