/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>

#include <ostream>

namespace inviwo {

template class IVW_CORE_TMPL_INST SpatialCoordinateTransformer<2>;
template class IVW_CORE_TMPL_INST SpatialCoordinateTransformer<3>;
template class IVW_CORE_TMPL_INST SpatialCameraCoordinateTransformer<2>;
template class IVW_CORE_TMPL_INST SpatialCameraCoordinateTransformer<3>;

template class IVW_CORE_TMPL_INST StructuredCoordinateTransformer<2>;
template class IVW_CORE_TMPL_INST StructuredCoordinateTransformer<3>;
template class IVW_CORE_TMPL_INST StructuredCameraCoordinateTransformer<2>;
template class IVW_CORE_TMPL_INST StructuredCameraCoordinateTransformer<3>;

std::string_view enumToStr(CoordinateSpace s) {
    switch (s) {
        case CoordinateSpace::Data:
            return "Data";
        case CoordinateSpace::Model:
            return "Model";
        case CoordinateSpace::World:
            return "World";
        case CoordinateSpace::Index:
            return "Index";
        case CoordinateSpace::Clip:
            return "Clip";
        case CoordinateSpace::View:
            return "View";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid CoordinateSpace enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, CoordinateSpace s) { return ss << enumToStr(s); }

}  // namespace inviwo
