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

#include <inviwo/cloudfiles/cloudfiles.hpp>

#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/std.h>
#include <tclap/CmdLine.h>

int main(int argc, char** argv) {
    using namespace inviwo;

    TCLAP::CmdLine cmd{"Inviwo cloud files tool: query and control OneDrive/iCloud on-demand files",
                       ' '};

    TCLAP::SwitchArg download{"d", "download", "Download (hydrate) the given files", false};
    TCLAP::SwitchArg offload{"o", "offload", "Offload (dehydrate/evict) the given files", false};
    TCLAP::UnlabeledMultiArg<std::string> paths{"paths", "Files to inspect or operate on", true,
                                                "path"};

    cmd.add(download);
    cmd.add(offload);
    cmd.add(paths);

    try {
        cmd.parse(argc, argv);

        if (download.getValue() && offload.getValue()) {
            throw std::runtime_error("--download and --offload are mutually exclusive");
        }

        if (!cloudfiles::isSupported()) {
            fmt::print("Warning: cloud file operations are not supported on this platform.\n");
        }

        int failures = 0;
        for (const auto& p : paths.getValue()) {
            const std::filesystem::path path{p};
            try {
                if (download.getValue()) {
                    cloudfiles::download(path);
                    fmt::print("download requested: {}\n", path);
                } else if (offload.getValue()) {
                    cloudfiles::offload(path);
                    fmt::print("offload requested:  {}\n", path);
                }

                const auto s = cloudfiles::status(path);
                fmt::print("{:<14} placeholder={:<5} size={:>12} {}\n",
                           cloudfiles::toString(s.availability), s.isPlaceholder,
                           s.size ? std::to_string(*s.size) : std::string{"?"}, path);
            } catch (const std::exception& e) {
                fmt::print("error: {}: {}\n", path, e.what());
                ++failures;
            }
        }
        return failures == 0 ? 0 : 1;
    } catch (const TCLAP::ArgException& e) {
        fmt::print("error: {} for arg {}\n", e.error(), e.argId());
        return 1;
    } catch (const std::exception& e) {
        fmt::print("error: {}\n", e.what());
        return 1;
    }
}
