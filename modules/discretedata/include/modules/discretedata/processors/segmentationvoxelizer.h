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
#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/discretedata/connectivity/connectioniterator.h>
#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/channeldispatching.h>

namespace inviwo {
namespace discretedata {
class IVW_MODULE_DISCRETEDATA_API SegmentationVoxelizer : public Processor {
public:
    SegmentationVoxelizer();
    virtual ~SegmentationVoxelizer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    template <typename T, ind N>
    Mesh createVoxels(Mesh& voxels, const Connectivity& grid, const DataChannel<T, N>& segmentation,
                      const Channel& vertices);

private:
    template <typename Ts, ind Ns, typename Tv, ind Nv>
    Mesh createVoxelsFullTemplate(Mesh& voxels, const Connectivity& grid,
                                  const DataChannel<Ts, Ns>& segmentation,
                                  const DataChannel<Tv, Nv>& vertices);

private:
    /** DataSet with segmented channel & vertex positions */
    DataSetInport portInData_;

    /** Mesh of voxels */
    MeshOutport portOutMesh_;

    /** Selection of channel with IDs */
    DataChannelProperty segmentationChannel_;

    /** Selection of channel with vertex positions */
    DataChannelProperty vertexChannel_;
};

template <typename T, ind N>  // TODO: Add a id-> color function.
Mesh SegmentationVoxelizer::createVoxels(Mesh& voxels, const Connectivity& grid,
                                         const DataChannel<T, N>& segmentation,
                                         const Channel& vertices) {
    static_assert(N > 0);
    return vertices.dispatch<Mesh, dispatching::filter::Scalars, 2, 4>(
        [&](const auto* vertDataChannel) {
            return createVoxelsFullTemplate(voxels, grid, segmentation, *vertDataChannel);
        });
}

template <typename Ts, ind Ns, typename Tv, ind Nv>
Mesh SegmentationVoxelizer::createVoxelsFullTemplate(Mesh& voxels, const Connectivity& grid,
                                                     const DataChannel<Ts, Ns>& segmentation,
                                                     const DataChannel<Tv, Nv>& vertices) {
    IVW_ASSERT(grid.getDimension() == GridPrimitive::Volume, "Not a voxel grid.");
    IVW_ASSERT(segmentation.getGridPrimitiveType() == GridPrimitive::Volume,
               "Segmentation channel not defined on the volume.");
    IVW_ASSERT(vertices.getGridPrimitiveType() == GridPrimitive::Vertex,
               "Vertex position channel not defined on the vertices.");
    IVW_ASSERT(vertices.getNumComponents() >= 2 || vertices.getNumComponents() <= 4,
               "Vertices are not 2/3/4D coordinates.");
    // // Triangle mesh without any additional connectivity information.
    // Mesh voxels = Mesh(DrawType::Triangles, ConnectivityType::None);

    auto vertRam = std::make_shared<Vec3BufferRAM>();
    auto vertBuffer = std::make_shared<Buffer<vec3>>(vertRam);
    voxels.addBuffer(BufferType::PositionAttrib, vertBuffer);

    auto idxBuffer = voxels.addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    auto colorRam = std::make_shared<Vec3BufferRAM>();
    auto colorBuffer = std::make_shared<Buffer<vec3>>(vertRam);
    voxels.addBuffer(BufferType::ColorAttrib, colorBuffer);
    // Not adding normals, since those would just be face normals.

    std::ofstream file;  //////////////////////////////////////////////////////////////////////
    file.open("/home/anke/Desktop/debug.log",
              std::ios::out);  ////////////////////////////////////////

    std::vector<ind> neighbors;
    for (auto face : grid.all(GridPrimitive::Face)) {

        grid.getConnections(neighbors, face.getIndex(), GridPrimitive::Face, GridPrimitive::Volume);

        typename DataChannel<Ts, Ns>::DefaultVec segVal0, segVal1;
        segmentation(segVal0, neighbors[0]);
        if (neighbors.size() > 1) segmentation(segVal1, neighbors[1]);
        if (neighbors.size() < 2 || segVal0 != segVal1) {
            IndexBufferRAM::type startIdx = vertRam->getSize();

            file << "Face " << face.getIndex() << '\n';
            file << "\tVoxel 0: " << neighbors[0] << '\n';
            if (neighbors.size() > 1) file << "\tVoxel 1: " << neighbors[1] << '\n';

            for (auto vertex : face.connection(GridPrimitive::Vertex)) {
                std::array<Tv, Nv> vertVal;
                vec3 vertF3;
                vertices(vertVal, vertex.getIndex());
                for (ind d = 0; d < std::min(ind(3), Nv); ++d) {
                    file << vertVal[d] << ", ";  /////////////////////////////////////////
                    vertF3[d] = vertVal[d];
                }
                file << '\n';

                vertRam->add(vertF3);
                colorRam->add(vec3(1, 0, 0));
            }
            idxBuffer->add({
                startIdx, startIdx + 1, startIdx + 2,     // First triangle
                startIdx + 2, startIdx + 1, startIdx + 3  // Second triangle
            });
        }
    }
    return voxels;
}  // namespace discretedata

}  // namespace discretedata
}  // namespace inviwo