/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/cell.h>

namespace inviwo {
namespace discretedata {
namespace dd_util {

inline double tetrahedronVolume(double corners[4][3]) {
    const glm::dvec3 a(corners[0][0] - corners[1][0], corners[0][1] - corners[1][1],
                       corners[0][2] - corners[1][2]);
    const glm::dvec3 b(corners[0][0] - corners[2][0], corners[0][1] - corners[2][1],
                       corners[0][2] - corners[2][2]);
    const glm::dvec3 c(corners[0][0] - corners[3][0], corners[0][1] - corners[3][1],
                       corners[0][2] - corners[3][2]);

    return fabs(glm::dot(glm::cross(a, b), c) / 6.0);
}

/** \brief Get the primitive type associated with the cell type
 *  @param type Geometric description of the cell
 */
inline GridPrimitive cellTypeToGridPrimitive(CellType type) {
    switch (type) {
        case CellType::Vertex:
        case CellType::PolyVertex:
            return GridPrimitive::Vertex;
            // Linear cells
        case CellType::Line:
        case CellType::PolyLine:
            return GridPrimitive::Edge;
        case CellType::Triangle:
        case CellType::TriangleStrip:
        case CellType::Polygon:
        case CellType::Pixel:
        case CellType::Quad:
            return GridPrimitive::Face;
        case CellType::Tetra:
        case CellType::Voxel:
        case CellType::Hexahedron:
        case CellType::Wedge:
        case CellType::Pyramid:
        case CellType::PentagonalPrism:
        case CellType::HexagonalPrism:
            return GridPrimitive::Volume;

        // Quadratic, isoparametric cells
        case CellType::QuadraticEdge:
            return GridPrimitive::Edge;
        case CellType::QuadraticTriangle:
        case CellType::QuadraticQuad:
        case CellType::QuadraticPolygon:
            return GridPrimitive::Face;
        case CellType::QuadraticTetra:
        case CellType::QuadraticHexahedron:
        case CellType::QuadraticWedge:
        case CellType::QuadraticPyramid:
            return GridPrimitive::Volume;
        case CellType::BiquadraticQuad:
            return GridPrimitive::Face;
        case CellType::TriquadraticHexahedron:
            return GridPrimitive::Volume;
        case CellType::QuadraticLinearQuad:
            return GridPrimitive::Face;
        case CellType::QuadraticLinearWedge:
        case CellType::BiquadraticQuadraticWedge:
        case CellType::BiquadraticQuadraticHexahedron:
            return GridPrimitive::Volume;
        case CellType::BiquadraticTriangle:
            return GridPrimitive::Face;

        // Cubic, isoparametric cell
        case CellType::CubicLine:
            return GridPrimitive::Face;

        default:
            return GridPrimitive::Undef;
    };
}

}  // namespace dd_util
}  // namespace discretedata
}  // namespace inviwo
