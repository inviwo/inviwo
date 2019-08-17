/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <vector>
#include <variant>
#include <filesystem>
#include <string_view>
#include <optional>

#include <inviwo/meta/inviwometadefine.hpp>
#include <inviwo/meta/iter/range.hpp>
#include <inviwo/meta/iter/filteriterator.hpp>
#include <inviwo/meta/iter/transformiterator.hpp>

namespace inviwo::meta::cmake {

struct INVIWO_META_API BeginBrace {};
struct INVIWO_META_API EndBrace {};
struct INVIWO_META_API LineEnding {};
struct INVIWO_META_API Space {
    std::string value;
};
struct INVIWO_META_API Argument {
    std::string value;
};
struct INVIWO_META_API Comment {
    std::string value;
};

using ArgElement = std::variant<Argument, Space, LineEnding, Comment, BeginBrace, EndBrace>;

struct INVIWO_META_API Command {
    Space space1 = {""};
    std::string identifier;
    Space space2 = {""};
    std::vector<ArgElement> arguments;

    std::ostream& print(std::ostream& os) const;

    using filter_t = bool (*)(const ArgElement&);
    using transform_t = Argument& (*)(ArgElement&);
    using const_transform_t = const Argument& (*)(const ArgElement&);
    using iterator =
        TransformIterator<transform_t, FilterIterator<filter_t, std::vector<ArgElement>::iterator>>;
    using const_iterator =
        TransformIterator<const_transform_t,
                          FilterIterator<filter_t, std::vector<ArgElement>::const_iterator>>;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    Range<iterator> args();
    Range<const_iterator> args() const;

private:
    static inline constexpr filter_t filter = [](const ArgElement& elem) -> bool {
        return std::holds_alternative<Argument>(elem);
    };
    static inline constexpr transform_t transform = [](ArgElement& elem) -> Argument& {
        return std::get<Argument>(elem);
    };
    static inline constexpr const_transform_t const_transform =
        [](const ArgElement& elem) -> const Argument& { return std::get<Argument>(elem); };
};

using FileElement = std::variant<Command, Comment, Space, LineEnding>;

class INVIWO_META_API CMakeFile {
public:
    CMakeFile() = default;
    CMakeFile(const std::filesystem::path& path);
    CMakeFile(std::string_view string);
    std::ostream& print(std::ostream& os) const;

    using filter_t = bool (*)(const FileElement&);
    using transform_t = Command& (*)(FileElement&);
    using const_transform_t = const Command& (*)(const FileElement&);
    using iterator =
        TransformIterator<transform_t,
                          FilterIterator<filter_t, std::vector<FileElement>::iterator>>;
    using const_iterator =
        TransformIterator<const_transform_t,
                          FilterIterator<filter_t, std::vector<FileElement>::const_iterator>>;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    Range<iterator> commands();
    Range<const_iterator> commands() const;

    std::vector<FileElement> items;
private:
    static inline constexpr filter_t filter = [](const FileElement& elem) -> bool {
        return std::holds_alternative<Command>(elem);
    };
    static inline constexpr transform_t transform = [](FileElement& elem) -> Command& {
        return std::get<Command>(elem);
    };
    static inline constexpr const_transform_t const_transform =
        [](const FileElement& elem) -> const Command& { return std::get<Command>(elem); };
};

}  // namespace inviwo::meta::cmake
