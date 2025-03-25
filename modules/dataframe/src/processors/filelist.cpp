/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframe/processors/filelist.h>

#include <filesystem>
#include <ranges>
#include <vector>

#include <fmt/chrono.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FileList::processorInfo_{
    "org.inviwo.FileList",    // Class identifier
    "File List",              // Display name
    "DataFrame",              // Category
    CodeState::Experimental,  // Code state
    Tags::CPU,                // Tags
    R"(Generate a DataFrame from a file listing of a directory)"_unindentHelp,
};

const ProcessorInfo& FileList::getProcessorInfo() const { return processorInfo_; }

FileList::FileList()
    : Processor{}
    , outport_{"outport", "<description of the generated outport data>"_help}
    , bnl_{"bnl"}
    , directory_{"directory", "Directory"}
    , selected_{"selected", "Selected"}
    , highlight_{"highlight", "Highlight"} {

    addPorts(outport_, bnl_);
    addProperties(directory_, selected_, highlight_);

    selected_.setReadOnly(true);
    highlight_.setReadOnly(true);
}

void FileList::process() {
    auto files = std::filesystem::directory_iterator(directory_.get()) |
                 std::views::filter([](auto& item) { return item.is_regular_file(); }) |
                 std::ranges::to<std::vector>();
    std::ranges::sort(files);

    auto df = std::make_shared<DataFrame>();

    df->addCategoricalColumn("Name", files | std::views::transform([](auto& item) {
                                         return item.path().filename().generic_string();
                                     }) | std::ranges::to<std::vector>());
    df->addColumn("Size", files | std::views::transform([](auto& item) {
                              return static_cast<int>(item.file_size());
                          }) | std::ranges::to<std::vector>());

    /*
    df->addCategoricalColumn("Modified", files | std::views::transform([](auto& item) {
                                             return fmt::format("{:%Y-%m-%d %H:%M:%S}",
                                                                item.last_write_time());
                                         }) | std::ranges::to<std::vector>());
    */

    df->updateIndexBuffer();
    outport_.setData(df);

    auto& sel = bnl_.getSelectedIndices();
    if (sel.empty()) {
        selected_.set("");
    } else {
        selected_.set(files.at(sel.min()).path());
    }

    auto& high = bnl_.getHighlightedIndices();
    if (high.empty()) {
        highlight_.set("");
    } else {
        highlight_.set(files.at(high.min()).path());
    }
}

}  // namespace inviwo
