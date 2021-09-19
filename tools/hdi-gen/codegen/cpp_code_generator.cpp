/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "cpp_code_generator.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include "codegen/cpp_client_interface_code_emitter.h"
#include "codegen/cpp_client_proxy_code_emitter.h"
#include "codegen/cpp_custom_types_code_emitter.h"
#include "codegen/cpp_service_driver_code_emitter.h"
#include "codegen/cpp_service_impl_code_emitter.h"
#include "codegen/cpp_service_interface_code_emitter.h"
#include "codegen/cpp_service_stub_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool CppCodeGenerator::Generate()
{
    Initializate();

    for (auto& astPair : astModule_->GetAllAsts()) {
        AutoPtr<AST> ast = astPair.second;
        switch (ast->GetASTFileType()) {
            case ASTFileType::AST_TYPES: {
                emitters_["types"]->OutPut(ast, targetDirectory_);
                break;
            }
            case ASTFileType::AST_IFACE:
            case ASTFileType::AST_ICALLBACK: {
                emitters_["clientIface"]->OutPut(ast, targetDirectory_);
                emitters_["proxy"]->OutPut(ast, targetDirectory_);
                emitters_["serviceIface"]->OutPut(ast, targetDirectory_);
                emitters_["driver"]->OutPut(ast, targetDirectory_);
                emitters_["stub"]->OutPut(ast, targetDirectory_);
                emitters_["impl"]->OutPut(ast, targetDirectory_);
                break;
            }
            default:
                break;
        }
    }
    return true;
}

void CppCodeGenerator::Initializate()
{
    emitters_ = {
        {"types", new CppCustomTypesCodeEmitter()},
        {"clientIface", new CppClientInterfaceCodeEmitter()},
        {"proxy", new CppClientProxyCodeEmitter()},
        {"serviceIface", new CppServiceInterfaceCodeEmitter()},
        {"driver", new CppServiceDriverCodeEmitter()},
        {"stub", new CppServiceStubCodeEmitter()},
        {"impl", new CppServiceImplCodeEmitter()},
    };
}
} // namespace HDI
} // namespace OHOS