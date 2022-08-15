/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

std::string_view util::name(DrawType dt) {
    switch (dt) {
        case DrawType::Points:
            return "Points";
        case DrawType::Lines:
            return "Lines";
        case DrawType::Triangles:
            return "Triangles";
        case DrawType::NotSpecified:
            return "Not specified";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid DrawType enum value '{}'",
                    static_cast<int>(dt));
}
std::string_view util::name(ConnectivityType ct) {
    switch (ct) {
        case ConnectivityType::None:
            return "None";
        case ConnectivityType::Strip:
            return "Strip";
        case ConnectivityType::Loop:
            return "Loop";
        case ConnectivityType::Fan:
            return "Fan";
        case ConnectivityType::Adjacency:
            return "Adjacency";
        case ConnectivityType::StripAdjacency:
            return "Strip adjacency";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"),
                    "Found invalid ConnectivityType enum value '{}'", static_cast<int>(ct));
}
std::string_view util::name(BufferType bt) {
    switch (bt) {
        case BufferType::PositionAttrib:
            return "Position";
        case BufferType::NormalAttrib:
            return "Normal";
        case BufferType::ColorAttrib:
            return "Color";
        case BufferType::TexCoordAttrib:
            return "TexCoord";
        case BufferType::CurvatureAttrib:
            return "Curvature";
        case BufferType::IndexAttrib:
            return "Index";
        case BufferType::RadiiAttrib:
            return "Radii";
        case BufferType::PickingAttrib:
            return "Picking";
        case BufferType::ScalarMetaAttrib:
            return "ScalarMeta";
        case BufferType::Unknown:
            return "Unknown";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid BufferType enum value '{}'",
                    static_cast<int>(bt));
}
std::string_view util::name(BufferUsage bu) {
    switch (bu) {
        case BufferUsage::Static:
            return "Static";
        case BufferUsage::Dynamic:
            return "Dynamic";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid BufferUsage enum value '{}'",
                    static_cast<int>(bu));
}
std::string_view util::name(BufferTarget bt) {
    switch (bt) {
        case BufferTarget::Data:
            return "Data";
        case BufferTarget::Index:
            return "Index";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid BufferTarget enum value '{}'",
                    static_cast<int>(bt));
}

std::ostream& operator<<(std::ostream& ss, DrawType dt) { return ss << util::name(dt); }
std::ostream& operator<<(std::ostream& ss, ConnectivityType ct) { return ss << util::name(ct); }
std::ostream& operator<<(std::ostream& ss, BufferType bt) { return ss << util::name(bt); }
std::ostream& operator<<(std::ostream& ss, BufferUsage bu) { return ss << util::name(bu); }
std::ostream& operator<<(std::ostream& ss, BufferTarget bt) { return ss << util::name(bt); }

}  // namespace inviwo
