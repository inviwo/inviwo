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
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>

#include <filesystem>
#include <ranges>
#include <vector>
#include <regex>
#include <chrono>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/chrono.h>

template <>
struct fmt::formatter<std::filesystem::file_time_type, char> {
    fmt::formatter<std::chrono::sys_time<std::filesystem::file_time_type::duration>> formatter;

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return formatter.parse(ctx);
    }

    template <class FmtContext>
    constexpr FmtContext::iterator format(std::filesystem::file_time_type time,
                                          FmtContext& ctx) const {
#ifdef WIN32
        const auto systime = std::chrono::clock_cast<std::chrono::system_clock>(time);
#else
        const auto systime = std::filesystem::file_time_type::clock::to_sys(time);
#endif
        return formatter.format(systime, ctx);
    }
};

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
    , bnlInport_{"bnlInport"}
    , bnlOutport_{"bnlOutport"}
    , directory_{"directory", "Directory"}
    , refresh_{"refresh", "Refresh"}
    , filter_{"filter", "Filter",
              "Set a regexp filter to match filenames to include in the list"_help}
    , selectedIndex_{"selectedIndex", "Selected Index", util::ordinalCount(size_t{0}, size_t{100})}
    , highlightIndex_{"highlightIndex", "Highlight Index",
                      util::ordinalCount(size_t{0}, size_t{100})}
    , selected_{"selected", "Selected"}
    , highlight_{"highlight", "Highlight"}
    , cycleFiles_{"cycleFiles", "Cycle through all files",
                  "When enabled, the processor will iterate over the list of files where the "
                  "network is evaluated once for each file before continuing."_help,
                  [this]() { cycleFiles(); }}
    , running_{false} {

    addPorts(outport_, bnlInport_, bnlOutport_);
    addProperties(directory_, refresh_, filter_, selectedIndex_, selected_, highlightIndex_,
                  highlight_, cycleFiles_);

    selected_.setReadOnly(true);
    highlight_.setReadOnly(true);

    bnlOutport_.getManager().setParent(&bnlInport_.getManager());
    bnlOutport_.getManager().onBrush([this](BrushingAction action, const BrushingTarget& target,
                                            const BitSet& indices,
                                            [[maybe_unused]] std::string_view source) {
        if (action == BrushingAction::Select && target == BrushingTarget::Row) {
            if (files_.empty() || indices.empty()) {
                selected_.set("");
            } else {
                const Property::OnChangeBlocker block{selectedIndex_};
                selectedIndex_.set(indices.min());
                selected_.set(files_[std::min(files_.size() - 1, size_t{indices.min()})].path());
            }
        } else if (action == BrushingAction::Highlight && target == BrushingTarget::Row) {
            if (files_.empty() || indices.empty()) {
                highlight_.set("");
            } else {
                const Property::OnChangeBlocker block{highlightIndex_};
                highlightIndex_.set(indices.min());
                highlight_.set(files_[std::min(files_.size() - 1, size_t{indices.min()})].path());
            }
        }
    });

    selectedIndex_.onChange([this]() {
        bnlOutport_.getManager().brush(BrushingAction::Select, BrushingTarget::Row,
                                       {selectedIndex_.get()});
    });
    highlightIndex_.onChange([this]() {
        bnlOutport_.getManager().brush(BrushingAction::Highlight, BrushingTarget::Row,
                                       {highlightIndex_.get()});
    });
}

FileList::~FileList() { running_ = false; }

void FileList::cycleFiles() {
    auto* net = getNetwork();
    auto* app = net->getApplication();

    bool expected = false;
    const bool desired = true;
    if (running_.compare_exchange_strong(expected, desired)) {
        app->dispatchFrontAndForget([pw = weak_from_this(), app, net]() {
            const auto getSelf = [&]() -> std::shared_ptr<FileList> {
                if (auto p = pw.lock()) {
                    return std::dynamic_pointer_cast<FileList>(p);
                }
                return nullptr;
            };

            auto running = [&]() -> bool {
                if (auto self = getSelf()) return self->running_;
                return false;
            };

            const util::OnScopeExit stopRunning{[&]() {
                if (auto self = getSelf()) self->running_ = false;
            }};

            const auto files = [&]() -> std::vector<std::filesystem::directory_entry> {
                if (auto self = getSelf()) {
                    return self->files_;
                } else {
                    return {};
                }
            }();

            for (size_t i = 0; running() && i < files.size(); ++i) {
                if (auto self = getSelf()) {
                    log::info("Loading: {} {:?g}", i, files[i].path());
                    self->selectedIndex_.set(i);
                }

                do {  // NOLINT
                    app->processFront();
                    app->processEvents();
                } while (running() && net->runningBackgroundJobs() > 0);
            }
        });
    } else {
        running_ = false;
    }
}

void FileList::process() {
    std::optional<std::regex> regex;
    if (!filter_.get().empty()) {
        regex.emplace(filter_.get());
    }

    files_ = std::filesystem::directory_iterator(directory_.get()) |
             std::views::filter([](auto& item) { return item.is_regular_file(); }) |
             std::views::filter([&](auto& item) {
                 return regex ? std::regex_search(item.path().generic_string(), *regex) : true;
             }) |
             std::ranges::to<std::vector>();
    std::ranges::sort(files_);

    auto df = std::make_shared<DataFrame>();

    df->addCategoricalColumn("Name", files_ | std::views::transform([](auto& item) {
                                         return item.path().filename().generic_string();
                                     }) | std::ranges::to<std::vector>());
    df->addColumn("Size", files_ | std::views::transform([](auto& item) {
                              return static_cast<int>(item.file_size());
                          }) | std::ranges::to<std::vector>());

    df->addCategoricalColumn("Modified", files_ | std::views::transform([](auto& item) {
                                             return fmt::format("{}", item.last_write_time());
                                         }) | std::ranges::to<std::vector>());

    df->updateIndexBuffer();
    outport_.setData(df);
}

}  // namespace inviwo
