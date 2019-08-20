/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/filedialogstate.h>

#include <algorithm>
#include <iomanip>
#include <iterator>

namespace inviwo {

const std::string FilePatternProperty::classIdentifier = "org.inviwo.FilePatternProperty";
std::string FilePatternProperty::getClassIdentifier() const { return classIdentifier; }

FilePatternProperty::FilePatternProperty(std::string identifier, std::string displayName,
                                         std::string pattern, std::string directory,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , helpText_("helpText", "",
                "A pattern might include '#' as placeholder for digits, where "
                "multiple '###' indicate leading zeros. Wildcards('*', '?') are supported.")
    , pattern_("pattern", "Pattern", {pattern})
    , updateBtn_("updateBtn", "Update File List")
    , sort_("sorting", "Sort File Names", true)
    , matchShorterNumbers_("matchShorterNumbers", "Match Numbers with less Digits", true)
    , rangeSelection_("rangeSelection", "Range Selection", false)
    , minIndex_("minIndex", "Minimum Index", 0, -1, std::numeric_limits<int>::max())
    , maxIndex_("maxIndex", "Maximum Index", 100, -1, std::numeric_limits<int>::max()) {

    helpText_.setReadOnly(true);
    helpText_.setSemantics(PropertySemantics::Multiline);

    addProperty(pattern_);
    addProperty(helpText_);
    addProperty(sort_);
    addProperty(matchShorterNumbers_);

    addProperty(rangeSelection_);
    rangeSelection_.addProperty(minIndex_);
    rangeSelection_.addProperty(maxIndex_);

    pattern_.setAcceptMode(AcceptMode::Open);
    pattern_.setFileMode(FileMode::ExistingFiles);

    minIndex_.setSemantics(PropertySemantics::Text);
    maxIndex_.setSemantics(PropertySemantics::Text);
    minIndex_.onChange([this]() {
        if (rangeSelection_.isChecked()) {
            updateFileList();
        }
    });
    maxIndex_.onChange([this]() {
        if (rangeSelection_.isChecked()) {
            updateFileList();
        }
    });

    minIndex_.setCurrentStateAsDefault();
    maxIndex_.setCurrentStateAsDefault();
    rangeSelection_.setCollapsed(true);
    rangeSelection_.setCurrentStateAsDefault();

    auto update = [this]() { updateFileList(); };
    pattern_.onChange(update);
    rangeSelection_.onChange(update);

    sort_.onChange([this]() { sort(); });
    matchShorterNumbers_.onChange([this]() { updateFileList(); });
    if (!pattern_.get().empty()) {
        updateFileList();
    }
}

FilePatternProperty::FilePatternProperty(const FilePatternProperty& rhs)
    : CompositeProperty(rhs)
    , helpText_{rhs.helpText_}
    , pattern_{rhs.pattern_}
    , updateBtn_{rhs.updateBtn_}
    , sort_{rhs.sort_}
    , matchShorterNumbers_{rhs.matchShorterNumbers_}
    , rangeSelection_{rhs.rangeSelection_}
    , minIndex_{rhs.minIndex_}
    , maxIndex_{rhs.maxIndex_} {

    addProperty(pattern_);
    addProperty(helpText_);
    addProperty(sort_);
    addProperty(matchShorterNumbers_);

    addProperty(rangeSelection_);
    rangeSelection_.addProperty(minIndex_);
    rangeSelection_.addProperty(maxIndex_);

    minIndex_.onChange([this]() {
        if (rangeSelection_.isChecked()) {
            updateFileList();
        }
    });
    maxIndex_.onChange([this]() {
        if (rangeSelection_.isChecked()) {
            updateFileList();
        }
    });
    auto update = [this]() { updateFileList(); };
    pattern_.onChange(update);
    rangeSelection_.onChange(update);
    sort_.onChange([this]() { sort(); });
    matchShorterNumbers_.onChange([this]() { updateFileList(); });
    if (!pattern_.get().empty()) {
        updateFileList();
    }
}

FilePatternProperty* FilePatternProperty::clone() const { return new FilePatternProperty(*this); }

FilePatternProperty::~FilePatternProperty() = default;

std::string FilePatternProperty::getFilePattern() const {
    if (!pattern_.get().empty()) {
        return filesystem::getFileNameWithExtension(pattern_.get().front());
    } else {
        return std::string();
    }
}

std::string FilePatternProperty::getFilePatternPath() const {
    if (!pattern_.get().empty()) {
        return filesystem::getFileDirectory(pattern_.get().front());
    } else {
        return std::string();
    }
}

std::vector<std::string> FilePatternProperty::getFileList() const {
    std::vector<std::string> fileList;
    std::transform(files_.begin(), files_.end(), std::back_inserter(fileList),
                   [](IndexFileTuple elem) { return std::get<1>(elem); });
    return fileList;
}

std::vector<int> FilePatternProperty::getFileIndices() const {
    std::vector<int> indexList;
    std::transform(files_.begin(), files_.end(), std::back_inserter(indexList),
                   [](IndexFileTuple elem) { return std::get<0>(elem); });
    return indexList;
}

bool FilePatternProperty::hasOutOfRangeMatches() const { return outOfRangeMatches_; }

bool FilePatternProperty::hasRangeSelection() const { return rangeSelection_.isChecked(); }

int FilePatternProperty::getMinRange() const { return minIndex_.get(); }

int FilePatternProperty::getMaxRange() const { return maxIndex_.get(); }

const FileExtension& FilePatternProperty::getSelectedExtension() const {
    return pattern_.getSelectedExtension();
}

void FilePatternProperty::setSelectedExtension(const FileExtension& ext) {
    pattern_.setSelectedExtension(ext);
}

void FilePatternProperty::updateFileList() {
    files_.clear();
    outOfRangeMatches_ = false;

    if (pattern_.get().empty()) {
        return;
    }

    for (auto item : pattern_.get()) {
        try {
            const std::string filePath = filesystem::getFileDirectory(item);
            const std::string pattern = filesystem::getFileNameWithExtension(item);

            std::vector<std::string> fileList = filesystem::getDirectoryContents(filePath);

            // apply pattern
            bool hasDigits = (pattern.find('#') != std::string::npos);
            bool hasWildcard = hasDigits || (pattern.find_first_of("*?", 0) != std::string::npos);

            if (!hasWildcard) {
                // look for exact match
                if (util::contains(fileList, pattern)) {
                    files_.push_back(std::make_tuple(-1, item));
                }
            } else if (hasDigits) {
                ivec2 indexRange{-1, std::numeric_limits<int>::max()};
                if (rangeSelection_.isChecked()) {
                    indexRange = ivec2(minIndex_.get(), maxIndex_.get());
                }

                const bool matchShorterNumbers = matchShorterNumbers_.get();
                const bool matchLongerNumbers = true;
                bool found = false;
                for (auto file : fileList) {
                    int index = -1;
                    if (filesystem::wildcardStringMatchDigits(
                            pattern, file, index, matchShorterNumbers, matchLongerNumbers)) {
                        // match found
                        found = true;
                        // check index
                        if ((index >= indexRange.x) && (index <= indexRange.y)) {
                            std::string filename = filePath + '/' + file;
                            files_.push_back(std::make_tuple(index, filename));
                        }
                    }
                }
                outOfRangeMatches_ = (found && files_.empty());
            } else {
                // apply range selection, assume file names are sorted
                if (rangeSelection_.isChecked()) {
                    ivec2 indexRange(minIndex_.get(), maxIndex_.get());
                    if (indexRange.y < static_cast<int>(fileList.size())) {
                        // remove all files after the maximum index
                        fileList.erase(
                            fileList.begin() + std::max(static_cast<std::size_t>(indexRange.y + 1),
                                                        fileList.size()),
                            fileList.end());
                        // remove file names at the begin
                        fileList.erase(
                            fileList.begin(),
                            fileList.begin() + static_cast<std::size_t>(indexRange.y + 1));
                    }
                }
                for (auto file : fileList) {
                    if (filesystem::wildcardStringMatch(pattern, file)) {
                        // match found
                        std::string filename = filePath + '/' + file;
                        files_.push_back(std::make_tuple(-1, filename));
                    }
                }
            }
        } catch (FileException& e) {
            LogError("Error (file exception): " << e.what());
        }
    }

    // sort file names
    sort();

    // LogInfo("Files matching the pattern:" << getFormattedFileList());
}

std::string FilePatternProperty::getFormattedFileList() const {
    std::ostringstream oss;
    for (auto elem : files_) {
        oss << std::setw(6) << std::get<0>(elem) << ": " << std::get<1>(elem) << "\n";
    }
    return oss.str();
}

void FilePatternProperty::sort() {
    if (!sort_.get()) return;

    std::sort(files_.begin(), files_.end(), [](IndexFileTuple a, IndexFileTuple b) {
        // do a lexical comparison if indices are equal
        // TODO: consider . in file names as the lowest character to ensure that
        //    abc.png is listed before abc-01.png
        return (std::get<0>(a) < std::get<0>(b)
                    ? true
                    : (std::get<0>(a) > std::get<0>(b) ? false : std::get<1>(a) < std::get<1>(b)));
    });
}

std::string FilePatternProperty::guessFilePattern() const {
    LogError("not implemented yet");
    return "";
}

void FilePatternProperty::clearNameFilters() { pattern_.clearNameFilters(); }

void FilePatternProperty::addNameFilter(std::string filter) { pattern_.addNameFilter(filter); }

void FilePatternProperty::addNameFilter(FileExtension filter) { pattern_.addNameFilter(filter); }

void FilePatternProperty::addNameFilters(const std::vector<FileExtension>& filters) {
    pattern_.addNameFilters(filters);
}

}  // namespace inviwo
