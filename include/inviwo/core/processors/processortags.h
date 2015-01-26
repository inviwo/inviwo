/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
    Tag();
    Tag(std::string tag);
    Tag(const Tag& rhs);
    Tag& operator=(const Tag& that);
    Tag& operator=(const std::string& that);

    virtual ~Tag() {}

    const std::string& getString() const;

    friend std::ostream& operator << (std::ostream& os, const inviwo::Tag& obj);

private:
    std::string tag_;
};

inline bool operator==(const Tag& lhs, const Tag& rhs) {
    return lhs.getString() == rhs.getString();
}
inline bool operator< (const Tag& lhs, const Tag& rhs) {
    return lhs.getString() < rhs.getString();
}
inline bool operator!=(const Tag& lhs, const Tag& rhs) { return !operator==(lhs, rhs); }
inline bool operator> (const Tag& lhs, const Tag& rhs) { return  operator< (rhs, lhs); }
inline bool operator<=(const Tag& lhs, const Tag& rhs) { return !operator> (lhs, rhs); }
inline bool operator>=(const Tag& lhs, const Tag& rhs) { return !operator< (lhs, rhs); }



class IVW_CORE_API Tags { 
public:
    Tags();
    Tags(const std::string tags);
    Tags(const char* tags);
    Tags(const Tags& rhs);
    Tags& operator=(const Tags& that);
    Tags& operator=(const std::string& that);

    virtual ~Tags(){}

    void addTag(Tag);

    std::string getString() const;

    int getMatches(const Tags&) const; 

    friend std::ostream& operator << (std::ostream& os, const inviwo::Tags& obj);
    friend bool operator==(const Tags& lhs, const Tags& rhs);
    friend bool operator< (const Tags& lhs, const Tags& rhs);

    std::vector<Tag> tags_;

    static const Tags None;
    static const Tags GL;
    static const Tags CL;
    static const Tags CPU;
};

inline bool operator==(const Tags& lhs, const Tags& rhs) {
    return lhs.tags_ == rhs.tags_;
}
inline bool operator< (const Tags& lhs, const Tags& rhs) {
    return lhs.tags_ < rhs.tags_;
}
inline bool operator!=(const Tags& lhs, const Tags& rhs) { return !operator==(lhs, rhs); }
inline bool operator> (const Tags& lhs, const Tags& rhs) { return  operator< (rhs, lhs); }
inline bool operator<=(const Tags& lhs, const Tags& rhs) { return !operator> (lhs, rhs); }
inline bool operator>=(const Tags& lhs, const Tags& rhs) { return !operator< (lhs, rhs); }


} // namespace

#endif // IVW_PROCESSORTAGS_H

