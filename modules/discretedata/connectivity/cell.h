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

namespace inviwo {
namespace discretedata {

/** Cell types of vtk **/
enum class CellType {
    // Linear cells
    EMPTY_CELL = 0,
    VERTEX = 1,
    POLY_VERTEX = 2,
    LINE = 3,
    POLY_LINE = 4,
    TRIANGLE = 5,
    TRIANGLE_STRIP = 6,
    POLYGON = 7,
    PIXEL = 8,
    QUAD = 9,
    TETRA = 10,
    VOXEL = 11,
    HEXAHEDRON = 12,
    WEDGE = 13,
    PYRAMID = 14,
    PENTAGONAL_PRISM = 15,
    HEXAGONAL_PRISM = 16,

    // Quadratic, isoparametric cells
    QUADRATIC_EDGE = 21,
    QUADRATIC_TRIANGLE = 22,
    QUADRATIC_QUAD = 23,
    QUADRATIC_POLYGON = 36,
    QUADRATIC_TETRA = 24,
    QUADRATIC_HEXAHEDRON = 25,
    QUADRATIC_WEDGE = 26,
    QUADRATIC_PYRAMID = 27,
    BIQUADRATIC_QUAD = 28,
    TRIQUADRATIC_HEXAHEDRON = 29,
    QUADRATIC_LINEAR_QUAD = 30,
    QUADRATIC_LINEAR_WEDGE = 31,
    BIQUADRATIC_QUADRATIC_WEDGE = 32,
    BIQUADRATIC_QUADRATIC_HEXAHEDRON = 33,
    BIQUADRATIC_TRIANGLE = 34,

    // Cubic, isoparametric cell
    CUBIC_LINE = 35,

    // Special class of cells formed by convex group of points
    CONVEX_POINT_SET = 41,

    // Polyhedron cell (consisting of polygonal faces)
    POLYHEDRON = 42,

    // Higher order cells in parametric form
    PARAMETRIC_CURVE = 51,
    PARAMETRIC_SURFACE = 52,
    PARAMETRIC_TRI_SURFACE = 53,
    PARAMETRIC_QUAD_SURFACE = 54,
    PARAMETRIC_TETRA_REGION = 55,
    PARAMETRIC_HEX_REGION = 56,

    // Higher order cells
    HIGHER_ORDER_EDGE = 60,
    HIGHER_ORDER_TRIANGLE = 61,
    HIGHER_ORDER_QUAD = 62,
    HIGHER_ORDER_POLYGON = 63,
    HIGHER_ORDER_TETRAHEDRON = 64,
    HIGHER_ORDER_WEDGE = 65,
    HIGHER_ORDER_PYRAMID = 66,
    HIGHER_ORDER_HEXAHEDRON = 67,

    // Arbitrary order Lagrange elements
    // (formulated separated from generic higher order cells)
    LAGRANGE_CURVE = 68,
    LAGRANGE_TRIANGLE = 69,
    LAGRANGE_QUADRILATERAL = 70,
    LAGRANGE_TETRAHEDRON = 71,
    LAGRANGE_HEXAHEDRON = 72,
    LAGRANGE_WEDGE = 73,
    LAGRANGE_PYRAMID = 74,

    NUMBER_OF_CELL_TYPES
};

}  // namespace
}