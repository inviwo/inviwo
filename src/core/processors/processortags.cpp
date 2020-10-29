/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2020 Inviwo Foundation
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

Tag::Tag(std::string_view tag) : tag_(tag) {}

Tags Tag::operator|(const Tag& rhs) const { return Tags{std::vector<Tag>{*this, rhs}}; }

const std::string& Tag::getString() const { return tag_; }

std::ostream& operator<<(std::ostream& os, const Tag& obj) {
    os << obj.tag_;
    return os;
}

const Tag Tag::GL("GL");
const Tag Tag::CL("CL");
const Tag Tag::CPU("CPU");
const Tag Tag::PY("PY");

Tags::Tags(const Tag& tag) : tags_{tag} {}

Tags::Tags(std::vector<Tag> tags) : tags_{std::move(tags)} {}

Tags::Tags(std::string_view tags) {
    util::forEachStringPart(tags, ",", [&](std::string_view part) { addTag(Tag(util::trim(part))); });
}

Tags::Tags(std::string tags) : Tags{std::string_view{tags}} {}

Tags::Tags(const char* tags) : Tags{std::string_view{tags}} {}

Tags& Tags::operator=(std::string_view that) {
    tags_.clear();
    util::forEachStringPart(that, ",", [&](std::string_view part) { addTag(Tag(util::trim(part))); });
    return *this;
}

Tags Tags::operator|(const Tag& rhs) const { return Tags{*this}.addTag(rhs); }

Tags Tags::operator|(const Tags& rhs) const { return Tags{*this}.addTags(rhs); }

Tags& Tags::addTag(Tag t) {
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

const Tags Tags::None{};
const Tags Tags::GL{Tag::GL};
const Tags Tags::CL{Tag::CL};
const Tags Tags::CPU{Tag::CPU};
const Tags Tags::PY{Tag::PY};

namespace util {

Tags getPlatformTags(const Tags& t) {
    Tags result;
    for (auto& tag : t.tags_) {
        if (util::contains(Tags::GL.tags_, tag)) {
            result.addTag(tag);
        }
        if (util::contains(Tags::CL.tags_, tag)) {
            result.addTag(tag);
        }
        if (util::contains(Tags::CPU.tags_, tag)) {
            result.addTag(tag);
        }
        if (util::contains(Tags::PY.tags_, tag)) {
            result.addTag(tag);
        }
    }
    return result;
}

}  // namespace util

}  // namespace inviwo
