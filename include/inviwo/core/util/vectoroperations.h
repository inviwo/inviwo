/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VECTOROPERATIONS_H
#define IVW_VECTOROPERATIONS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <vector>

namespace inviwo {

template <typename T>
class ComparePointers {
public:
    explicit ComparePointers(const T* p) : p(p) {}
    bool operator() (const T* rhs) const { return p == rhs; }
private:
    const T* p;
};

template<class T, class U>
T* getTypeFromVector(std::vector<U> vec) {
    if (vec.size() > 0) {
        for (size_t i=0; i<vec.size(); i++) {
            T* item = dynamic_cast<T*>(vec[i]);

            if (item)
                return item;
        }
    }

    return NULL;
}

template<class T, class U>
bool hasTypeInVector(const std::vector<U> vec) {
    for (size_t i=0; i<vec.size(); i++) {
        T* item = dynamic_cast<T*>(vec[i]);

        if (item)
            return true;
    }

    return false;
}

template <typename T>
bool comparePtr(T* a, T* b) { return (*a < *b); }

template <typename T>
bool equalPtr(T* a, T* b) { return (*a == *b); }

} // namespace

#endif // IVW_VECTOROPERATIONS_H
