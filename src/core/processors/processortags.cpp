/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

Tags Tag::operator|(const Tag& rhs) const { return Tags{std::vector<Tag>{*this, rhs}}; }

std::ostream& operator<<(std::ostream& os, const Tag& obj) {
    os << obj.getString();
    return os;
}
Tags::Tags(const Tag& tag) : tags_{tag} {}

Tags::Tags(std::vector<Tag> tags) : tags_{std::move(tags)} {}

Tags::Tags(std::string_view tags) {
    util::forEachStringPart(tags, ",",
                            [&](std::string_view part) { addTag(Tag(util::trim(part))); });
}

Tags::Tags(const std::string& tags) : Tags{std::string_view{tags}} {}

Tags::Tags(const char* tags) : Tags{std::string_view{tags}} {}

Tags& Tags::operator=(std::string_view that) {
    tags_.clear();
    util::forEachStringPart(that, ",",
                            [&](std::string_view part) { addTag(Tag(util::trim(part))); });
    return *this;
}

Tags& Tags::addTag(const Tag& t) {
    if (!util::contains(tags_, t)) {
        tags_.push_back(t);
    }
    return *this;
}

Tags& Tags::addTags(const Tags& t) {
    for (auto& tag : t.tags_) {
        addTag(tag);
    }
    return *this;
}

size_t Tags::size() const { return tags_.size(); }

bool Tags::empty() const { return tags_.empty(); }

std::string Tags::getString() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

int Tags::getMatches(const Tags& input) const {
    int matches = 0;
    for (auto& elem : input.tags_) {
        for (auto& t : tags_) {
            if (elem == t) {
                matches++;
            }
        }
    }
    return matches;
}

std::ostream& operator<<(std::ostream& os, const Tags& obj) {
    for (std::vector<Tag>::const_iterator it = obj.tags_.begin(); it != obj.tags_.end(); ++it) {
        os << *it;
        if (std::distance(it, obj.tags_.end()) > 1) {
            os << ", ";
        }
    }
    return os;
}

namespace util {

Tags getPlatformTags(const Tags& t) {
    Tags result;
    for (auto& tag : t.tags_) {
        if (Tags::GL == tag) {
            result.addTag(tag);
        } else if (Tags::CL == tag) {
            result.addTag(tag);
        } else if (Tags::CPU == tag) {
            result.addTag(tag);
        } else if (Tags::PY == tag) {
            result.addTag(tag);
        }
    }
    return result;
}

}  // namespace util

}  // namespace inviwo
