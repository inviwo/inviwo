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

#ifndef IVW_MESH_H
#define IVW_MESH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datagroup.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/meshrepresentation.h>
#include <utility>

namespace inviwo {

class IVW_CORE_API Mesh : public DataGroup<MeshRepresentation>, public SpatialEntity<3> {
public:
    struct MeshInfo {
        MeshInfo() : dt(DrawType::Points), ct(ConnectivityType::None) {}
        MeshInfo(DrawType d, ConnectivityType c) : dt(d), ct(c) {}
        DrawType dt;
        ConnectivityType ct;
    };

    using IndexVector = std::vector<std::pair<MeshInfo, std::shared_ptr<IndexBuffer>>>;
    using BufferVector = std::vector<std::pair<BufferType, std::shared_ptr<BufferBase>>>;

    Mesh() = default;
    Mesh(DrawType dt, ConnectivityType ct);

    Mesh(const Mesh& rhs);
    Mesh& operator=(const Mesh& that);
    virtual Mesh* clone() const;
    virtual ~Mesh() = default;
    virtual std::string getDataInfo() const;

    /**
     * Add a buffer with rendering data, such as positions/colors/normals.
     *
     * @param att Data to be rendered.
     * @param takeOwnership True if the buffer should be deleted by the mesh.
     */
    void addBuffer(BufferType type, std::shared_ptr<BufferBase> att);

    /**
     * Replaces buffer at index with new buffer and deletes old one if it has ownership of it.
     * Does nothing if index out of range.
     * @param idx Index of buffer to replace
     * @param att New buffer
     * @param takeOwnership True if new buffer should be owned.
     */
    void setBuffer(size_t idx, BufferType type, std::shared_ptr<BufferBase> att);

    /**
     * Add index buffer. The indices will be used as look up
     * values into the buffers during rendering.
     * The Mesh will take ownership of the added buffer.
     * @param info Rendering type and connectivity.
     * @param ind Index buffer, will be owned by mesh.
     */
    void addIndicies(MeshInfo info, std::shared_ptr<IndexBuffer> ind);

    const BufferVector& getBuffers() const;
    const IndexVector& getIndexBuffers() const;

    const BufferBase* getBuffer(size_t idx) const;
    const IndexBuffer* getIndicies(size_t idx) const;

    BufferBase* getBuffer(size_t idx);
    IndexBuffer* getIndicies(size_t idx);

    MeshInfo getDefaultMeshInfo() const;
    MeshInfo getIndexMeshInfo(size_t idx) const;

    size_t getNumberOfBuffers() const;
    size_t getNumberOfIndicies() const;

    virtual const SpatialCameraCoordinateTransformer<3>& getCoordinateTransformer(
        const Camera& camera) const;
    using SpatialEntity<3>::getCoordinateTransformer;

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

protected:
    BufferVector buffers_;
    IndexVector indices_;
    MeshInfo defaultMeshInfo_;
};

}  // namespace

#endif  // IVW_MESH_H
