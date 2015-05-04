/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_GROUP_H
#define IVW_GROUP_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <map>

namespace inviwo {

/*

1) Group can consist key and data
2) Key, for example, can be std::string, Processor* , int ... etc
3) Data, for example, can be Port*, Processor*, Canvas* ... etc

Example Group types: <std::string, Port*> where Key is std::string, Data is Port*
                     <std::string, Processor*> where Key is std::string, Data is Processor*
                     <Processor*, Port*> where Key is Processor, Data is Port*

 */
template <typename Key, typename Data>
class Group {
public:
    Group();
    ~Group();

    // Erase all data corresponding to each key
    void clear();

    // Get the data corresponding to supplied key. There can be multiple data for each key.
    std::vector<Data> getGroupedData(Key groupKEY) const;

    // Get all existing keys in map with no duplicates
    std::vector<Key> getGroupKeys() const;

    // Get key that corresponds to data
    Key getKey(const Data& data) const;

    // Insert key and data
    void insert(Key key, Data data);

private:
    using GroupMap = std::multimap<Key, Data>;
    GroupMap groupMap_;
};

template <typename Key, typename Data>
Group<Key, Data>::Group() {}

template <typename Key, typename Data>
Group<Key, Data>::~Group() {}

template <typename Key, typename Data>
void Group<Key, Data>::clear() {
    groupMap_.clear();
}

template <typename Key, typename Data>
std::vector<Data> Group<Key, Data>::getGroupedData(Key groupKEY) const {
    std::vector<Data> ports;

    for (const auto& elem : util::as_range(groupMap_.equal_range(groupKEY)))
        ports.push_back(elem.second);

    return ports;
}

template <typename Key, typename Data>
std::vector<Key> Group<Key, Data>::getGroupKeys() const {
    std::vector<Key> groups;

    for (auto it = groupMap_.begin(); it != groupMap_.end();
         it = groupMap_.upper_bound(it->first)) {
        groups.push_back(it->first);
    }

    return groups;
}

template <typename Key, typename Data>
Key Group<Key, Data>::getKey(const Data& data) const {
    Key key {};

    for (const auto& elem : groupMap_) {
        if (elem.second == data) {
            key = elem.first;
            break;
        }
    }

    return key;
}

template <typename Key, typename Data>
void Group<Key, Data>::insert(Key key, Data data) {
    groupMap_.insert(std::pair<Key, Data>(key, data));
}

} // namespace

#endif
