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

#include <modules/base/io/filesequenceloader.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/foreacharg.h>

#include <utility>

#include <fmt/std.h>

namespace inviwo {

FileSequenceLoader::FileSequenceLoader(std::vector<std::filesystem::path> paths,
                                       std::vector<Seconds> times, DataReaderFactory* factory,
                                       FileExtension extension)
    : paths_{std::move(paths)}
    , times_{std::move(times)}
    , factory_{factory}
    , extension_{std::move(extension)} {

    if (!factory_) {
        throw Exception("FileSequenceLoader requires a non-null DataReaderFactory");
    }
    if (paths_.empty()) {
        throw Exception("FileSequenceLoader requires at least one file");
    }
    if (!times_.empty() && times_.size() != paths_.size()) {
        throw Exception(SourceContext{},
                        "FileSequenceLoader: number of time values ({}) does not match number of "
                        "files ({})",
                        times_.size(), paths_.size());
    }

    // Load the first frame once to obtain a metadata-only prototype.
    prototype_ = readFile(0)->config();
}

std::shared_ptr<Volume> FileSequenceLoader::readFile(size_t index, std::stop_token stop) const {
    const auto& path = paths_[index];

    // Prefetches queue up on this mutex, so bail out both before and after acquiring it.
    if (stop.stop_requested()) return nullptr;
    const std::scoped_lock lock{readerMutex_};
    if (stop.stop_requested()) return nullptr;

    auto reader = extension_.empty()
                      ? factory_->getReaderForTypeAndExtension<Volume>(path)
                      : factory_->getReaderForTypeAndExtension<Volume>(extension_, path);
    if (!reader) {
        throw DataReaderException(SourceContext{}, "Unable to find a Volume reader for file: {}",
                                  path);
    }
    auto volume = reader->readData(path, nullptr, std::move(stop));
    if (!volume) return nullptr;

    if (!volume->hasMetaData<StringMetaData>("filename")) {
        volume->setMetaData<StringMetaData>("filename", path.generic_string());
    }

    return volume;
}

namespace {

std::vector<std::string> mismatch(const VolumeConfig& a, const VolumeConfig& b) {
    std::vector<std::string> res;

    constexpr auto format = util::overloaded{
        [](const std::optional<Axis>& a, std::string& out) {
            if (a.has_value()) {
                fmt::format_to(std::back_inserter(out), "{}{: [}", a->name, a->unit);
            } else {
                out += "<none>";
            }
        },
        [](const DataFormatBase* f, std::string& out) {
            if (f) {
                out += f->getString();
            } else {
                out += "<none>";
            }
        },
        [](const auto& a, std::string& out) {
            if (a.has_value()) {
                fmt::format_to(std::back_inserter(out), "{}", *a);
            } else {
                out += "<none>";
            }
        }
    };

    util::for_each_in_tuple(
        [&](const auto& a, const auto& b) {
            if (a != b) {
                auto& str = res.emplace_back();
                format(a, str);
                str += " != ";
                format(b, str);
            }
        },
        a.tie(), b.tie());

    return res;
}

}  // namespace

std::shared_ptr<Volume> FileSequenceLoader::load(size_t index, std::shared_ptr<Volume>,
                                                 std::stop_token stop) const {
    if (index >= paths_.size()) {
        throw RangeException(SourceContext{}, "Frame index {} out of range [0, {})", index,
                             paths_.size());
    }
    // The reader always allocates a fresh volume, so the reuse hint is ignored.
    auto volume = readFile(index, std::move(stop));
    if (volume->config() != prototype_) {
        const auto mismatches = mismatch(volume->config(), prototype_);
        throw DataReaderException(SourceContext{}, "Volume config mismatch {} for file: {}",
                                  fmt::join(mismatches, ", "), paths_[index]);
    }
    return volume;
}

size_t FileSequenceLoader::size() const { return paths_.size(); }

Seconds FileSequenceLoader::time(size_t index) const {
    if (times_.empty()) {
        return static_cast<Seconds>(index);
    } else if (index < times_.size()) {
        return times_[index];
    } else {
        throw RangeException(SourceContext{}, "Frame index {} out of range [0, {})", index,
                             times_.size());
    }
}

VolumeConfig FileSequenceLoader::prototype() const { return prototype_; }

}  // namespace inviwo
