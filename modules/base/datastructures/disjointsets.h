/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_DISJOINTSETS_H
#define IVW_DISJOINTSETS_H

#include <modules/base/basemoduledefine.h>

#include <vector>

namespace inviwo {

class DisjointSets {
public:
    explicit DisjointSets(int size);

    /** 
     * join sets of element r and s 
     * return true it sets were joined and false if they
     * already were joined
     */
    bool join(int r, int s);

    /** 
     * return name of current set for x
     * i.e. return root of tree for x
     */
    int find(int x);

    /**
     * return size of current set for x
     */
    int size(int x);

private:
    std::vector<int> array_;
};

inline DisjointSets::DisjointSets(int size) : array_(size, -1) {}

inline bool DisjointSets::join(int r, int s) {
    r = find(r);
    s = find(s);
    if (r == s) return false;
    // weighted union (by size)
    if (array_[r] <= array_[s]) {
        array_[r] += array_[s];
        array_[s] = r;
    } else {
        array_[s] += array_[r];
        array_[r] = s;
    }
    return true;
}

inline int DisjointSets::find(int x) {
    // find with path compression
    if (array_[x] < 0) {
        return x;
    } else {
        return array_[x] = find(array_[x]);
    }
}

inline int DisjointSets::size(int x) {
    auto r = find(x);
    return -array_[r];
}


} // namespace

#endif // IVW_DISJOINTSETS_H

