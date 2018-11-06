/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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

#include <inviwo/meta/creator.hpp>
#include <inviwo/meta/paths.hpp>
#include <inviwo/meta/util.hpp>
#include <inviwo/meta/inviwomoduleconf.hpp>

#include <exception>
#include <array>
#include <string>
#include <algorithm>

#include <warn/push>
#include <warn/ignore/all>
#include <nlohmann/json.hpp>
#include <inja.hpp>
#include <warn/pop>

namespace inviwo::meta {

namespace json = ::nlohmann;
using namespace std::literals;
namespace fs = std::filesystem;
using namespace fmt::literals;

Creator::Creator(const fs::path& inviwoRepo, std::optional<fs::path> templateDir,
                 std::optional<fs::path> configFile, Options aOpts)
    : opts{std::move(aOpts)}
    , inviwoRepo_{inviwoRepo}
    , templateDir_{templateDir ? *templateDir : inviwoRepo / "tools" / "meta" / "templates"}
    , configFile_{configFile ? *configFile : templateDir_ / "config.json"} {

    if (!fs::exists(templateDir_)) {
        throw util::makeError("Template directory \"{}\" not found", templateDir_.string());
    }
}

void Creator::createFile(const fs::path& filePath) const { generate(filePath, "file"); }

void Creator::createTest(const fs::path& testPath) const { generate(testPath, "test"); }

void Creator::createProcessor(const fs::path& processorPath) const {
    generate(processorPath, "processor");
}

json::json Creator::createSettings(const InviwoModule& im, std::string_view fileName) const {
    const auto dfName =
        std::regex_replace(std::string{fileName}, std::regex("([a-z])([A-Z])"), "$1 $2");
    return {{"module",
             {{"name", im.name()},
              {"api", im.api()},
              {"include", im.moduleInclude()},
              {"define_include", im.defineInclude()}}},
            {"file", {{"name", fileName}, {"disp_name", dfName}}}};
}

void Creator::generate(const std::filesystem::path& filePath, std::string_view tag) const {
    auto im = InviwoModule::findInviwoModule(filePath.parent_path(), inviwoRepo_);
    im.setDryrun(opts.dryrun);

    const auto name = filePath.filename().string();
    const auto lname = util::toLower(name);
    const auto path = fs::weakly_canonical(filePath.parent_path().empty() ? fs::current_path()
                                                                          : filePath.parent_path());

    const auto& files = templates_[std::string(tag)];
    const auto incpath = files.header
                             ? im.getHeaderInclude(path / (lname + "." + files.header->extension))
                             : "NOT_USED";
    std::string type{tag};
    type[0] = static_cast<char>(::toupper(type[0]));

    logf("Creating {} '{}' at '{}'", type, name, path.generic_string());
    log(" * Settings");
    log("Module", im.name());
    log("Module Include", im.moduleInclude());
    log("File Include", incpath);
    log("API Macro", im.api());
    log("API Include", im.defineInclude());
    log(" * Creating files:");

    generate(im, filePath, tag);
}

void Creator::generate(InviwoModule& im, const std::filesystem::path& filePath,
                       std::string_view tag) const {
    const auto name = filePath.filename().string();
    const auto lname = util::toLower(name);
    const auto path = fs::weakly_canonical(filePath.parent_path().empty() ? fs::current_path()
                                                                          : filePath.parent_path());
    auto env =
        inja::Environment{templateDir_.generic_string() + "/", im.path().generic_string() + "/"};

    const auto& files = templates_[std::string(tag)];
    auto settings = createSettings(im, name);

    const fs::path incpath =
        files.header ? im.getHeaderInclude(path / (lname + "." + files.header->extension))
                     : fs::path{"NOT_USED"};
    settings["file"]["include"] = incpath.generic_string();

    auto cmake = [&](const std::optional<std::string>& cmakeGroup,
                     const std::filesystem::path& cmakepath) {
        if (!opts.dryrun && cmakeGroup) {
            logf("    - Register {} as '{}' in {} : {}", name, cmakepath.generic_string(),
                 im.cmakelistsFile().generic_string(), *cmakeGroup);
            im.addFileToGroup(*cmakeGroup, cmakepath);
        }
    };

    auto write = [&](const File& file, const std::filesystem::path& dst) {
        log(dst.generic_string(), file.description);

        if (!opts.force && fs::exists(dst)) {
            throw util::makeError("Error: file '{}' already exits, use --force to overwrite",
                                  dst.generic_string());
        }
        if (!opts.dryrun) {
            env.write(env.parse_template(file.templateFile), settings, dst.generic_string());
        }
        cmake(file.cmakeGroup, dst);
    };

    if (auto header = files.header) {
        write(*header, im.getHeaderPath(path / (lname + "." + header->extension)));
    }
    if (auto source = files.source) {
        write(*source, im.getSourcePath(path / (lname + "." + source->extension)));
    }
    if (auto other = files.other) {
        write(*other, im.getOtherPath(path / (lname + "." + other->extension)));
    }
    if (files.registration && !opts.dryrun) {
        logf("    - Register {} '{}' in {}", *files.registration, name,
             im.registrationFile().generic_string());
        im.registerProcessor(name, incpath);
    }
}

void Creator::createModule(const fs::path& modulePath, std::string_view org) const {
    const auto name = modulePath.filename().generic_string();
    const auto path =
        fs::weakly_canonical(modulePath.parent_path().empty() ? fs::current_path()
                                                              : modulePath.parent_path()) /
        util::toLower(name);
    logf("Creating module '{}' at '{}'", name, path.generic_string());
    if (fs::exists(path) && !opts.force) {
        throw util::makeError("Error: Module folder for \"{}\" already exits", name);
    }

    InviwoModule im(path, ModuleConf{name, org}, "");
    im.setDryrun(opts.dryrun);

    const auto settings = createSettings(im, name);
    inja::Environment env{templateDir_.generic_string() + "/", path.generic_string() + "/"};
    const auto cmakeconf = *templates_["cmakelists"].other;
    im.setContents(env.render_file(cmakeconf.templateFile, settings));

    log(" * Settings");
    log("Name", im.name());
    log("Include", im.moduleInclude());
    log("API", im.api());
    log("API Include", im.defineInclude());

    log(" * Creating folders:");
    if (!opts.dryrun) fs::create_directories(path);
    for (auto [dir, desc] : im.dirs()) {
        log(dir.generic_string(), desc);
        if (!opts.dryrun && !fs::exists(path / dir)) {
            fs::create_directories(path / dir);
        }
    }

    log(" * Creating files:");
    log(cmakeconf.templateFile, cmakeconf.description);

    const std::vector<std::pair<std::string, fs::path>> files = {
        {"module", path / im.incPath() / (name + "module")},
        {"api", path / im.incPath() / (name + "moduledefine")},
        {"readme", path / "readme"},
        {"depends", path / "depends"},
        {"testmain", path / "tests" / "unittests" / (name + "-unittest-main")}};

    for (auto [key, file] : files) {
        generate(im, file, key);
    }
}

void Creator::log(std::string_view message) const {
    if (opts.verbose) {
        opts.log << message << "\n";
    }
}
void Creator::log(std::string_view key, std::string_view message) const {
    logf("  - {:50} {}", key, message);
}

}  // namespace inviwo::meta
