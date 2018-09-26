/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#include "discretedata/connectivity/connectivity.h"
#include "discretedata/connectivity/cell.h"

namespace inviwo {
namespace discretedata {
namespace dd_util {

inline double tetrahedronVolume(double corners[4][3]) {
    const glm::dvec3 a(corners[0][0] - corners[1][0], corners[0][1] - corners[1][1], corners[0][2] - corners[1][2]);
    const glm::dvec3 b(corners[0][0] - corners[2][0], corners[0][1] - corners[2][1], corners[0][2] - corners[2][2]);
    const glm::dvec3 c(corners[0][0] - corners[3][0], corners[0][1] - corners[3][1], corners[0][2] - corners[3][2]);

    return fabs(glm::dot(glm::cross(a, b), c) / 6.0);
}

/** \brief Get the primitive type associated with the cell type
 *  @param type Geometric description of the cell
 */
inline GridPrimitive cellTypeToGridPrimitive(CellType type) {
    switch (type) {
        case CellType::VERTEX:
        case CellType::POLY_VERTEX:
            return GridPrimitive::Vertex;
            // Linear cells
        case CellType::LINE:
        case CellType::POLY_LINE:
            return GridPrimitive::Edge;
        case CellType::TRIANGLE:
        case CellType::TRIANGLE_STRIP:
        case CellType::POLYGON:
        case CellType::PIXEL:
        case CellType::QUAD:
            return GridPrimitive::Face;
        case CellType::TETRA:
        case CellType::VOXEL:
        case CellType::HEXAHEDRON:
        case CellType::WEDGE:
        case CellType::PYRAMID:
        case CellType::PENTAGONAL_PRISM:
        case CellType::HEXAGONAL_PRISM:
            return GridPrimitive::Volume;

        // Quadratic, isoparametric cells
        case CellType::QUADRATIC_EDGE:
            return GridPrimitive::Edge;
        case CellType::QUADRATIC_TRIANGLE:
        case CellType::QUADRATIC_QUAD:
        case CellType::QUADRATIC_POLYGON:
            return GridPrimitive::Face;
        case CellType::QUADRATIC_TETRA:
        case CellType::QUADRATIC_HEXAHEDRON:
        case CellType::QUADRATIC_WEDGE:
        case CellType::QUADRATIC_PYRAMID:
            return GridPrimitive::Volume;
        case CellType::BIQUADRATIC_QUAD:
            return GridPrimitive::Face;
        case CellType::TRIQUADRATIC_HEXAHEDRON:
            return GridPrimitive::Volume;
        case CellType::QUADRATIC_LINEAR_QUAD:
            return GridPrimitive::Face;
        case CellType::QUADRATIC_LINEAR_WEDGE:
        case CellType::BIQUADRATIC_QUADRATIC_WEDGE:
        case CellType::BIQUADRATIC_QUADRATIC_HEXAHEDRON:
            return GridPrimitive::Volume;
        case CellType::BIQUADRATIC_TRIANGLE:
            return GridPrimitive::Face;

        // Cubic, isoparametric cell
        case CellType::CUBIC_LINE:
            return GridPrimitive::Face;

        default:
            return GridPrimitive::Undef;
    };
}

} // namespace
}
}
