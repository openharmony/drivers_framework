/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/code_emitter.h"
#include "util/file.h"
#include "util/options.h"

namespace OHOS {
namespace HDI {
bool CodeEmitter::OutPut(const AutoPtr<AST>& ast, const String& targetDirectory, bool isKernelCode)
{
    if (!Reset(ast, targetDirectory, isKernelCode)) {
        return false;
    }

    EmitCode();
    return true;
}

bool CodeEmitter::Reset(const AutoPtr<AST>& ast, const String& targetDirectory, bool isKernelCode)
{
    if (ast == nullptr || targetDirectory.Equals("")) {
        return false;
    }

    CleanData();

    isKernelCode_ = isKernelCode;
    ast_ = ast;
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE || ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        interface_ = ast_->GetInterfaceDef();
        interfaceName_ = interface_->GetName();
        interfaceFullName_ = interface_->GetNamespace()->ToString() + interfaceName_;
        infName_ = interfaceName_.StartsWith("I") ? interfaceName_.Substring(1) : interfaceName_;
        proxyName_ = infName_ + "Proxy";
        proxyFullName_ = interface_->GetNamespace()->ToString() + proxyName_;

        stubName_ = infName_ + "Stub";
        stubFullName_ = interface_->GetNamespace()->ToString() + stubName_;

        implName_ = infName_ + "Service";
        implFullName_ = interface_->GetNamespace()->ToString() + implName_;
    } else if (ast_->GetASTFileType() == ASTFileType::AST_TYPES) {
        infName_ = ast_->GetName();
    }

    majorVerName_ = String::Format("%s_MAJOR_VERSION", interfaceName_.ToUnderLineUpper().string());
    minorVerName_ = String::Format("%s_MINOR_VERSION", interfaceName_.ToUnderLineUpper().string());

    if (!ResolveDirectory(targetDirectory)) {
        return false;
    }

    return true;
}

void CodeEmitter::CleanData()
{
    isKernelCode_ = false;
    ast_ = nullptr;
    interface_ = nullptr;
    directory_ = "";
    interfaceName_ = "";
    interfaceFullName_ = "";
    infName_ = "";
    proxyName_ = "";
    proxyFullName_ = "";
    stubName_ = "";
    stubFullName_ = "";
    implName_ = "";
    implFullName_ = "";
}

String CodeEmitter::GetFilePath(const String& outDir)
{
    String outPath = outDir.EndsWith(File::pathSeparator) ?
        outDir.Substring(0, outDir.GetLength() - 1) : outDir;
    String packagePath = Options::GetInstance().GetPackagePath(ast_->GetPackageName());
    if (packagePath.EndsWith(File::pathSeparator)) {
        return String::Format("%s/%s", outPath.string(), packagePath.string());
    } else {
        return String::Format("%s/%s/", outPath.string(), packagePath.string());
    }
}
} // namespace HDI
} // namespace OHOS