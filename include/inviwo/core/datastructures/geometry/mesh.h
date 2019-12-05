/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/document.h>
#include <utility>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API Mesh : public DataGroup<Mesh, MeshRepresentation>,
                          public SpatialEntity<3>,
                          public MetaDataOwner {
public:
    struct MeshInfo {
        MeshInfo() : dt(DrawType::Points), ct(ConnectivityType::None) {}
        MeshInfo(DrawType d, ConnectivityType c) : dt(d), ct(c) {}
        DrawType dt;
        ConnectivityType ct;
    };
    struct BufferInfo {
        BufferInfo(BufferType atype) : type(atype), location(static_cast<int>(atype)) {}
        BufferInfo() : BufferInfo(BufferType::PositionAttrib) {}
        BufferInfo(BufferType atype, int alocation) : type(atype), location(alocation) {}

        BufferType type;
        int location;  //<! attribute location of buffer in GLSL shader
    };

    using IndexVector = std::vector<std::pair<MeshInfo, std::shared_ptr<IndexBuffer>>>;
    using BufferVector = std::vector<std::pair<BufferInfo, std::shared_ptr<BufferBase>>>;

    Mesh() = default;
    Mesh(DrawType dt, ConnectivityType ct);
    Mesh(Mesh::MeshInfo meshInfo);

    Mesh(const Mesh& rhs);
    Mesh& operator=(const Mesh& that);
    virtual Mesh* clone() const;
    virtual ~Mesh() = default;
    virtual Document getInfo() const;

    /**
     * Add a buffer with rendering data, such as positions/colors/normals.
     *
     * @param info  information about the buffer contents (e.g. buffer type and attrib location)
     * @param att   buffer data used during rendering
     */
    void addBuffer(BufferInfo info, std::shared_ptr<BufferBase> att);

    /**
     * Add a buffer with rendering data, such as positions/colors/normals, and associate it with its
     * default attrib location.
     *
     * @param type  buffer type (Position, Color, Normal, etc.)
     * @param att   buffer data used during rendering
     */
    void addBuffer(BufferType type, std::shared_ptr<BufferBase> att);

    /**
     * Removes buffer at given position, all subsequent buffers will be moved.
     * Does nothing if index is out of range.
     *
     * @param idx   position of buffer to be removed
     */
    void removeBuffer(size_t idx);

    /**
     * Replaces buffer at index with new buffer
     * Does nothing if index out of range.
     * @param idx   Index of buffer to replace
     * @param info  information about the buffer contents (e.g. buffer type and shader location)
     * @param att   new buffer data used during rendering
     */
    void replaceBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> att);

    /**
     * Deprecated: Mesh::setBuffer() has been renamed to Mesh::replaceBuffer()
     *
     * @param idx   Index of buffer to replace
     * @param info  information about the buffer contents (e.g. buffer type and shader location)
     * @param att   new buffer data used during rendering
     *
     * \see replaceBuffer
     */
    void setBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> att);

    /**
     * Add index buffer. The indices will be used as look up
     * values into the buffers during rendering.
     * The Mesh will take ownership of the added buffer.
     * @param info Rendering type and connectivity.
     * @param ind Index buffer, will be owned by mesh.
     */
    void addIndices(MeshInfo info, std::shared_ptr<IndexBuffer> ind);

    // clang-format off
    [[deprecated("Mesh::addIndicies is deprecated, use addIndices (deprecated since 2019-12-04)")]]
    void addIndicies(MeshInfo info, std::shared_ptr<IndexBuffer> ind) { addIndices(info, ind); }
    // clang-format on

    /**
     * Creates and add a new index buffer to the mesh
     *
     * @see addIndicies
     * @param dt DrawType of the new buffer
     * @param ct ConnectivityType of the new buffer
     * @return returns the RAM Representation of the new buffer.
     */
    std::shared_ptr<IndexBufferRAM> addIndexBuffer(DrawType dt, ConnectivityType ct);

    /**
     * Removes index buffer at given position, all subsequent index buffers will be moved.
     * Does nothing if index is out of range.
     *
     * @param idx   position of index buffer to be removed
     */
    void removeIndexBuffer(size_t idx);

    /**
     * Reserve memory for a given number of vertices in each buffer.
     * @param size number of elements that will be reserved in each buffer
     */
    void reserveSizeInVertexBuffer(size_t size);

    /**
     * Reserve memory for a given number of index buffers.
     * Useful when having a mesh on which we will add a lot of index buffers
     * @param size the new reserved size of the vector containing index buffers
     */
    void reserveIndexBuffers(size_t size);

    const BufferVector& getBuffers() const;
    const IndexVector& getIndexBuffers() const;

    const BufferBase* getBuffer(size_t idx) const;
    BufferInfo getBufferInfo(size_t idx) const;
    const IndexBuffer* getIndices(size_t idx) const;

    /**
     * Try and find a buffer of the given BufferType.
     * Returns the buffer and its location.
     * If no buffer is found the buffer will be a nullptr.
     */
    std::pair<const BufferBase*, int> findBuffer(BufferType type) const;

    /**
     * Check if there exits a buffer of BufferType type in the mesh.
     */
    bool hasBuffer(BufferType type) const;

    BufferBase* getBuffer(size_t idx);

    /**
     * @return The first buffer matching BufferType type or nullptr;
     */
    BufferBase* getBuffer(BufferType type);

    IndexBuffer* getIndices(size_t idx);

    MeshInfo getDefaultMeshInfo() const;
    MeshInfo getIndexMeshInfo(size_t idx) const;

    size_t getNumberOfBuffers() const;
    size_t getNumberOfIndicies() const;

    /**
     * \brief Append another mesh to this mesh
     *
     * Requires that both meshes has the same type and number of vertex buffers.
     * Will append the contents of each vertex buffer in mesh to each vertex in this.
     * Index buffers are copied and each index is incremented with an offset.
     *
     * @param mesh The mesh to copy values from
     */
    void append(const Mesh& mesh);

    virtual const SpatialCameraCoordinateTransformer<3>& getCoordinateTransformer(
        const Camera& camera) const;
    using SpatialEntity<3>::getCoordinateTransformer;

    static uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

protected:
    BufferVector buffers_;
    IndexVector indices_;
    MeshInfo meshInfo_;
};

inline bool operator==(const Mesh::BufferInfo& a, const Mesh::BufferInfo& b) {
    return (a.type == b.type) && (a.location == b.location);
}
inline bool operator!=(const Mesh::BufferInfo& a, const Mesh::BufferInfo& b) { return !(a == b); }

inline bool operator==(const Mesh::MeshInfo& a, const Mesh::MeshInfo& b) {
    return (a.ct == b.ct) && (a.dt == b.dt);
}
inline bool operator!=(const Mesh::MeshInfo& a, const Mesh::MeshInfo& b) { return !(a == b); }

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             Mesh::BufferInfo info) {
    ss << info.type << " (location = " << info.location << ")";
    return ss;
}

namespace meshutil {

IVW_CORE_API bool hasPickIDBuffer(const Mesh* mesh);

IVW_CORE_API bool hasRadiiBuffer(const Mesh* mesh);

}  // namespace meshutil

// https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations?view=vs-2017
extern template class IVW_CORE_TMPL_EXP DataReaderType<Mesh>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Mesh>;

}  // namespace inviwo

#endif  // IVW_MESH_H
