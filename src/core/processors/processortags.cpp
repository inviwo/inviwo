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

#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

Tag::Tag() : tag_("") {}

Tag::Tag(std::string tag) : tag_(tag) {}

Tag::Tag(const Tag& rhs) : tag_(rhs.tag_) {}
    
Tag& Tag::operator=(const Tag& that) {
    if (this != &that) tag_ = that.tag_;
    return *this;
}

Tag& Tag::operator=(const std::string& that) {
    tag_ = that;
    return *this;
}

const std::string& Tag::getString() const {
    return tag_;
}

std::ostream& operator<<(std::ostream& os, const Tag& obj) {
    os << obj.tag_;
    return os;
}

Tags::Tags() {}

Tags::Tags(const std::string tags) {
    std::vector<std::string> strings = splitString(tags, ',');
    for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it) {
        addTag(Tag(trim(*it)));
    }
}

Tags::Tags(const char* chartags) {
    const std::string tags(chartags);
    std::vector<std::string> strings = splitString(tags, ',');
    for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it) {
        addTag(Tag(trim(*it)));
    }
}

Tags::Tags(const Tags& rhs) {
    tags_ = rhs.tags_;
}

Tags& Tags::operator=(const Tags& that) {
    if (this != &that) tags_ = that.tags_;

    return *this;
}

Tags& Tags::operator=(const std::string& that) {
    tags_.clear();
    std::vector<std::string> strings = splitString(that, ',');
    for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it) {
        addTag(Tag(trim(*it)));
    }
    return *this;
}

void Tags::addTag(Tag t){
    if (std::find(tags_.begin(), tags_.end(), t) == tags_.end()) {
        tags_.push_back(t);
    }
}

std::string Tags::getString() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

int Tags::getMatches(const Tags& input) const{
    int matches = 0;
    for (int i=0; i < input.tags_.size(); i++){
        for (int j=0; j < tags_.size(); j++){
            if(input.tags_[i] == tags_[j]){
                matches++;
            }
        }
    }
    return matches;
}

std::ostream& operator<<(std::ostream& os, const Tags& obj) {
    for (std::vector<Tag>::const_iterator it = obj.tags_.begin(); it != obj.tags_.end();
         ++it) {
        os << *it;
        if (std::distance(it, obj.tags_.end()) > 1) {
            os << ", ";
        }
    }
    return os;
}

const Tags Tags::None("");
const Tags Tags::GL("GL");
const Tags Tags::CL("CL");
const Tags Tags::CPU("CPU");

} // namespace

