/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <inviwo/core/util/ostreamjoiner.h>

#include <flags/flags.h>

#include <iterator>
#include <ostream>
#include <string_view>

#include <fmt/format.h>

namespace inviwo {

enum class CoordinatePlane { XY, XZ, YZ, ZY };

enum class CartesianCoordinateAxis {
    X,  // Right
    Y,  // Up
    Z   // Into screen
};

enum class BufferType {
    PositionAttrib = 0,
    NormalAttrib,
    ColorAttrib,
    TexcoordAttrib,
    CurvatureAttrib,
    IndexAttrib,
    RadiiAttrib,
    PickingAttrib,
    ScalarMetaAttrib,
    NumberOfBufferTypes
};

ALLOW_FLAGS_FOR_ENUM(BufferType)
using BufferTypes = flags::flags<BufferType>;

enum class BufferUsage { Static, Dynamic };

enum class BufferTarget {
    Data,
    Index
};  // Index maps to GL_ELEMENT_ARRAY_BUFFER, Data maps to GL_ARRAY_BUFFER

enum class DrawType { NotSpecified = 0, Points, Lines, Triangles, NumberOfDrawTypes };

enum class ConnectivityType {
    None = 0,
    Strip,
    Loop,
    Fan,
    Adjacency,
    StripAdjacency,
    NumberOfConnectivityTypes
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, DrawType dt) {
    switch (dt) {
        case DrawType::Points:
            ss << "Points";
            break;
        case DrawType::Lines:
            ss << "Lines";
            break;
        case DrawType::Triangles:
            ss << "Triangles";
            break;
        case DrawType::NotSpecified:
        case DrawType::NumberOfDrawTypes:
        default:
            ss << "Not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             ConnectivityType ct) {
    switch (ct) {
        case ConnectivityType::None:
            ss << "None";
            break;
        case ConnectivityType::Strip:
            ss << "Strip";
            break;
        case ConnectivityType::Loop:
            ss << "Loop";
            break;
        case ConnectivityType::Fan:
            ss << "Fan";
            break;
        case ConnectivityType::Adjacency:
            ss << "Adjacency";
            break;
        case ConnectivityType::StripAdjacency:
            ss << "Strip adjacency";
            break;
        case ConnectivityType::NumberOfConnectivityTypes:
        default:
            ss << "Not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, BufferType bt) {
    switch (bt) {
        case BufferType::PositionAttrib:
            ss << "Position";
            break;
        case BufferType::NormalAttrib:
            ss << "Normal";
            break;
        case BufferType::ColorAttrib:
            ss << "Color";
            break;
        case BufferType::TexcoordAttrib:
            ss << "Texture";
            break;
        case BufferType::CurvatureAttrib:
            ss << "Curvature";
            break;
        case BufferType::IndexAttrib:
            ss << "Index";
            break;
        case BufferType::RadiiAttrib:
            ss << "Radii";
            break;
        case BufferType::PickingAttrib:
            ss << "Picking";
            break;
        case BufferType::ScalarMetaAttrib:
            ss << "ScalarMeta";
            break;
        case BufferType::NumberOfBufferTypes:
            ss << "NumberOfBufferTypes";
            break;
        default:
            ss << "Type not specified";
            break;
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, BufferUsage bu) {
    switch (bu) {
        case BufferUsage::Static:
            ss << "Static";
            break;
        case BufferUsage::Dynamic:
            ss << "Dynamic";
            break;
        default:
            ss << "Usage not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             BufferTarget bt) {
    switch (bt) {
        case BufferTarget::Data:
            ss << "Data";
            break;
        case BufferTarget::Index:
            ss << "Index";
            break;
        default:
            ss << "Target not specified";
    }
    return ss;
}

}  // namespace inviwo

namespace fmt {
template <>
struct formatter<::inviwo::BufferType> : formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(::inviwo::BufferType bt, FormatContext& ctx) {
        using namespace std::literals;
        const std::string_view str = [&]() {
            switch (bt) {
                case ::inviwo::BufferType::PositionAttrib:
                    return "Position"sv;
                case ::inviwo::BufferType::NormalAttrib:
                    return "Normal"sv;
                case ::inviwo::BufferType::ColorAttrib:
                    return "Color"sv;
                case ::inviwo::BufferType::TexcoordAttrib:
                    return "Texture"sv;
                case ::inviwo::BufferType::CurvatureAttrib:
                    return "Curvature"sv;
                case ::inviwo::BufferType::IndexAttrib:
                    return "Index"sv;
                case ::inviwo::BufferType::RadiiAttrib:
                    return "Radii"sv;
                case ::inviwo::BufferType::PickingAttrib:
                    return "Picking"sv;
                case ::inviwo::BufferType::ScalarMetaAttrib:
                    return "ScalarMeta"sv;
                case ::inviwo::BufferType::NumberOfBufferTypes:
                    return "NumberOfBufferTypes"sv;
                default:
                    return "Type not specified"sv;
            }
        }();
        return formatter<string_view>::format(str, ctx);
    }
};
}  // namespace fmt
