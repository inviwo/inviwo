/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/sourcetools.h>

#include <inviwo/core/io/curlutils.h>
#include <inviwo/core/util/buildinfo.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/inviwomodule.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <QDesktopServices>
#include <QString>
#include <QUrl>

namespace inviwo::util {

namespace {

InviwoModule* findModuleForFile(ModuleManager* mm, const std::filesystem::path& file) {
    std::filesystem::path best;
    InviwoModule* inviwoModule = nullptr;
    for (auto& m : mm->getInviwoModules()) {
        const auto relpath = file.lexically_relative(m.getPath());
        if (!relpath.empty() && !relpath.string().starts_with("..")) {
            if (best.empty() || relpath.string().length() < best.string().length()) {
                best = relpath;
                inviwoModule = &m;
            }
        }
    }
    return inviwoModule;
}

auto headerCandidates(const std::filesystem::path& relSrc, const std::filesystem::path& base,
                      std::string_view moduleId) -> std::vector<std::filesystem::path> {
    if (*relSrc.begin() == "src") {
        std::filesystem::path header{};
        for (auto&& part : std::ranges::subrange(++relSrc.begin(), relSrc.end())) {
            header /= part;
        }
        header.replace_extension(".h");

        return {base / "include" / "modules" / toLower(moduleId) / header,
                base / "include" / "inviwo" / toLower(moduleId) / header};
    }
    return {};
}

bool openLocalUrl(const std::filesystem::path& url) {
    QDesktopServices::openUrl(
        QUrl(utilqt::toQString("file:///" + url.generic_string()), QUrl::TolerantMode));
    return true;
}
bool openUrl(const std::string& url) {
    QDesktopServices::openUrl(QUrl(utilqt::toQString(url), QUrl::TolerantMode));
    return true;
}

std::optional<std::string> getRepoLink(const std::filesystem::path& path) {
    auto bi = util::getBuildInfo();

    auto file = path.generic_string();
    for (auto&& modulesDir : bi->modulesDirs) {
        if (!modulesDir.repo.contains("github.com")) continue;
        if (!file.starts_with(modulesDir.repoDir.generic_string())) continue;

        file.erase(0, modulesDir.repoDir.generic_string().size());
        const auto repoLink = modulesDir.repo.ends_with(".git")
                                  ? modulesDir.repo.substr(0, modulesDir.repo.size() - 4)
                                  : modulesDir.repo;
        return fmt::format("{}/blob/{}{}", repoLink, modulesDir.sha, file);
    }
    return std::nullopt;
}

}  // namespace

void openProcessorFile(OpenProcessorFile config) {
    auto* inviwoModule = findModuleForFile(config.manager, config.cppFile);

    const auto candidates =
        inviwoModule ? headerCandidates(config.cppFile.lexically_relative(inviwoModule->getPath()),
                                        inviwoModule->getPath(), inviwoModule->getIdentifier())
                     : std::vector<std::filesystem::path>{};

    if (config.header) {
        if (!config.web) {
            for (auto&& candidate : candidates) {
                if (std::filesystem::is_regular_file(candidate)) {
                    openLocalUrl(candidate);
                    return;
                }
            }
        }
        for (auto&& candidate : candidates) {
            if (getRepoLink(candidate)
                    .and_then([](auto&& url) {
                        return net::urlExists(url) ? url : std::optional<std::string>{};
                    })
                    .transform(openUrl)
                    .value_or(false)) {
                return;
            }
        }

    } else {
        if (!config.web && std::filesystem::is_regular_file(config.cppFile)) {
            openLocalUrl(config.cppFile);
            return;
        }

        getRepoLink(config.cppFile).transform(openUrl);
    }
}

}  // namespace inviwo::util
