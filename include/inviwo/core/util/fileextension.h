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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/hashcombine.h>

#include <string>
#include <string_view>
#include <iosfwd>

namespace inviwo {

class IVW_CORE_API FileExtension : public Serializable {
public:
    FileExtension();
    FileExtension(const FileExtension&) = default;
    FileExtension(FileExtension&&) = default;
    FileExtension& operator=(FileExtension&&) = default;
    FileExtension& operator=(const FileExtension&) = default;

    FileExtension(std::string_view extension, std::string_view description);
    virtual ~FileExtension() = default;

    /**
     * \brief extracts a FileExtension object from a string. This function assumes
     * that the extension is given within the right most parentheses.
     *
     * @param str Input string  formed like "Text files (*.txt)".
     * @return FileExtension object created from the information given in the input string.
     */
    static FileExtension createFileExtensionFromString(std::string_view str);
    std::string toString() const;

    bool empty() const;
    /**
     * \brief returns true, if the current extension matches all files, i.e. it is equal to '*'.
     * Note: the description can be different from all().
     *
     * @return true if extension_ == '*'
     */
    bool matchesAll() const;

    /**
     * \brief checks whether the given string is matched by this FileExtension
     * Extensions are matched case insensitive while an empty FileExtension, i.e. '*', will match
     * all strings.
     *
     * @param str   string to be tested
     * @return true if the string is matched by the FileExtension
     */
    bool matches(std::string_view str) const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    static FileExtension all();

    std::string extension_;  ///< File extension in lower case letters.
    std::string description_;

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& ss, const FileExtension& ext);

    IVW_CORE_API friend bool operator==(const FileExtension&, const FileExtension&);
    IVW_CORE_API friend bool operator!=(const FileExtension&, const FileExtension&);

    IVW_CORE_API friend bool operator<(const FileExtension&, const FileExtension&);
    IVW_CORE_API friend bool operator<=(const FileExtension&, const FileExtension&);
    IVW_CORE_API friend bool operator>(const FileExtension&, const FileExtension&);
    IVW_CORE_API friend bool operator>=(const FileExtension&, const FileExtension&);
};

template <>
struct PropertyTraits<OptionProperty<FileExtension>> {
    static std::string classIdentifier() { return "org.inviwo.OptionPropertyFileExtension"; }
};

}  // namespace inviwo

namespace std {
template <>
struct hash<inviwo::FileExtension> {
    size_t operator()(const inviwo::FileExtension& f) const {
        size_t h = 0;
        inviwo::util::hash_combine(h, f.extension_);
        inviwo::util::hash_combine(h, f.description_);
        return h;
    }
};
}  // namespace std
