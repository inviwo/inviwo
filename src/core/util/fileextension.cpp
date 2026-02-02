/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/util/fileextension.h>

#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>

#include <ostream>
#include <filesystem>

namespace inviwo {

FileExtension FileExtension::createFileExtensionFromString(std::string_view str) {
    // try to split extension string
    std::size_t extStart = str.find('(');
    if (extStart == std::string_view::npos) {
        // could not find extension inside ()
        return {};
    }

    std::size_t extEnd = str.rfind(')');
    if (extEnd == std::string_view::npos) {
        // not matching ')' for extension string
        return {};
    }
    std::string_view desc{str.substr(0, extStart)};
    // trim trailing white spaces
    std::size_t whiteSpacePos = desc.find_last_not_of(" \t\n\r(");
    if (whiteSpacePos != std::string_view::npos) {
        desc = desc.substr(0, whiteSpacePos + 1);
    } else {
        // description only consisted of whitespace characters
        desc = std::string_view{};
    }

    std::string_view ext = str.substr(extStart + 1, extEnd - extStart - 1);
    // '*.*' should not be used as it is not platform-independent
    // ('*' should be used instead of '*.*')

    // special case '*', i.e. '*.*', this should result in an empty string
    if ((ext == "*") || (ext == "*.*")) {
        ext = std::string_view{};
    }
    // get rid of '*.'
    if (ext.compare(0, 2, "*.") == 0) {
        ext = ext.substr(2);
    }

    return {.extension = LCString{ext}, .description = std::string{desc}};
}

std::string FileExtension::toString() const { return fmt::to_string(*this); }

bool FileExtension::empty() const { return extension.empty() && description.empty(); }

bool FileExtension::matchesAll() const { return extension == "*"; }

bool FileExtension::matches(const std::filesystem::path& path) const {
    if (empty()) {
        return false;
    }
    if (extension.empty()) {
        return true;  // wildcard '*' matches everything
    }
    return util::iCaseEndsWith(path.string(), extension);
}

void FileExtension::serialize(Serializer& s) const {
    s.serialize("extension", extension.view());
    s.serialize("description", description);
}

void FileExtension::deserialize(Deserializer& d) {
    d.deserialize("extension", extension);
    d.deserialize("description", description);
}

std::strong_ordering FileExtension::operator<=>(const FileExtension& that) const noexcept {
    if (*this == FileExtension::all() && that != FileExtension::all()) {
        return std::strong_ordering::less;
    }
    if (that == FileExtension::all() && *this != FileExtension::all()) {
        return std::strong_ordering::greater;
    }
    return std::tie(description, extension) <=> std::tie(that.description, that.extension);
}

FileExtension FileExtension::all() { return {.extension = "*", .description = "All Files"}; }

}  // namespace inviwo
