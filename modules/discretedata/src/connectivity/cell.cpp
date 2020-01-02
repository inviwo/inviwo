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

#include <modules/discretedata/connectivity/cell.h>

namespace inviwo {
namespace discretedata {

const std::array<const CellStructure*, (int)CellType::NumberOfCellTypes> CellStructureByCellType = {
    // Linear cells
    nullptr,         // EmptyCell
    &VERTEX_CELL,    // Vertex
    nullptr,         // PolyVertex
    &EDGE_CELL,      // Line
    nullptr,         // PolyLine
    &TRIANGLE_CELL,  // Triangle
    nullptr,         // TriangleStrip
    &POLYGON_CELL,   // Polygon
    &QUAD_CELL,      // Pixel
    &QUAD_CELL,      // Quad
    nullptr,         // Tetra
    nullptr,         // Voxel
    nullptr,         // Hexahedron
    nullptr,         // Wedge
    nullptr,         // Pyramid
    nullptr,         // PentagonalPrism
    nullptr,         // HexagonalPrism

    // Quadratic, isoparametric cells
    nullptr,  // QuadraticEdge
    nullptr,  // QuadraticTriangle
    nullptr,  // QuadraticQuad
    nullptr,  // QuadraticPolygon
    nullptr,  // QuadraticTetra
    nullptr,  // QuadraticHexahedron
    nullptr,  // QuadraticWedge
    nullptr,  // QuadraticPyramid
    nullptr,  // BiquadraticQuad
    nullptr,  // TriquadraticHexahedron
    nullptr,  // QuadraticLinearQuad
    nullptr,  // QuadraticLinearWedge
    nullptr,  // BiquadraticQuadraticWedge
    nullptr,  // BiquadraticQuadraticHexahedron
    nullptr,  // BiquadraticTriangle

    // Cubic, isoparametric cell
    nullptr,  // CubicLine

    // Special class of cells formed by convex group of points
    nullptr,  // ConvexPointSet

    // Polyhedron cell (consisting of polygonal faces)
    nullptr,  // Polyhedron

    // Higher order cells in parametric form
    nullptr,  // ParametricCurve
    nullptr,  // ParametricSurface
    nullptr,  // ParametricTriSurface
    nullptr,  // ParametricQuadSurface
    nullptr,  // ParametricTetraRegion
    nullptr,  // ParametricHexRegion

    // Higher order cells
    nullptr,  // HigherOrderEdge
    nullptr,  // HigherOrderTriangle
    nullptr,  // HigherOrderQuad
    nullptr,  // HigherOrderPolygon
    nullptr,  // HigherOrderTetrahedron
    nullptr,  // HigherOrderWedge
    nullptr,  // HigherOrderPyramid
    nullptr,  // HigherOrderHexahedron

    // Arbitrary order Lagrange elements
    // (formulated separated from generic higher order cells)
    nullptr,  // LagrangeCurve
    nullptr,  // LagrangeTriangle
    nullptr,  // LagrangeQuadrilateral
    nullptr,  // LagrangeTetrahedron
    nullptr,  // LagrangeHexahedron
    nullptr,  // LagrangeWedge
    nullptr   // LagrangePyramid
};

// ind TriangleCellStructure::getNumElements(GridPrimitive dim, const std::vector<ind>&) const {
//     if (dim == GridPrimitive::Edge) return 3;
// }

void TriangleCellStructure::getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                                  GridPrimitive dim,
                                                  const std::vector<ind>& vertices) const {
    if (dim != GridPrimitive::Edge) return;
    elementsOut.data_.insert(elementsOut.data_.end(), {vertices[0], vertices[1],    // Edge 0
                                                       vertices[1], vertices[2],    // Edge 1
                                                       vertices[0], vertices[2]});  // Edge 2
    elementsOut.prefixSum_.insert(elementsOut.prefixSum_.end(), {2, 2, 2});
}
const CellStructure* TriangleCellStructure::getElementCellStructure(GridPrimitive dim,
                                                                    ind index) const {
    if (dim != GridPrimitive::Edge) return nullptr;
    return CellStructureByCellType[(int)CellType::Line];
}

// ind QuadCellStructure::getNumElements(GridPrimitive dim, const std::vector<ind>&) const {
//     if (dim == GridPrimitive::Edge) return 4;
// }

void QuadCellStructure::getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                              GridPrimitive dim,
                                              const std::vector<ind>& vertices) const {
    if (dim != GridPrimitive::Edge) return;
    elementsOut.data_.insert(elementsOut.data_.end(), {vertices[0], vertices[1],    // Edge 0
                                                       vertices[1], vertices[2],    // Edge 1
                                                       vertices[2], vertices[3],    // Edge 2
                                                       vertices[0], vertices[3]});  // Edge 3
    elementsOut.prefixSum_.insert(elementsOut.prefixSum_.end(), {2, 2, 2, 2});
}

const CellStructure* QuadCellStructure::getElementCellStructure(GridPrimitive dim,
                                                                ind index) const {
    if (dim != GridPrimitive::Edge) return nullptr;
    return CellStructureByCellType[(int)CellType::Line];
}

// ind PolygonCellStructure::getNumElements(GridPrimitive dim,
//                                          const std::vector<ind>& vertices) const {
//     if (dim == GridPrimitive::Edge) return vertices.size();
// }

/** Get the primitves (e.g., edges) by the vertices that make them up **/
void PolygonCellStructure::getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                                 GridPrimitive dim,
                                                 const std::vector<ind>& vertices) const {
    if (dim != GridPrimitive::Edge) return;

    for (size_t i = 0; i < vertices.size(); ++i) {
        elementsOut.data_.push_back(vertices[i]);
        elementsOut.data_.push_back(vertices[(i + 1) % vertices.size()]);
        elementsOut.prefixSum_.push_back(2);
    }
}
const CellStructure* PolygonCellStructure::getElementCellStructure(GridPrimitive dim,
                                                                   ind index) const {
    if (dim != GridPrimitive::Edge) return nullptr;
    return CellStructureByCellType[(int)CellType::Line];
}

// template<
//     GridPrimitive PrimitiveType
//     , ind NumVertices
//     , CellType EdgeType = CellType::EmptyCell
//     , ind numEdges = 0
//     , CellType FaceType = CellType::EmptyCell
//     , ind NumFaces = 0>
// struct CellInfoConstant{ using IsSizeConstant = true; };

// template<CellType EdgeType = CellType::EmptyCell, CellType FaceType = CellType::EmptyCell>
// struct CellInfoPoly{ using IsSizeConstant = false; };

// template <CellType Type>
// struct CellInfo {
// };

// template <>
// struct CellInfo<CellType::Vertex> : CellInfoConstant<GridPrimitive::Vertex, 1> {};

// template <>
// struct CellInfo<CellType::PolyVertex> : CellInfoConstant<GridPrimitive::Vertex, 1> {};

}  // namespace discretedata
}  // namespace inviwo
