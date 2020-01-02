/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/util/prefixsumvector.h>

namespace inviwo {
namespace discretedata {

struct CellStructure;

/** Cell types of vtk **/
enum class CellType {
    // Linear cells
    EmptyCell = 0,
    Vertex = 1,
    PolyVertex = 2,
    Line = 3,
    PolyLine = 4,
    Triangle = 5,
    TriangleStrip = 6,
    Polygon = 7,
    Pixel = 8,
    Quad = 9,
    Tetra = 10,
    Voxel = 11,
    Hexahedron = 12,
    Wedge = 13,
    Pyramid = 14,
    PentagonalPrism = 15,
    HexagonalPrism = 16,

    // Quadratic, isoparametric cells
    QuadraticEdge = 21,
    QuadraticTriangle = 22,
    QuadraticQuad = 23,
    QuadraticPolygon = 36,
    QuadraticTetra = 24,
    QuadraticHexahedron = 25,
    QuadraticWedge = 26,
    QuadraticPyramid = 27,
    BiquadraticQuad = 28,
    TriquadraticHexahedron = 29,
    QuadraticLinearQuad = 30,
    QuadraticLinearWedge = 31,
    BiquadraticQuadraticWedge = 32,
    BiquadraticQuadraticHexahedron = 33,
    BiquadraticTriangle = 34,

    // Cubic, isoparametric cell
    CubicLine = 35,

    // Special class of cells formed by convex group of points
    ConvexPointSet = 41,

    // Polyhedron cell (consisting of polygonal faces)
    Polyhedron = 42,

    // Higher order cells in parametric form
    ParametricCurve = 51,
    ParametricSurface = 52,
    ParametricTriSurface = 53,
    ParametricQuadSurface = 54,
    ParametricTetraRegion = 55,
    ParametricHexRegion = 56,

    // Higher order cells
    HigherOrderEdge = 60,
    HigherOrderTriangle = 61,
    HigherOrderQuad = 62,
    HigherOrderPolygon = 63,
    HigherOrderTetrahedron = 64,
    HigherOrderWedge = 65,
    HigherOrderPyramid = 66,
    HigherOrderHexahedron = 67,

    // Arbitrary order Lagrange elements
    // (formulated separated from generic higher order cells)
    LagrangeCurve = 68,
    LagrangeTriangle = 69,
    LagrangeQuadrilateral = 70,
    LagrangeTetrahedron = 71,
    LagrangeHexahedron = 72,
    LagrangeWedge = 73,
    LagrangePyramid = 74,

    NumberOfCellTypes
};

/** Map from vtk cell types to corresponding cell structure structs **/
extern const std::array<const CellStructure*, (int)CellType::NumberOfCellTypes>
    CellStructureByCellType;

struct CellStructure {
    const GridPrimitive primitive_;
    CellStructure(GridPrimitive prim) : primitive_(prim) {}
    /** Number of primitves (e.g., edges) of this cell type **/
    virtual ind getNumElements(GridPrimitive dim, const std::vector<ind>& vertices) const = 0;

    /** Get the primitves (e.g., edges) by the vertices that make them up **/
    virtual void getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                       GridPrimitive dim,
                                       const std::vector<ind>& vertices) const = 0;
    /** Get the type of primitives **/
    virtual const CellStructure* getElementCellStructure(GridPrimitive dim, ind index) const = 0;
};

// template <GridPrimitive Primitive>
struct DefaultCellStructure : CellStructure {
    DefaultCellStructure(GridPrimitive prim) : CellStructure(prim) {}

    virtual ind getNumElements(GridPrimitive, const std::vector<ind>&) const override { return 0; }

    virtual void getElementsByVertices(dd_util::PrefixSumVector<ind>&, GridPrimitive,
                                       const std::vector<ind>&) const override {}

    virtual const CellStructure* getElementCellStructure(GridPrimitive, ind) const {
        return nullptr;
    }
} static const UNDEFINED_CELL(GridPrimitive::Undef), VERTEX_CELL(GridPrimitive::Vertex),
    EDGE_CELL(GridPrimitive::Edge);

struct TriangleCellStructure : CellStructure {
    TriangleCellStructure() : CellStructure(GridPrimitive::Face) {}

    virtual ind getNumElements(GridPrimitive dim, const std::vector<ind>&) const override {
        if (dim == GridPrimitive::Edge) return 3;
        return 0;
    }

    virtual void getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                       GridPrimitive dim,
                                       const std::vector<ind>& vertices) const override;
    virtual const CellStructure* getElementCellStructure(GridPrimitive dim,
                                                         ind index) const override;
} static const TRIANGLE_CELL;

struct QuadCellStructure : CellStructure {
    QuadCellStructure() : CellStructure(GridPrimitive::Face) {}

    virtual ind getNumElements(GridPrimitive dim, const std::vector<ind>&) const override {
        if (dim == GridPrimitive::Edge) return 4;
        return 0;
    }

    virtual void getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                       GridPrimitive dim,
                                       const std::vector<ind>& vertices) const override;
    virtual const CellStructure* getElementCellStructure(GridPrimitive dim,
                                                         ind index) const override;
} static const QUAD_CELL;

struct PolygonCellStructure : CellStructure {
    PolygonCellStructure() : CellStructure(GridPrimitive::Face) {}

    virtual ind getNumElements(GridPrimitive dim, const std::vector<ind>& vertices) const override {
        if (dim == GridPrimitive::Edge) return vertices.size();
        return 0;
    }

    /** Get the primitves (e.g., edges) by the vertices that make them up **/
    virtual void getElementsByVertices(dd_util::PrefixSumVector<ind>& elementsOut,
                                       GridPrimitive dim,
                                       const std::vector<ind>& vertices) const override;
    virtual const CellStructure* getElementCellStructure(GridPrimitive dim,
                                                         ind index) const override;
} static const POLYGON_CELL;

}  // namespace discretedata
}  // namespace inviwo
