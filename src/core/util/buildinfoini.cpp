/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <inviwo/core/util/buildinfo.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/serialization/deserializer.h>

#include <fstream>
#include <locale>

namespace inviwo {

namespace util {

namespace {

struct DeserializeModuleDir : BuildInfo::ModulesDir {
    void deserialize(Deserializer& d) {
        d.deserialize("name", name, SerializationTarget::Attribute);
        d.deserialize("dir", dir, SerializationTarget::Attribute);
        d.deserialize("sha", sha, SerializationTarget::Attribute);
        d.deserialize("repo", repo, SerializationTarget::Attribute);
        d.deserialize("repoDir", repoDir, SerializationTarget::Attribute);
        d.deserialize("dirty", dirty, SerializationTarget::Attribute);
    }
};
}  // namespace

const std::optional<BuildInfo>& getBuildInfo() {
    static constexpr std::string_view xmlFileName = "inviwo_buildinfo.xml";

    const auto maybeIn = []() -> std::optional<std::filesystem::path> {
        auto exeDir = filesystem::getExecutablePath().parent_path();
        auto dir = exeDir / xmlFileName;
        if (std::filesystem::is_regular_file(dir)) return dir;

#if defined(__APPLE__)
        if (dir.parent_path().filename() == "MacOS") {
            dir = exeDir.parent_path().parent_path().parent_path() / xmlFileName;
            if (std::filesystem::is_regular_file(dir)) return dir;
        }
#endif

        dir = filesystem::getInviwoBinDir() / xmlFileName;
        if (std::filesystem::is_regular_file(dir)) return dir;

        return std::nullopt;
    }();

    static const std::optional<BuildInfo> maybebuildInfo =
        maybeIn.and_then([](const std::filesystem::path& file) -> std::optional<BuildInfo> {
            BuildInfo buildInfo;

            try {
                Deserializer d{file, "BuildInfo"};

                d.deserialize("year", buildInfo.year, SerializationTarget::Attribute);
                d.deserialize("month", buildInfo.month, SerializationTarget::Attribute);
                d.deserialize("day", buildInfo.day, SerializationTarget::Attribute);
                d.deserialize("hour", buildInfo.hour, SerializationTarget::Attribute);
                d.deserialize("minute", buildInfo.minute, SerializationTarget::Attribute);
                d.deserialize("second", buildInfo.second, SerializationTarget::Attribute);

                std::vector<DeserializeModuleDir> modulesDirs;
                d.deserialize("ModulesDirs", modulesDirs, "ModulesDir");

                for (auto& md : modulesDirs) {
                    buildInfo.modulesDirs.emplace_back(md.name, md.dir, md.sha, md.repo, md.repoDir,
                                                       md.dirty);
                }
                return buildInfo;
            } catch (const Exception& e) {
                log::exception(e);
            } catch (const std::exception& e) {
                log::exception(e);
            } catch (...) {
                log::exception();
            }
            return std::nullopt;
        });

    return maybebuildInfo;
}

}  // namespace util

}  // namespace inviwo
