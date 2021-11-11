/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */


#ifndef HC_GEN_MACRO_GEN_H
#define HC_GEN_MACRO_GEN_H

#include <fstream>
#include "generator.h"
#include <map>

namespace OHOS {
namespace Hardware {

class MacroGen : public Generator {
public:
    explicit MacroGen(std::shared_ptr<Ast> ast);

    ~MacroGen() override = default;

    bool Output() override;

private:
    bool Initialize();

    bool TemplateNodeSeparate();

	std::string GenFullName(uint32_t depth, const std::shared_ptr<AstObject> &node, const std::string &sep);

	bool GenNodeForeach(uint32_t depth, const std::shared_ptr<AstObject> &node);

    bool NodeWalk();

	bool HeaderTopOutput();

	bool HeaderBottomOutput();

	const std::string &ToUpperString(std::string &str);

	bool GenArray(const std::string &name, uint32_t &arrSize, uint32_t type, const std::shared_ptr<AstObject> &node);

    std::string GenRefObjName(uint32_t depth, const std::shared_ptr<AstObject> &object);

    const std::string &TypeToStr(uint32_t type);

    std::ofstream ofs_;
    std::string outFileName_;
	std::map<int, std::string> nodeNameMap_;
};
} // Hardware
} // OHOS
#endif // HC_GEN_MACRO_GEN_H
