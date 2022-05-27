/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_ASTENUMTYPE_H
#define OHOS_HDI_ASTENUMTYPE_H

#include "ast/ast_attribute.h"
#include "ast/ast_expr.h"
#include "ast/ast_type.h"
#include "util/autoptr.h"
#include "util/string.h"

#include <vector>

namespace OHOS {
namespace HDI {
class ASTEnumValue : public ASTNode {
public:
    explicit ASTEnumValue(const String &name) : mName_(name), value_(nullptr) {}

    inline virtual ~ASTEnumValue() {}

    inline String GetName()
    {
        return mName_;
    }

    inline void SetType(const AutoPtr<ASTType> &type)
    {
        mType_ = type;
    }

    inline AutoPtr<ASTType> GetType()
    {
        return mType_;
    }

    inline void SetExprValue(const AutoPtr<ASTExpr> &value)
    {
        value_ = value;
    }

    inline AutoPtr<ASTExpr> GetExprValue()
    {
        return value_;
    }

private:
    String mName_;
    AutoPtr<ASTType> mType_;
    AutoPtr<ASTExpr> value_;
};

class ASTEnumType : public ASTType {
public:
    inline void SetName(const String &name)
    {
        name_ = name;
    }

    inline String GetName()
    {
        return name_;
    }

    inline void SetAttribute(const AutoPtr<ASTTypeAttr> &attr)
    {
        if (attr != nullptr) {
            attr_ = attr;
        }
    }

    inline bool IsFull()
    {
        return attr_ != nullptr ? attr_->isFull_ : false;
    }

    inline bool IsLite()
    {
        return attr_ != nullptr ? attr_->isLite_ : false;
    }

    void SetBaseType(const AutoPtr<ASTType> &baseType);

    inline AutoPtr<ASTType> GetBaseType()
    {
        return baseType_;
    }

    void AddMember(const AutoPtr<ASTEnumValue> &member);

    inline size_t GetMemberNumber()
    {
        return members_.size();
    }

    inline AutoPtr<ASTEnumValue> GetMember(size_t index)
    {
        if (index >= members_.size()) {
            return nullptr;
        }
        return members_[index];
    }

    bool IsEnumType() override;

    String ToString() override;

    String Dump(const String &prefix) override;

    TypeKind GetTypeKind() override;

    String EmitCType(TypeMode mode = TypeMode::NO_MODE) const override;

    String EmitCppType(TypeMode mode = TypeMode::NO_MODE) const override;

    String EmitJavaType(TypeMode mode, bool isInnerType = false) const override;

    String EmitCTypeDecl() const;

    String EmitCppTypeDecl() const;

    String EmitJavaTypeDecl() const;

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

    void EmitCMarshalling(const String &name, StringBuilder &sb, const String &prefix) const override;

    void EmitCUnMarshalling(const String &name, const String &gotoLabel, StringBuilder &sb, const String &prefix,
        std::vector<String> &freeObjStatements) const override;

    void EmitCppMarshalling(const String &parcelName, const String &name, StringBuilder &sb, const String &prefix,
        unsigned int innerLevel = 0) const override;

    void EmitCppUnMarshalling(const String &parcelName, const String &name, StringBuilder &sb, const String &prefix,
        bool emitType, unsigned int innerLevel = 0) const override;

private:
    AutoPtr<ASTTypeAttr> attr_ = new ASTTypeAttr();
    AutoPtr<ASTType> baseType_ = nullptr;
    std::vector<AutoPtr<ASTEnumValue>> members_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_ASTENUMTYPE_H