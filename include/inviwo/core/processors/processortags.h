/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <string>
#include <vector>

namespace inviwo {

class Tags;

class IVW_CORE_API Tag {
public:
    Tag() = default;
    Tag(std::string_view tag);
    const std::string& getString() const;

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& os, const inviwo::Tag& obj);
    Tags operator|(const Tag& rhs) const;

    friend inline bool operator==(const Tag& lhs, const Tag& rhs) {
        return lhs.getString() == rhs.getString();
    }
    friend inline bool operator<(const Tag& lhs, const Tag& rhs) {
        return lhs.getString() < rhs.getString();
    }
    friend inline bool operator!=(const Tag& lhs, const Tag& rhs) { return !operator==(lhs, rhs); }
    friend inline bool operator>(const Tag& lhs, const Tag& rhs) { return operator<(rhs, lhs); }
    friend inline bool operator<=(const Tag& lhs, const Tag& rhs) { return !operator>(lhs, rhs); }
    friend inline bool operator>=(const Tag& lhs, const Tag& rhs) { return !operator<(lhs, rhs); }

    // pre-defined platform tags
    static const Tag GL;
    static const Tag CL;
    static const Tag CPU;
    static const Tag PY;

private:
    std::string tag_;
};

class IVW_CORE_API Tags {
public:
    Tags() = default;

    /*
     * Creates tags from a tag.
     */
    Tags(const Tag& tag);

    /*
     * Creates tags from a vector of tags.
     */
    Tags(std::vector<Tag> tags);

    /*
     * Creates tags from a string. Multiple tags are delimited by ','.
     */
    Tags(std::string_view tags);

    /*
     * Creates tags from a string. Multiple tags are delimited by ','.
     */
    Tags(std::string tags);

    /*
     * Creates tags from a string. Multiple tags are delimited by ','.
     */
    Tags(const char* tags);

    /*
     * Assign tags from a string. Multiple tags are delimited by ','.
     */
    Tags& operator=(std::string_view that);

    Tags& addTag(Tag);
    Tags& addTags(const Tags& t);

    size_t size() const;
    bool empty() const;

    std::string getString() const;

    int getMatches(const Tags&) const;

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& os, const inviwo::Tags& obj);

    std::vector<Tag> tags_;

    // pre-defined platform tags
    static const Tags None;
    static const Tags GL;
    static const Tags CL;
    static const Tags CPU;
    static const Tags PY;

    friend inline bool operator==(const Tags& lhs, const Tags& rhs) {
        return lhs.tags_ == rhs.tags_;
    }
    friend inline bool operator<(const Tags& lhs, const Tags& rhs) { return lhs.tags_ < rhs.tags_; }
    friend inline bool operator!=(const Tags& lhs, const Tags& rhs) {
        return !operator==(lhs, rhs);
    }
    friend inline bool operator>(const Tags& lhs, const Tags& rhs) { return operator<(rhs, lhs); }
    friend inline bool operator<=(const Tags& lhs, const Tags& rhs) { return !operator>(lhs, rhs); }
    friend inline bool operator>=(const Tags& lhs, const Tags& rhs) { return !operator<(lhs, rhs); }

    Tags operator|(const Tag& rhs) const;
    Tags operator|(const Tags& rhs) const;
};

namespace util {

Tags IVW_CORE_API getPlatformTags(const Tags& t);

}  // namespace util

}  // namespace inviwo
