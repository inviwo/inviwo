/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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
#include <inviwo/core/util/fmtutils.h>

#include <iosfwd>
#include <string_view>

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
    TexCoordAttrib,
    CurvatureAttrib,
    IndexAttrib,
    RadiiAttrib,
    PickingAttrib,
    ScalarMetaAttrib,
    Unknown
};

enum class BufferUsage { Static, Dynamic };

enum class BufferTarget {
    Data,  // Data maps to GL_ARRAY_BUFFER
    Index  // Index maps to GL_ELEMENT_ARRAY_BUFFER
};

enum class DrawType { NotSpecified = 0, Points, Lines, Triangles };

enum class ConnectivityType { None = 0, Strip, Loop, Fan, Adjacency, StripAdjacency };

namespace util {
IVW_CORE_API std::string_view name(DrawType dt);
IVW_CORE_API std::string_view name(ConnectivityType ct);
IVW_CORE_API std::string_view name(BufferType bt);
IVW_CORE_API std::string_view name(BufferUsage bu);
IVW_CORE_API std::string_view name(BufferTarget bt);
}  // namespace util

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, DrawType dt);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, ConnectivityType ct);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, BufferType bt);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, BufferUsage bu);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, BufferTarget bt);

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::DrawType> : inviwo::FlagFormatter<inviwo::DrawType> {};
template <>
struct fmt::formatter<inviwo::ConnectivityType> : inviwo::FlagFormatter<inviwo::ConnectivityType> {
};
template <>
struct fmt::formatter<inviwo::BufferType> : inviwo::FlagFormatter<inviwo::BufferType> {};
template <>
struct fmt::formatter<inviwo::BufferUsage> : inviwo::FlagFormatter<inviwo::BufferUsage> {};
template <>
struct fmt::formatter<inviwo::BufferTarget> : inviwo::FlagFormatter<inviwo::BufferTarget> {};
