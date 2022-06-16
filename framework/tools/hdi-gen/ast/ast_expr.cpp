/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_expr.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
String ASTUnaryExpr::Dump(const String &prefix)
{
    StringBuilder sb;
    sb.Append(prefix);
    if (isParenExpr) {
        sb.Append("(");
    }

    sb.AppendFormat("%s%s", UnaryOpToString(op_).string(), expr_->Dump("").string());

    if (isParenExpr) {
        sb.Append(")");
    }

    return sb.ToString();
}

String ASTUnaryExpr::UnaryOpToString(UnaryOpKind op)
{
    switch (op) {
        case UnaryOpKind::PLUS:
            return "+";
        case UnaryOpKind::MINUS:
            return "-";
        case UnaryOpKind::TILDE:
            return "~";
        default:
            return "unknown";
    }
}

String ASTBinaryExpr::Dump(const String &prefix)
{
    StringBuilder sb;
    sb.Append(prefix);
    if (isParenExpr) {
        sb.Append("(");
    }

    sb.AppendFormat("%s %s %s", lExpr_->Dump("").string(), BinaryOpToString(op_).string(), rExpr_->Dump("").string());

    if (isParenExpr) {
        sb.Append(")");
    }

    return sb.ToString();
}

String ASTBinaryExpr::BinaryOpToString(BinaryOpKind op)
{
    switch (op) {
        case BinaryOpKind::MUL:
            return "*";
        case BinaryOpKind::DIV:
            return "/";
        case BinaryOpKind::MOD:
            return "%";
        case BinaryOpKind::ADD:
            return "+";
        case BinaryOpKind::SUB:
            return "-";
        case BinaryOpKind::LSHIFT:
            return "<<";
        case BinaryOpKind::RSHIFT:
            return ">>";
        case BinaryOpKind::AND:
            return "&";
        case BinaryOpKind::XOR:
            return "^";
        case BinaryOpKind::OR:
            return "|";
        default:
            return "unknown";
    }
}

String ASTNumExpr::Dump(const String &prefix)
{
    StringBuilder sb;
    sb.Append(prefix);
    if (isParenExpr) {
        sb.Append("(");
    }

    sb.AppendFormat("%s", value_.string());

    if (isParenExpr) {
        sb.Append("(");
    }

    return sb.ToString();
}
} // namespace HDI
} // namespace OHOS