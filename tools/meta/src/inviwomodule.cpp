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

#include <inviwo/meta/inviwomodule.hpp>
#include <inviwo/meta/inviwomoduleregistration.hpp>
#include <inviwo/meta/util.hpp>

#include <fstream>
#include <streambuf>
#include <regex>
#include <sstream>

#include <fmt/format.h>

namespace inviwo::meta {

InviwoModule::InviwoModule(const std::filesystem::path& path, Config config,
                           std::optional<std::string_view> contents)
    : config_{config}, path_{path}, cmf_{[&]() {
        if (contents) {
            return cmake::CMakeFile(*contents);
        } else {
            return cmake::CMakeFile(path_ / cmakelistsFile());
        }
    }()} {}

InviwoModule::~InviwoModule() {
    if (!std::uncaught_exceptions()) {
        save();
    }
}

void InviwoModule::setContents(std::string_view contents) { cmf_ = cmake::CMakeFile(contents); }

void InviwoModule::setDryrun(bool newDryrun) { dryrun = newDryrun; }

void InviwoModule::save() {
    if (dryrun) return;
    std::ofstream os{path_ / cmakelistsFile()};
    cmf_.print(os);
}

cmake::CMakeFile& InviwoModule::cMakeFile() { return cmf_; }
const cmake::CMakeFile& InviwoModule::cMakeFile() const { return cmf_; }

const std::filesystem::path& InviwoModule::path() const { return path_; }
std::string InviwoModule::name() const {
    return std::visit([](const auto& c) { return c.name(); }, config_);
}
std::string InviwoModule::api() const {
    return std::visit([](const auto& c) { return c.api(); }, config_);
}
std::string InviwoModule::moduleInclude() const {
    return std::visit([](const auto& c) { return c.moduleInclude(); }, config_).generic_string();
}
std::string InviwoModule::defineInclude() const {
    return std::visit([](const auto& c) { return c.defineInclude(); }, config_).generic_string();
}
std::filesystem::path InviwoModule::registrationFile() const {
    return std::visit([](const auto& c) { return c.registrationFile(); }, config_);
}
std::filesystem::path InviwoModule::cmakelistsFile() const {
    return std::visit([](const auto& c) { return c.listFile(); }, config_);
}
std::filesystem::path InviwoModule::incPath() const {
    return std::visit([](const auto& c) { return c.incPath(); }, config_);
}
std::filesystem::path InviwoModule::srcPath() const {
    return std::visit([](const auto& c) { return c.srcPath(); }, config_);
}

std::filesystem::path InviwoModule::getHeaderPath(const std::filesystem::path& path) const {
    const auto relPath = findShortestRelativePath(path, {path_ / incPath(), path_ / srcPath()});
    return incPath() / relPath;
}
std::filesystem::path InviwoModule::getSourcePath(const std::filesystem::path& path) const {
    const auto relPath = findShortestRelativePath(path, {path_ / incPath(), path_ / srcPath()});
    return srcPath() / relPath;
}
std::filesystem::path InviwoModule::getOtherPath(const std::filesystem::path& path) const {
    return findShortestRelativePath(path, {path_});
}

std::string InviwoModule::getHeaderInclude(const std::filesystem::path& path) const {
    const auto relPath = findShortestRelativePath(path, {path_ / incPath(), path_ / srcPath()});
    return (std::visit([](const auto& c) { return c.includePrefix(); }, config_) / relPath)
        .generic_string();
}

std::filesystem::path InviwoModule::findShortestRelativePath(
    const std::filesystem::path& path, std::vector<std::filesystem::path> bases) {
    if (auto res = util::findShortestRelativePath(path, bases)) {
        return *res;
    } else {
        std::stringstream ss;
        bool notfirst = false;
        for (auto&& l : bases) {
            if (notfirst) ss << " or ";
            notfirst = true;
            ss << "'" << l.generic_string() << "'";
        }
        throw util::makeError("Error: File '{}' should be in folder {}", path.generic_string(),
                              ss.str());
    }
}

cmake::Command* InviwoModule::findGroup(std::string_view group) {
    auto it = std::find_if(cmf_.begin(), cmf_.end(), [&](auto& cmd) {
        return cmd.identifier == "set" && cmd.begin() != cmd.end() && cmd.begin()->value == group;
    });
    if (it != cmf_.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

void InviwoModule::addFileToGroup(std::string_view group, const std::filesystem::path& path) {
    auto p = std::filesystem::relative(path_ / path, (path_ / cmakelistsFile()).parent_path());

    if (*p.begin() == "..") {
        p = "${IVW_INCLUDE_DIR}" / std::filesystem::relative(path, "include");
    }

    if (group.empty()) return;

    if (auto it = findGroup(group)) {
        auto& cmd = *it;
        const auto pstr = p.generic_string();

        auto pit =
            std::find_if(cmd.begin(), cmd.end(), [&](auto& arg) { return arg.value == pstr; });
        if (pit != cmd.end()) {
            throw util::makeError("Error: File '{}' already in group '{}'", pstr, group);
        }

        addArg(cmd.arguments, pstr);
        cleanArgs(cmd);
        sortArgs(cmd);

    } else {
        throw std::runtime_error(fmt::format("Could not find source group: '{}'", group));
    }
}
void InviwoModule::addHeaderFile(const std::filesystem::path& path) {
    addFileToGroup(headerGroup, path);
}
void InviwoModule::addSourceFile(const std::filesystem::path& path) {
    addFileToGroup(sourceGroup, path);
}
void InviwoModule::addShaderFile(const std::filesystem::path& path) {
    addFileToGroup(shaderGroup, path);
}
void InviwoModule::addTestFile(const std::filesystem::path& path) {
    addFileToGroup(testGroup, path);
}

void InviwoModule::sortArgs(cmake::Command& cmd) {
    std::vector<std::string> args;
    std::transform(++cmd.begin(), cmd.end(), std::back_inserter(args),
                   [](const cmake::Argument& arg) { return arg.value; });
    std::sort(args.begin(), args.end());

    std::vector<cmake::ArgElement> arguments;
    arguments.emplace_back(*cmd.begin());
    arguments.emplace_back(cmake::LineEnding{});
    for (auto& arg : args) {
        addArg(arguments, arg);
    }
    cmd.arguments = arguments;
}

void InviwoModule::addArg(std::vector<cmake::ArgElement>& args, std::string_view arg) {
    args.emplace_back(cmake::Space{"    "});
    args.emplace_back(cmake::Argument{std::string(arg)});
    args.emplace_back(cmake::LineEnding{});
}

void InviwoModule::cleanArgs(cmake::Command& cmd) {
    const std::string_view toStrip = "${CMAKE_CURRENT_SOURCE_DIR}/";
    auto args = cmd.args();
    for (auto& arg : Range{++args.begin(), args.end()}) {
        arg.value = util::removePrefix(toStrip, arg.value);
    }
}

void InviwoModule::registerProcessor(std::string_view name, const std::filesystem::path& incPath) {
    InviwoModuleRegistration mr(path_ / registrationFile());
    mr.registerProcessor(name, incPath);
}

std::optional<std::string> InviwoModule::findModuleName(const cmake::CMakeFile& cmf) {
    auto it = std::find_if(cmf.begin(), cmf.end(),
                           [](auto& cmd) { return cmd.identifier == "ivw_module"; });
    if (it != cmf.end() && it->begin() != it->end()) {
        return it->begin()->value;
    }
    return {};
}

InviwoModule InviwoModule::findInviwoModule(const std::filesystem::path& inpath,
                                            const std::filesystem::path& inviwoRepo) {

    auto path = std::filesystem::weakly_canonical(inpath);

    const std::filesystem::path inccore = inviwoRepo / "include" / "inviwo" / "core";
    const std::filesystem::path incqt = inviwoRepo / "include" / "inviwo" / "qt" / "editor";
    const std::filesystem::path srccore = inviwoRepo / "src" / "core";
    const std::filesystem::path srcqt = inviwoRepo / "src" / "qt" / "editor";

    if (std::mismatch(inccore.begin(), inccore.end(), path.begin(), path.end()).first ==
            inccore.end() ||
        std::mismatch(srccore.begin(), srccore.end(), path.begin(), path.end()).first ==
            srccore.end()) {

        return InviwoModule(inviwoRepo, InviwoConf{});
    }
    if (std::mismatch(incqt.begin(), incqt.end(), path.begin(), path.end()).first == incqt.end() ||
        std::mismatch(srcqt.begin(), srcqt.end(), path.begin(), path.end()).first == srcqt.end()) {

        return InviwoModule(inviwoRepo, QtEditorConf{});
    }

    auto findOrg = [](const std::filesystem::path& modulePath,
                      const std::string& name) -> std::string {
        const auto lname = util::toLower(name);

        if (std::filesystem::exists(modulePath / "include" / lname)) {
            return "";  // no org name;
        } else {
            for (auto&& item : std::filesystem::directory_iterator(modulePath / "include")) {
                if (std::filesystem::exists(item.path() / lname)) {
                    return item.path().filename().generic_string();
                }
            }
        }
        throw util::makeError("Error: unable to find org name of module");
    };

    auto p = path;
    for (; p != p.parent_path(); p = p.parent_path()) {
        if (std::filesystem::exists(p / "CMakeLists.txt")) {
            const auto cmf = cmake::CMakeFile{p / "CMakeLists.txt"};
            if (auto name = findModuleName(cmf)) {
                if (std::filesystem::exists(p / "include") && std::filesystem::exists(p / "src")) {
                    return InviwoModule{p, ModuleConf{*name, findOrg(p, *name)}};
                } else {
                    return InviwoModule{p, LegacyModuleConf{*name}};
                }
            }
        }
    }
    throw util::makeError("Error: Unable to find Inviwo Module");
}

std::vector<std::pair<std::filesystem::path, std::string>> InviwoModule::dirs() const {
    return {{"data", "Folder for non code stuff"},
            {"data/images", "Image resources"},
            {"data/volumes", "Volume resources "},
            {"data/workspaces", "Workspaces, listed in File::Examples::ExampleModule"},
            {"docs", "Put documentation here"},
            {"docs/images", "Put images that should show up in doxygen here"},
            {incPath(), "Put headers here"},
            {srcPath(), "Put sources here"},
            {incPath() / "processors", "Put Processor headers here"},
            {srcPath() / "processors", "Put Processor sources here"},
            {incPath() / "properties", "Put Property headers here"},
            {srcPath() / "properties", "Put Property sources here"},
            {"tests", "Test related things"},
            {"tests/unittests", "Put unittests here"},
            {"tests/regression",
             "Regression Test workspaces, listed in File::Test::ExampleModule. Automatically "
             "run in regression tests on Jenkins"}};
}

}  // namespace inviwo::meta
