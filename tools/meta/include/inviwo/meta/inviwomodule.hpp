/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/meta/inviwometadefine.hpp>
#include <inviwo/meta/cmake/cmakefile.hpp>
#include <inviwo/meta/inviwomoduleconf.hpp>

#include <filesystem>
#include <string_view>
#include <optional>
#include <array>
#include <string>
#include <variant>

namespace inviwo::meta {

class INVIWO_META_API InviwoModule {
public:
    enum class Type { Module, LegacyModule, Core, QtEditor };

    using Config = std::variant<ModuleConf, LegacyModuleConf, InviwoConf, QtEditorConf>;

    InviwoModule(const std::filesystem::path& path, Config config,
                 std::optional<std::string_view> contents = {});

    ~InviwoModule();
    cmake::CMakeFile& cMakeFile();
    const cmake::CMakeFile& cMakeFile() const;

    void setDryrun(bool dryrun);
    void setContents(std::string_view contents);

    const std::filesystem::path& path() const;
    std::string name() const;
    std::string api() const;
    std::string moduleInclude() const;
    std::string defineInclude() const;
    std::filesystem::path registrationFile() const;
    std::filesystem::path cmakelistsFile() const;
    std::filesystem::path incPath() const;
    std::filesystem::path srcPath() const;

    std::filesystem::path getHeaderPath(const std::filesystem::path&) const;
    std::filesystem::path getSourcePath(const std::filesystem::path&) const;
    std::filesystem::path getOtherPath(const std::filesystem::path&) const;
    std::string getHeaderInclude(const std::filesystem::path&) const;

    cmake::Command* findGroup(std::string_view group);
    void addFileToGroup(std::string_view group, const std::filesystem::path& path);
    void addHeaderFile(const std::filesystem::path& path);
    void addSourceFile(const std::filesystem::path& path);
    void addShaderFile(const std::filesystem::path& path);
    void addTestFile(const std::filesystem::path& path);

    void registerProcessor(std::string_view name, const std::filesystem::path& incPath);

    void save();

    std::vector<std::pair<std::filesystem::path, std::string>> dirs() const;

    static InviwoModule findInviwoModule(const std::filesystem::path& path,
                                         const std::filesystem::path& inviwoRepo);

    static std::optional<std::string> findModuleName(const cmake::CMakeFile& cmf);

    static std::filesystem::path findShortestRelativePath(const std::filesystem::path& path,
                                                          std::vector<std::filesystem::path> bases);

private:
    static inline constexpr std::string_view headerGroup{"HEADER_FILES"};
    static inline constexpr std::string_view sourceGroup{"SOURCE_FILES"};
    static inline constexpr std::string_view shaderGroup{"SHADER_FILES"};
    static inline constexpr std::string_view testGroup{"TEST_FILES"};
    static inline constexpr std::string_view moduleCmd{"ivw_module"};

    static void sortArgs(cmake::Command& cmd);
    static void addArg(std::vector<cmake::ArgElement>& args, std::string_view arg);
    static void cleanArgs(cmake::Command& cmd);

    Config config_;
    std::filesystem::path path_;
    cmake::CMakeFile cmf_;

    bool dryrun = false;
};

}  // namespace inviwo::meta
