/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_BASICMESH_H
#define IVW_BASICMESH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <tuple>

namespace inviwo {

/**
 * \ingroup datastructures	
 */
class IVW_CORE_API BasicMesh : public Mesh {
public:
    BasicMesh();
    BasicMesh(const BasicMesh& rhs);
    BasicMesh& operator=(const BasicMesh& that);
    virtual BasicMesh* clone() const override;
    virtual ~BasicMesh() = default;

    /**
     * Adds a new vertex to the Mesh, returns the index of the new vertex
     * @param pos position of the new vertex
     * @param normal normal at the new vertex
     * @param texCoord texture coordinates at the new vertex
     * @param color color (RGBA) at the new vertex
     */
    uint32_t addVertex(vec3 pos, vec3 normal, vec3 texCoord, vec4 color);

    // add a list of verties: {{pos, normal, tex, color}, ...}
    struct Vertex {
        vec3 pos;
        vec3 normal;
        vec3 tex;
        vec4 color;
    };
    void addVertices(const std::vector<Vertex> &data);

    void setVertex(size_t index, vec3 pos, vec3 normal, vec3 texCoord, vec4 color);
    void setVertexPosition(size_t index, vec3 pos);
    void setVertexNormal(size_t index, vec3 normal);
    void setVertexTexCoord(size_t index, vec3 texCoord);
    void setVertexColor(size_t index, vec4 color);
    IndexBufferRAM* addIndexBuffer(DrawType dt, ConnectivityType ct);

    const Buffer<vec3>* getVertices() const;
    const Buffer<vec3>* getTexCoords() const;
    const Buffer<vec4>* getColors() const;
    const Buffer<vec3>* getNormals() const;

    Buffer<vec3>* getEditableVertices();
    Buffer<vec3>* getEditableTexCoords();
    Buffer<vec4>* getEditableColors();
    Buffer<vec3>* getEditableNormals();

    void append(const BasicMesh* mesh);

protected:

    const Vec3BufferRAM* getVerticesRAM() const;
    const Vec3BufferRAM* getTexCoordsRAM() const;
    const Vec4BufferRAM* getColorsRAM() const;
    const Vec3BufferRAM* getNormalsRAM() const;

    Vec3BufferRAM* getEditableVerticesRAM();
    Vec3BufferRAM* getEditableTexCoordsRAM();
    Vec4BufferRAM* getEditableColorsRAM();
    Vec3BufferRAM* getEditableNormalsRAM();

    std::shared_ptr<Buffer<vec3>> vertices_;
    std::shared_ptr<Buffer<vec3>> texCoords_;
    std::shared_ptr<Buffer<vec4>> colors_;
    std::shared_ptr<Buffer<vec3>> normals_;
};

}  // namespace

#endif  // IVW_BASICMESH_H
