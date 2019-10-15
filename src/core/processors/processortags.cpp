/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

Tag::Tag(std::string tag) : tag_(std::move(tag)) {}

Tag& Tag::operator=(const std::string& that) {
    tag_ = that;
    return *this;
}

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

Tags::Tags(const std::string& tags) {
    std::vector<std::string> strings = splitString(tags, ',');
    for (auto& strings_it : strings) {
        addTag(Tag(trim(strings_it)));
    }
}

Tags::Tags(const char* tags) : Tags{std::string{tags}} {}

Tags& Tags::operator=(const std::string& that) {
    tags_.clear();
    std::vector<std::string> strings = splitString(that, ',');
    for (auto& strings_it : strings) {
        addTag(Tag(trim(strings_it)));
    }
    return *this;
}

void Tags::addTag(Tag t) {
    if (!util::contains(tags_, t)) {
        tags_.push_back(t);
    }
}

void Tags::addTags(const Tags& t) {
    for (auto& tag : t.tags_) {
        addTag(tag);
    }
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
