/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_INDEXMAPPER_H
#define IVW_INDEXMAPPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {
    namespace util {
        /**
         * \class IndexMapper
         *
         * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
         *
         * DESCRIBE_THE_CLASS
         */
        struct IndexMapper {
            inline IndexMapper(const size3_t &dim) : dimx(dim.x), dimxy(dim.x * dim.y) {};
            inline size_t operator() (size_t x, size_t y, size_t z) {
                return x + y * dimx + z * dimxy;
            }
            inline size_t operator() (long long x, long long y, long long z) {
                return static_cast<size_t>(x + y * dimx + z * dimxy);
            }
            inline size_t operator() (const size3_t &pos) { return pos.x + pos.y * dimx + pos.z * dimxy; }

        private:
            const size_t dimx;
            const size_t dimxy;
        };
    } //namespace util
}  // namespace

#endif  // IVW_INDEXMAPPER_H
