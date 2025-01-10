/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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
#include <inviwo/core/util/fmtutils.h>
#include <iosfwd>
#include <string_view>

namespace inviwo {

/**
 * Controls the behavior of
 */
enum class ConstraintBehavior {
    /**
     * The default behavior. Clamps values and the boundary is editable by the user in the GUI and
     * by the programmer from code. The bounds are linked to other properties. Typical use case
     * would when you have a good default value for a bound, but other values are still valid.
     */
    Editable,

    /**
     * Clamps values and the boundary is editable by the programmer (setMinValue, setMaxValue) and
     * not from the GUI. Bounds are not linked to other properties. Typical use case would when you
     * have a bound that the user should not be able to modify but needs to be modified from the
     * programmers side, say for example the upper bound of the size of a vector when the value is
     * used for indexing.
     */
    Mutable,

    /**
     * Clamps values and the boundary can not be modified. Bounds are not linked to other
     * properties. Typical use case would something like a color where there is a defined range,
     * (0,1) in this case, that should never be modified.
     */
    Immutable,

    /**
     * Don't clamp values and the boundary is editable by the user and by the programmer. The bounds
     * are only used for interaction purposes. Bounds are linked to other properties. Typical use
     * case would be for a value of unbounded character, like the look from in the camera. The any
     * value is usually valid, the bound are only used to suggest a reasonable value.
     */
    Ignore

};

IVW_CORE_API std::string_view enumToStr(ConstraintBehavior cb);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, ConstraintBehavior cb);

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ConstraintBehavior>
    : inviwo::FlagFormatter<inviwo::ConstraintBehavior> {};
#endif
