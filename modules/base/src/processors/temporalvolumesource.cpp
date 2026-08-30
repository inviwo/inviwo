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

#include <modules/base/processors/temporalvolumesource.h>

#include <modules/base/io/filesequenceloader.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/fileextensionutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <utility>

#include <fmt/std.h>

namespace inviwo {

const ProcessorInfo TemporalVolumeSource::processorInfo_{
    "org.inviwo.TemporalVolumeSource",  // Class identifier
    "Temporal Volume Source",           // Display name
    "Data Input",                       // Category
    CodeState::Experimental,            // Code state
    Tags::CPU | Tag{"Volume"} | Tag{"Temporal"},
    R"(Loads a sequence of volume files from a folder as a lazily-loaded TemporalVolume. Only the
    metadata of the first frame is read up front, individual frames are loaded on demand and kept
    in a bounded cache. Suitable for large time-varying volume data.)"_unindentHelp,
};

const ProcessorInfo& TemporalVolumeSource::getProcessorInfo() const { return processorInfo_; }

namespace {

// Extract the last contiguous run of digits from a file stem, e.g. "frame_0042" -> 42.
std::optional<double> numberFromName(const std::filesystem::path& path) {
    const auto stem = path.stem().string();
    auto end = stem.rend();
    auto digitsEnd =
        std::find_if(stem.rbegin(), end, [](unsigned char c) { return std::isdigit(c) != 0; });
    if (digitsEnd == end) {
        return std::nullopt;
    }
    auto digitsBegin =
        std::find_if(digitsEnd, end, [](unsigned char c) { return std::isdigit(c) == 0; });
    const std::string number{digitsBegin.base(), digitsEnd.base()};
    if (number.empty()) {
        return std::nullopt;
    }
    try {
        return std::stod(number);
    } catch (...) {
        return std::nullopt;
    }
}

}  // namespace

TemporalVolumeSource::TemporalVolumeSource(InviwoApplication* app)
    : Processor()
    , app_{app}
    , outport_{"data", "The loaded temporal volume"_help}
    , folder_{"folder", "Folder", "Folder to look for volume files in"_help}
    , filter_{"filter", "Filter", "Wildcard filter applied to the folder contents"_help, "*"}
    , reader_{"reader", "Data Reader", "The reader used for loading each frame"_help,
              util::optionsForTypes<Volume>(*util::getDataReaderFactory(app))}
    , timeMode_{"timeMode",
                "Time Mode",
                "How time values are assigned to the frames"_help,
                {{"index", "Frame Index", TimeMode::Index},
                 {"fileNumber", "File Number", TimeMode::FileNumber}},
                0}
    , cacheSize_{"cacheSize", "Cache Size",
                 util::ordinalCount<size_t>(8u, 256u).set(
                     "Maximum number of decoded frames kept in memory"_help)}
    , reload_{"reload", "Reload", "Reload the data from disk"_help} {

    addPort(outport_);
    addProperties(folder_, filter_, reader_, timeMode_, cacheSize_, reload_);

    isSink_.setUpdate([]() { return true; });

    const auto markDirty = [this]() { dirty_ = true; };
    folder_.onChange(markDirty);
    filter_.onChange(markDirty);
    reader_.onChange(markDirty);
    timeMode_.onChange(markDirty);
    reload_.onChange(markDirty);
    cacheSize_.onChange(markDirty);
}

void TemporalVolumeSource::load() {
    dirty_ = false;

    if (folder_.get().empty() || !std::filesystem::is_directory(folder_.get())) {
        outport_.clear();
        return;
    }

    // Gather and sort the matching files.
    std::vector<std::filesystem::path> paths;
    for (const auto& entry : filesystem::getDirectoryContents(folder_.get())) {
        const auto file = folder_.get() / entry;
        if (filesystem::wildcardStringMatch(filter_.get(), file.generic_string())) {
            paths.push_back(file);
        }
    }
    std::ranges::sort(paths);

    if (paths.empty()) {
        outport_.clear();
        log::warn("No files matching '{}' found in {}", filter_.get(), folder_.get());
        return;
    }

    // Assign time values according to the selected mode.
    std::vector<Seconds> times;
    if (timeMode_.get() == TimeMode::FileNumber) {
        times.reserve(paths.size());
        bool allValid = true;
        for (const auto& path : paths) {
            if (auto number = numberFromName(path)) {
                times.push_back(Seconds{*number});
            } else {
                allValid = false;
                break;
            }
        }
        if (!allValid) {
            times.clear();  // fall back to frame indices
            log::warn("Could not extract a number from every file name, using frame indices");
        }
    }

    try {
        auto loader = std::make_unique<FileSequenceLoader>(std::move(paths), std::move(times),
                                                           util::getDataReaderFactory(app_),
                                                           reader_.getSelectedValue());
        outport_.setData(std::make_shared<TemporalVolume>(std::move(loader), cacheSize_.get()));
    } catch (const Exception& e) {
        outport_.clear();
        log::error("Failed to load temporal volume: {}", e.getMessage());
    }
}

void TemporalVolumeSource::process() {
    if (dirty_) {
        load();
    }
}

}  // namespace inviwo
