/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "java_code_generator.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include "codegen/java_client_interface_code_emitter.h"
#include "codegen/java_client_proxy_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool JavaCodeGenerator::Generate()
{
    Initializate();

    for (auto& astPair : astModule_->GetAllAsts()) {
        AutoPtr<AST> ast = astPair.second;
        switch (ast->GetASTFileType()) {
            case ASTFileType::AST_IFACE:
            case ASTFileType::AST_ICALLBACK: {
                emitters_["clientIface"]->OutPut(ast, targetDirectory_);
                emitters_["proxy"]->OutPut(ast, targetDirectory_);
                break;
            }
            default:
                break;
        }
    }
    return true;
}

void JavaCodeGenerator::Initializate()
{
    emitters_ = {
        {"clientIface", new JavaClientInterfaceCodeEmitter()},
        {"proxy", new JavaClientProxyCodeEmitter()},
    };
}
} // namespace HDI
} // namespace OHOS