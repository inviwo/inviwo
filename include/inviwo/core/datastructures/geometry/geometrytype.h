/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_GEOMETRYTYPE_H
#define IVW_GEOMETRYTYPE_H

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
    NumberOfBufferTypes
};

enum class BufferUsage { Static, Dynamic };

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
            ss << "Positions";
            break;
        case BufferType::NormalAttrib:
            ss << "Normals";
            break;
        case BufferType::ColorAttrib:
            ss << "Colors";
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
        case BufferType::NumberOfBufferTypes:
        default:
            ss << "Type not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, BufferUsage ut) {
    switch (ut) {
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

}  // namespace

#endif  // IVW_GEOMETRYTYPE_H
