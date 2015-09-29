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
    POSITION_ATTRIB = 0,
    NORMAL_ATTRIB,
    COLOR_ATTRIB,
    TEXCOORD_ATTRIB,
    CURVATURE_ATTRIB,
    INDEX_ATTRIB,
    NUMBER_OF_BUFFER_TYPES
};

enum class BufferUsage { STATIC, DYNAMIC };

enum class DrawType { NOT_SPECIFIED = 0, POINTS, LINES, TRIANGLES, NUMBER_OF_DRAW_TYPES };

enum class ConnectivityType {
    NONE = 0,
    STRIP,
    LOOP,
    FAN,
    ADJACENCY,
    STRIP_ADJACENCY,
    NUMBER_OF_CONNECTIVITY_TYPES
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, DrawType dt) {
    switch (dt) {
        case DrawType::POINTS:
            ss << "Points";
            break;
        case DrawType::LINES:
            ss << "Lines";
            break;
        case DrawType::TRIANGLES:
            ss << "Triangles";
            break;
        case DrawType::NOT_SPECIFIED:
        case DrawType::NUMBER_OF_DRAW_TYPES:
        default:
            ss << "Not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             ConnectivityType ct) {
    switch (ct) {
        case ConnectivityType::NONE:
            ss << "None";
            break;
        case ConnectivityType::STRIP:
            ss << "Strip";
            break;
        case ConnectivityType::LOOP:
            ss << "Loop";
            break;
        case ConnectivityType::FAN:
            ss << "Fan";
            break;
        case ConnectivityType::ADJACENCY:
            ss << "Adjacency";
            break;
        case ConnectivityType::STRIP_ADJACENCY:
            ss << "Strip adjacency";
            break;
        case ConnectivityType::NUMBER_OF_CONNECTIVITY_TYPES:
        default:
            ss << "Not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, BufferType bt) {
    switch (bt) {
        case BufferType::POSITION_ATTRIB:
            ss << "Positions";
            break;
        case BufferType::NORMAL_ATTRIB:
            ss << "Normals";
            break;
        case BufferType::COLOR_ATTRIB:
            ss << "Colors";
            break;
        case BufferType::TEXCOORD_ATTRIB:
            ss << "Texture";
            break;
        case BufferType::CURVATURE_ATTRIB:
            ss << "Curvature";
            break;
        case BufferType::INDEX_ATTRIB:
            ss << "Index";
            break;
        case BufferType::NUMBER_OF_BUFFER_TYPES:
        default:
            ss << "Type not specified";
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, BufferUsage ut) {
    switch (ut) {
        case BufferUsage::STATIC:
            ss << "Static";
            break;
        case BufferUsage::DYNAMIC:
            ss << "Dynamic";
            break;
        default:
            ss << "Usage not specified";
    }
    return ss;
}

}  // namespace

#endif  // IVW_GEOMETRYTYPE_H
