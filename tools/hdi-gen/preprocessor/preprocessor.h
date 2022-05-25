/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_PREPROCESSOR_H
#define OHOS_HDI_PREPROCESSOR_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lexer/lexer.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
struct FileDetail {
public:
    inline String GetFullName() const
    {
        return String::Format("%s.%s", packageName_.string(), fileName_.string());
    }

    String Dump() const;

public:
    String filePath_;
    String fileName_;
    String packageName_;
    std::unordered_set<String, StringHashFunc, StringEqualFunc> imports_;
};

class Preprocessor {
public:
    using FileDetailMap = std::unordered_map<String, FileDetail, StringHashFunc, StringEqualFunc>;

    // analyze idl files and return sorted ids files
    bool Preprocess(std::vector<String> &compileSourceFiles);

private:
    bool CheckAllFilesPath(const std::vector<String> &sourceFiles);

    bool AnalyseImportInfo(const std::vector<String> &sourceFiles, FileDetailMap &allFileDetails);

    bool ParseFileDetail(const String &sourceFile, FileDetail &info);

    bool ParsePackage(Lexer &lexer, FileDetail &info);

    bool ParseImports(Lexer &lexer, FileDetail &info);

    bool LoadOtherIdlFiles(const FileDetail &ownerFileDetail, FileDetailMap &allFileDetails);

    bool CheckCircularReference(FileDetailMap &allFileDetails, std::vector<String> &compileSourceFiles);

    void PrintCyclefInfo(FileDetailMap &allFileDetails);

    void FindCycle(const String &curNode, FileDetailMap &allFiles, std::vector<String> &trace);

private:
    static constexpr char *TAG = "Preprocessor";
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_PREPROCESSOR_H