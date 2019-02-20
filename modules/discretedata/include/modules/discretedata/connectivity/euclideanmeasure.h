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

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/connectivity/cell.h>
#include <inviwo/core/util/formatdispatching.h>
#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
namespace discretedata {
namespace euclidean {

/**
 * \brief Get the measure (i.e., length, area, volume...) of an element
 * @param dim Dimension of element (edge, face, volume...)
 * @param index Index of respective element type
 * @param cell Cell type
 * @param positions Vertex positions
 */
IVW_MODULE_DISCRETEDATA_API double getMeasure(const Connectivity& grid, const Channel& positions,
                                              GridPrimitive dim, ind index);

/**
 * \brief Get the measure (i.e., length, area, volume...) of an element
 * @param element Element to get measure of
 * @param positions Vertex positions
 */
inline double getMeasure(const Channel& positions, const ElementIterator& element) {
    return getMeasure(*element.getGrid(), positions, element.getType(), element.getIndex());
}

struct HexVolumeComputer {
    template <typename T>
    double computeHexVolume(const Channel& positions, const std::vector<ind>& corners);

    template <typename R, typename T>
    double operator()(const Connectivity& grid, const Channel& positions, ind index);
};

template <typename R, typename T>
double HexVolumeComputer::operator()(const Connectivity& grid, const Channel& positions,
                                     ind index) {

    // Get all corner points.
    std::vector<ind> corners;
    grid.getConnections(corners, index, GridPrimitive::Volume, GridPrimitive::Vertex, true);
    return computeHexVolume<typename T::type>(positions, corners);
}

template <typename T>
double HexVolumeComputer::computeHexVolume(const Channel& positions,
                                           const std::vector<ind>& corners) {
    assert(corners.size() == 8 && "Not a hexahedron.");

    // Work with respective type
    const DataChannel<T, 3>* doubleVertices = dynamic_cast<const DataChannel<T, 3>*>(&positions);
    if (!doubleVertices) return -1;

    // Tetrahedron corners
    static constexpr ind tetrahedra[5][4] = {
        {0, 3, 5, 6}, {1, 0, 3, 5}, {0, 2, 6, 3}, {5, 3, 6, 7}, {4, 5, 6, 0}};

    // Setup variables for measure calculation
    double measure = 0;
    double cornerMatrix[4][3];
    std::array<T, 3> vertex;

    // Calculate measure of 5 tetrahedra
    for (ind tet = 0; tet < 5; tet++) {
        for (ind corner = 0; corner < 4; corner++) {
            ind cornerIndex = corners[tetrahedra[tet][corner]];
            doubleVertices->fill(vertex, cornerIndex);
            for (ind dim = 0; dim < 3; ++dim) {
                cornerMatrix[corner][dim] = double(vertex[(unsigned)dim]);
            }
        }

        // Compute measure and sum
        measure += dd_util::tetrahedronVolume(cornerMatrix);
    }

    return measure;
}

}  // namespace euclidean
}  // namespace discretedata
}  // namespace inviwo
