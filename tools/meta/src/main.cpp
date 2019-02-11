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

#include <inviwo/meta/paths.hpp>
#include <inviwo/meta/creator.hpp>

#include <filesystem>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <tclap/CmdLine.h>

int main(int argc, char** argv) {
    TCLAP::CmdLine cmd{"Inviwo meta tool", ' '};

    TCLAP::SwitchArg verbose{"v", "verbose", "Print extra information", false};
    TCLAP::SwitchArg dryrun{"d", "dryrun", "Don't actually write anything", false};
    TCLAP::SwitchArg force{"", "force", "Overwrite any existing files", false};
    TCLAP::ValueArg<std::string> templateDir{
        "",
        "templates",
        "Path to the templates directory. If not given, the templates folder in the "
        "Inviwo-Tools folder will be used.",
        false,
        "",
        "template folder"};

    TCLAP::ValueArg<std::string> inviwoDir{
        "",    "inviwo", "Path to the inviwo repository. Tries to find it in the current path",
        false, "",       "inviwo repo"};

    TCLAP::ValueArg<std::string> org{"o",   "org",    "Organization (defaults to 'inviwo')",
                                     false, "inviwo", "Organization"};

    TCLAP::MultiArg<std::string> modules{
        "m", "modules", "Modules to add, form: path/name1 path/name2 ...", false, "module"};

    TCLAP::MultiArg<std::string> files{
        "f", "files", "Files to add, form: path/name1 path/name2 ...", false, "file"};

    TCLAP::MultiArg<std::string> processors{"p", "processors",
                                            "Processors to add, form: path/name1 path/name2 ...",
                                            false, "processor"};

    TCLAP::MultiArg<std::string> tests{
        "t", "tests", "Tests to add, form: path/name1 path/name2 ...", false, "test"};

    TCLAP::MultiArg<std::string> updateModule{
        "", "updateModule",
        "Update a module to use include and src folders\n"
        "Will move all .h file into the include/<org>/<module> sub folder\n"
        "and all .cpp into the src folder\n"
        "except for files under /ext, /tests, or paths excluded be the given updatePathFilter.",
        false, "module"};

    TCLAP::MultiArg<std::string> updatePathFilter{
        "", "updatePathFilter", "Exclude file matching filter from module update", false, "filter"};

    cmd.add(verbose);
    cmd.add(dryrun);
    cmd.add(force);
    cmd.add(templateDir);
    cmd.add(inviwoDir);
    cmd.add(org);
    cmd.add(modules);
    cmd.add(files);
    cmd.add(processors);
    cmd.add(tests);
    cmd.add(updateModule);
    cmd.add(updatePathFilter);

    auto getInviwoPath = [&]() -> std::filesystem::path {
        if (inviwoDir.isSet()) {
            return std::filesystem::canonical(std::filesystem::path{inviwoDir.getValue()});
        }
        const auto foundInviwoDir = inviwo::meta::findInviwoPath({std::filesystem::current_path()});
        if (foundInviwoDir) {
            return *foundInviwoDir;
        } else {
            throw std::runtime_error("Error could not find inviwo repo");
        }
    };

    auto getTemplatesPath = [&]() -> std::filesystem::path {
        if (templateDir.isSet()) {
            return std::filesystem::canonical(std::filesystem::path{templateDir.getValue()});
        }
        return getInviwoPath() / "tools" / "meta" / "templates";
    };

    try {
        cmd.parse(argc, argv);
        inviwo::meta::Creator creator(
            getInviwoPath(), getTemplatesPath(), {},
            {verbose.getValue(), dryrun.getValue(), force.getValue(), std::cout});

        if (modules.isSet()) {
            for (auto& item : modules.getValue()) {
                creator.createModule(std::filesystem::path{item}, org.getValue());
            }
        }

        if (files.isSet()) {
            for (auto& item : files.getValue()) {
                creator.createFile(std::filesystem::path{item});
            }
        }

        if (processors.isSet()) {
            for (auto& item : processors.getValue()) {
                creator.createProcessor(std::filesystem::path{item});
            }
        }

        if (tests.isSet()) {
            for (auto& item : tests.getValue()) {
                creator.createTest(std::filesystem::path{item});
            }
        }

        if (updateModule.isSet()) {
            for (auto& item : updateModule.getValue()) {
                creator.updateModule(std::filesystem::path{item}, org.getValue(),
                                     updatePathFilter.getValue());
            }
        }

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
