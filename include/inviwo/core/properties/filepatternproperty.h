/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/util/exception.h>

#include <string>
#include <tuple>
#include <vector>

namespace inviwo {

/** class FilePatternProperty
 *  A property class for handling file lists matching a pattern.
 *  The pattern might include '#' as placeholder for digits, where multiple '###' indicate
 *  leading zeros.
 *
 *  Wildcards ('*', '?') are supported.
 *
 * @see FileProperty, DirectoryProperty, StringProperty
 */

class IVW_CORE_API FilePatternProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    FilePatternProperty(std::string_view identifier, std::string_view displayName,
                        const std::filesystem::path& pattern = "",
                        std::string_view contentType = "default",
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                        PropertySemantics semantics = PropertySemantics::Default);

    FilePatternProperty(const FilePatternProperty& rhs);

    virtual FilePatternProperty* clone() const override;
    virtual ~FilePatternProperty();

    std::string getFilePattern() const;
    std::filesystem::path getFilePatternPath() const;

    std::vector<std::filesystem::path> getFileList() const;

    std::string getFormattedFileList() const;

    // return the indices extracted from the file names, -1 for no number found
    std::vector<int> getFileIndices() const;

    bool hasOutOfRangeMatches() const;

    bool hasRangeSelection() const;
    int getMinRange() const;
    int getMaxRange() const;

    const FileExtension& getSelectedExtension() const;
    void setSelectedExtension(const FileExtension& ext);

    void clearNameFilters();
    void addNameFilter(std::string filter);
    void addNameFilter(FileExtension filter);
    void addNameFilters(const std::vector<FileExtension>& filters);

protected:
    void updateFileList();
    void sort();
    std::string guessFilePattern() const;

private:
    StringProperty helpText_;
    MultiFileProperty pattern_;
    ButtonProperty updateBtn_;
    BoolProperty sort_;
    BoolProperty matchShorterNumbers_;

    BoolCompositeProperty rangeSelection_;
    IntProperty minIndex_;
    IntProperty maxIndex_;

    // contains file names and the extracted numbers/indices
    std::vector<std::tuple<int, std::filesystem::path>> files_;
    // flag is true if all matching files are outside the selected range
    bool outOfRangeMatches_ = false;
};

}  // namespace inviwo
