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

#ifndef IVW_PROCESSORTAGS_H
#define IVW_PROCESSORTAGS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class IVW_CORE_API Tag {
public:
    Tag() = default;
    Tag(std::string tag);
    Tag& operator=(const std::string& that);
    const std::string& getString() const;

    friend std::ostream& operator<<(std::ostream& os, const inviwo::Tag& obj);

    // pre-defined platform tags
    static const Tag GL;
    static const Tag CL;
    static const Tag CPU;
    static const Tag PY;

private:
    std::string tag_;
};

inline bool operator==(const Tag& lhs, const Tag& rhs) {
    return lhs.getString() == rhs.getString();
}
inline bool operator<(const Tag& lhs, const Tag& rhs) { return lhs.getString() < rhs.getString(); }
inline bool operator!=(const Tag& lhs, const Tag& rhs) { return !operator==(lhs, rhs); }
inline bool operator>(const Tag& lhs, const Tag& rhs) { return operator<(rhs, lhs); }
inline bool operator<=(const Tag& lhs, const Tag& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const Tag& lhs, const Tag& rhs) { return !operator<(lhs, rhs); }

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
    Tags(const std::string& tags);

    /*
     * Creates tags from a string. Multiple tags are delimited by ','.
     */
    Tags(const char* tags);

    /*
     * Assign tags from a string. Multiple tags are delimited by ','.
     */
    Tags& operator=(const std::string& that);

    void addTag(Tag);
    void addTags(const Tags& t);

    size_t size() const;
    bool empty() const;

    std::string getString() const;

    int getMatches(const Tags&) const;

    friend std::ostream& operator<<(std::ostream& os, const inviwo::Tags& obj);
    friend bool operator==(const Tags& lhs, const Tags& rhs);
    friend bool operator<(const Tags& lhs, const Tags& rhs);

    std::vector<Tag> tags_;

    // pre-defined platform tags
    static const Tags None;
    static const Tags GL;
    static const Tags CL;
    static const Tags CPU;
    static const Tags PY;
};

inline bool operator==(const Tags& lhs, const Tags& rhs) { return lhs.tags_ == rhs.tags_; }
inline bool operator<(const Tags& lhs, const Tags& rhs) { return lhs.tags_ < rhs.tags_; }
inline bool operator!=(const Tags& lhs, const Tags& rhs) { return !operator==(lhs, rhs); }
inline bool operator>(const Tags& lhs, const Tags& rhs) { return operator<(rhs, lhs); }
inline bool operator<=(const Tags& lhs, const Tags& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const Tags& lhs, const Tags& rhs) { return !operator<(lhs, rhs); }

namespace util {

Tags IVW_CORE_API getPlatformTags(const Tags& t);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_PROCESSORTAGS_H
