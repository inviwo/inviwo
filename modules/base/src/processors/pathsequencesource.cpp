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

#include <modules/base/processors/pathsequencesource.h>

#include <filesystem>
#include <ranges>
#include <vector>
#include <regex>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PathSequenceSource::processorInfo_{
    "org.inviwo.PathSequenceSource",  // Class identifier
    "Path Sequence Source",           // Display name
    "Undefined",                      // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
    R"(Create a PathSequence from a directory)"_unindentHelp,
};

const ProcessorInfo& PathSequenceSource::getProcessorInfo() const { return processorInfo_; }

PathSequenceSource::PathSequenceSource()
    : Processor{}
    , outport_{"outport", "List of found paths"_help}
    , directory_{"directory", "Directory"}
    , filter_{"filter", "Filter",
              "Set a regexp filter to match filenames to include in the list"_help}
    , recursive_{"recursive", "Recursive"}
    , refresh_{"refresh", "Refresh"} {

    addPorts(outport_);
    addProperties(directory_, filter_, recursive_, refresh_);
}

void PathSequenceSource::process() {

    std::optional<std::regex> regex;
    if (!filter_.get().empty()) regex.emplace(filter_.get());

    const auto regularFilter =
        std::views::filter([](auto& item) { return item.is_regular_file(); });

    const auto regexFilter = std::views::filter([&](auto& item) {
        return regex ? std::regex_search(item.path().generic_string(), *regex) : true;
    });

    auto files = [&]() {
        if (recursive_.get()) {
            return std::filesystem::recursive_directory_iterator(directory_.get()) | regularFilter |
                   regexFilter | std::ranges::to<std::vector>();
        } else {
            return std::filesystem::directory_iterator(directory_.get()) | regularFilter |
                   regexFilter | std::ranges::to<std::vector>();
        }
    }();

    std::ranges::sort(files);

    auto data = std::make_shared<DataSequence<std::filesystem::path>>();
    for (const auto& file : files) {
        data->emplace_back(std::make_shared<std::filesystem::path>(file));
    }

    outport_.setData(data);
}

}  // namespace inviwo
