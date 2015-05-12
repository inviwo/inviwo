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
#include <utility>

namespace inviwo {

class IVW_CORE_API Mesh : public DataGroup, public SpatialEntity<3> {
public:
    struct AttributesInfo {
        GeometryEnums::DrawType dt;
        GeometryEnums::ConnectivityType ct;
        AttributesInfo() : dt(GeometryEnums::POINTS), ct(GeometryEnums::NONE) {}
        AttributesInfo(GeometryEnums::DrawType d, GeometryEnums::ConnectivityType c)
            : dt(d), ct(c) {}
    };

    typedef std::vector<std::pair<AttributesInfo, IndexBuffer*> > IndexVector;

    Mesh();
    Mesh(GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct);

    Mesh(const Mesh& rhs);
    Mesh& operator=(const Mesh& that);
    virtual Mesh* clone() const;
    virtual ~Mesh();
    virtual std::string getDataInfo() const;

    virtual void performOperation(DataOperation*) const {};

    /**
     * Add a buffer with rendering data, such as positions/colors/normals.
     *
     * @param att Data to be rendered.
     * @param takeOwnership True if the buffer should be deleted by the mesh.
     */
    void addAttribute(Buffer* att, bool takeOwnership = true);

    /**
     * Replaces buffer at index with new buffer and deletes old one if it has ownership of it.
     * Does nothing if index out of range.
     * @param size_t idx Index of buffer to replace
     * @param Buffer * att New buffer
     * @param bool takeOwnership True if new buffer should be owned.
     */
    void setAttribute(size_t idx, Buffer* att, bool takeOwnership = true);

    /**
     * Add index buffer. The indices will be used as look up
     * values into the buffers during rendering.
     * The Mesh will take ownership of the added buffer.
     * @param info Rendering type and connectivity.
     * @param ind Index buffer, will be owned by mesh.
     */
    void addIndicies(AttributesInfo info, IndexBuffer* ind);

    const std::vector<Buffer*>& getBuffers() const;
    const IndexVector& getIndexBuffers() const;

    const Buffer* getAttributes(size_t idx) const;
    const Buffer* getIndicies(size_t idx) const;

    Buffer* getAttributes(size_t idx);
    Buffer* getIndicies(size_t idx);

    AttributesInfo getDefaultAttributesInfo() const;
    AttributesInfo getIndexAttributesInfo(size_t idx) const;

    size_t getNumberOfAttributes() const;
    size_t getNumberOfIndicies() const;

    virtual const SpatialCameraCoordinateTransformer<3>& getCoordinateTransformer(
        const CameraProperty* camera) const;
    using SpatialEntity<3>::getCoordinateTransformer;

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

protected:
    std::vector<Buffer*> attributes_;
    std::vector<bool>
        attributesOwnership_;  // Indicates if the Mesh owns the corresponding Buffer in attributes_
    IndexVector indexAttributes_;
    AttributesInfo defaultAttributeInfo_;

    void deinitialize();
};

}  // namespace

#endif  // IVW_MESH_H
