/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CODEGENERATOR_H
#define OHOS_HDI_CODEGENERATOR_H

#include "codegen/code_emitter.h"

namespace OHOS {
namespace HDI {
using CodeEmitMap = std::unordered_map<String, AutoPtr<CodeEmitter>, StringHashFunc, StringEqualFunc>;

class CodeGenerator : public LightRefCountBase {
public:
    using StrAstMap = std::unordered_map<String, AutoPtr<AST>, StringHashFunc, StringEqualFunc>;
    explicit CodeGenerator(const StrAstMap &allAst) : LightRefCountBase(), allAst_(allAst), targetDirectory_() {}

    bool Generate();

private:
    void GenerateCCode(const AutoPtr<AST> &ast, const String &outDir, const String &codePart, bool isKernel);
    void GenerateCppCode(const AutoPtr<AST> &ast, const String &outDir, const String &codePart);
    void GenerateJavaCode(const AutoPtr<AST> &ast, const String &outDir, const String &codePart);

    String targetDirectory_;

    const StrAstMap &allAst_;

    static CodeEmitMap cCodeEmitters_;
    static CodeEmitMap cppCodeEmitters_;
    static CodeEmitMap javaCodeEmitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CODEGENERATOR_H