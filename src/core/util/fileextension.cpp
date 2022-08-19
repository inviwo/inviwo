/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>

#include <ostream>

namespace inviwo {

FileExtension::FileExtension() : extension_(), description_() {}

FileExtension::FileExtension(std::string_view extension, std::string_view description)
    : extension_(
          toLower(std::string{extension}))  // Make sure that the extension is given in lower case
    , description_(description) {}

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

    return {ext, desc};
}

std::string FileExtension::toString() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

bool FileExtension::empty() const { return extension_.empty() && description_.empty(); }

bool FileExtension::matchesAll() const { return extension_ == "*"; }

bool FileExtension::matches(std::string_view str) const {
    if (empty()) {
        return false;
    }
    if (extension_.empty()) {
        return true;  // wildcard '*' matches everything
    }
    return util::iCaseEndsWith(str, extension_);
}

void FileExtension::serialize(Serializer& s) const {
    s.serialize("extension", extension_);
    s.serialize("description", description_);
}

void FileExtension::deserialize(Deserializer& d) {
    d.deserialize("extension", extension_);
    d.deserialize("description", description_);
}

bool operator==(const FileExtension& rhs, const FileExtension& lhs) {
    return rhs.extension_ == lhs.extension_ && rhs.description_ == lhs.description_;
}
bool operator!=(const FileExtension& rhs, const FileExtension& lhs) { return !(rhs == lhs); }

bool operator<(const FileExtension& a, const FileExtension& b) {
    if (a == FileExtension::all() && b != FileExtension::all()) return true;
    if (b == FileExtension::all() && a != FileExtension::all()) return false;
    return std::tie(a.description_, a.extension_) < std::tie(b.description_, b.extension_);
}

bool operator>(const FileExtension& lhs, const FileExtension& rhs) { return operator<(rhs, lhs); }
bool operator<=(const FileExtension& lhs, const FileExtension& rhs) { return !operator>(lhs, rhs); }
bool operator>=(const FileExtension& lhs, const FileExtension& rhs) { return !operator<(lhs, rhs); }

FileExtension FileExtension::all() { return FileExtension("*", "All Files"); }

std::ostream& operator<<(std::ostream& ss, const FileExtension& ext) {
    if (ext.extension_.empty()) {
        return ss;
    }
    ss << ext.description_ << " ";
    if (ext.extension_ == "*") {
        ss << "(*)";
    } else {
        ss << "(*." << ext.extension_ << ")";
    }
    return ss;
}

}  // namespace inviwo
