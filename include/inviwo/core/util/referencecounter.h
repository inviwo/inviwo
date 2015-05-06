/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_REFERENCE_COUNTER_H
#define IVW_REFERENCE_COUNTER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

/** \class ReferenceCounter
 *
 * Reference counter for an object. Object should be removed when reference count equals zero.
 */
class IVW_CORE_API ReferenceCounter {
public:
    ReferenceCounter();;
    virtual ~ReferenceCounter();
    /**
     * Increase reference count of this object.
     * Object should not be removed unless reference count is zero.
     *
     * @return New reference count after increasing it.
     */
    int increaseRefCount();

    /**
     * Decrease reference count of this object.
     * Object should not be removed unless reference count is zero.
     *
     * @return New reference count after decreasing it.
     */
    int decreaseRefCount();
    /**
     * Get reference count of this object.
     * Object should not be removed unless reference count is zero.
     *
     */
    int getRefCount() const;

private:
    int referenceCount_;
};

};  // namespace inviwo

#endif  // IVW_REFERENCE_COUNTER_H