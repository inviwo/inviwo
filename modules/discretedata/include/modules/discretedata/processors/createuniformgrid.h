/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/glm.h>
// #include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

/*
class IVW_MODULE_DISCRETEDATA_API DimensionProperty : public CompositeProperty {
public:
    TransformListProperty(const std::string& identifier, const std::string& displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                          PropertySemantics semantics = PropertySemantics::Default);
    TransformListProperty(const TransformListProperty& other);
    ~TransformListProperty() = default;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    virtual TransformListProperty* clone() const override;

    const mat4& getMatrix() const;

    ListProperty transforms_;
    FloatMat4Property result_;
};
*/

/** \docpage{org.inviwo.CreateUniformGrid, Create Uniform Grid}
 * ![](org.inviwo.CreateUniformGrid.png?classIdentifier=org.inviwo.CreateUniformGrid)
 * Create a uniform grid in up to DISCRETEDATA_MAX_NUM_DIMENSIONS dimensions.
 *
 * ### Outports
 *   * dataSetOutport_ A new DataSet based on a uniform grid.
 *
 * ### Properties
 *   * dimensions_ List of dimensions.
 */
class IVW_MODULE_DISCRETEDATA_API CreateUniformGrid : public Processor {
public:
    CreateUniformGrid();
    virtual ~CreateUniformGrid() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetOutport dataSetOutport_;
    ListProperty dimensions_;
    StringProperty name_;
};

class DimensionProperty : public CompositeProperty {
public:
    DimensionProperty(std::string identifier, std::string displayName);

    DimensionProperty(const DimensionProperty& rhs);
    virtual DimensionProperty* clone() const override;
    virtual ~DimensionProperty() = default;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    IntSizeTProperty numCells_;
    DoubleProperty cellSize_;
    DoubleProperty gridOffset_;
};

namespace detail {

struct IVW_MODULE_DISCRETEDATA_API CreateUniformGridDispatcher {

    template <typename Result, int N>
    Result operator()(const std::vector<ind>& numCells, const std::vector<double>& cellSizes,
                      const std::vector<double>& gridOffsets, const std::string& name) {
        // using TransformMat = Matrix<N + 1, double>;
        // using SizeVec = glm::vec<N, ind>;

        std::array<ind, N> numVertices;
        for (ind n = 0; n < N; ++n) numVertices[n] = numCells[n];
        ind product =
            std::accumulate(numVertices.begin(), numVertices.end(), 1, std::multiplies<ind>());
        // Matrix<N + 1, double> matrix;
        // glm::vec<N, ind> size;

        // matrix[N][N] = 1;
        // for (size_t n = 0; n < N; ++n) {
        //     matrix[n][n] = cellSizes[n];
        //     // matrix[N][0] = gridOffsets[n];
        //     size[n] = numCells[n];
        // }

        // CurvilinearPositions<double, N> func(matrix, size);

        // auto channel = std::make_shared<AnalyticChannel<double, N, std::array<double, N>>>(
        //     func, product, name, GridPrimitive::Vertex);

        auto channel = std::make_shared<AnalyticChannel<double, N, std::array<double, N>>>(
            [numVerts = std::move(numVertices), sizes = cellSizes, offsets = gridOffsets](
                std::array<double, N>& vec, ind idx) {
                ind remainingIdx = idx;
                for (ind n = 0; n < N; ++n) {
                    ind dimIdx = remainingIdx % numVerts[n];
                    remainingIdx /= numVerts[n];
                    vec[n] = offsets[n] + sizes[n] * dimIdx;
                }
            },
            product, name, GridPrimitive::Vertex);
        std::array<ind, N> numCellsArray;
        std::copy_n(numCells.begin(), N, numCellsArray.begin());
        auto grid = std::make_shared<StructuredGrid<static_cast<ind>(N)>>(std::move(numCellsArray));

        auto dataSet = std::make_shared<DataSet>(name, grid);
        dataSet->addChannel(channel);
        return dataSet;
    }
};
}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
