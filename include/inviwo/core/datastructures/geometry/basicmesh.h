/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

namespace inviwo {

class IVW_CORE_API BasicMesh : public Mesh {
public:
    BasicMesh();
    BasicMesh(const BasicMesh& rhs);
    BasicMesh& operator=(const BasicMesh& that);
    virtual BasicMesh* clone() const;
    virtual ~BasicMesh();
    
    void addVertex(vec3 pos, vec3 normal, vec3 texCoord, vec4 color);
    IndexBufferRAM* addIndexBuffer(GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct);

    virtual std::string getDataInfo() const;

    const Position3dBuffer* getVertices() const;
    const TexCoord3dBuffer* getTexCoords() const;
    const ColorBuffer* getColors() const;
    const NormalBuffer* getNormals() const;

    Position3dBuffer* getVertices();
    TexCoord3dBuffer* getTexCoords();
    ColorBuffer* getColors();
    NormalBuffer* getNormals();
    
    void append(const BasicMesh* mesh);
    

    static BasicMesh* disk(const vec3& center,
                           const vec3& normal,
                           const vec4& color = vec4(1.0f,0.0f,0.0f,1.0f),
                           const float& radius = 1.0f,
                           const size_t& segments=16);
    static BasicMesh* cone(const vec3& start,
                           const vec3& stop,
                           const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
                           const float& radius = 1.0f,
                           const size_t& segments=16);
    static BasicMesh* cylinder(const vec3& start,
                               const vec3& stop,
                               const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
                               const float& radius = 1.0f,
                               const size_t& segments=16);
    static BasicMesh* line(const vec3& start,
                            const vec3& stop, 
                            const vec3& normal,
                            const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
                            const float&width = 1.0f,
                            const ivec2& res = ivec2(1));
    static BasicMesh* arrow(const vec3& start,
                            const vec3& stop,
                            const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
                            const float& radius = 1.0f,
                            const float& arrowfraction = 0.15f,
                            const float& arrowRadius = 2.0f,
                            const size_t& segments=16);

    static BasicMesh* colorsphere(const vec3& center,
                                  const float& radius);

    static BasicMesh* square(const vec3& pos, const vec3& normal, const glm::vec2& extent,
                            const vec4& color = vec4(1,1,1,1), const ivec2& res = ivec2(1));


    static BasicMesh* box(const mat4& orientation, const vec4 &color = vec4(1,1,1,1));

    static BasicMesh* coordindicator(const vec3& center,
                                     const float& size);

    static BasicMesh* boundingbox(const mat4& basisandoffset, const vec4& color);

protected:
    static vec3 orthvec(const vec3& vec);
    static vec3 calcnormal(const vec3& r, const vec3& p);
    static vec3 tospherical(const vec2& v);
};

} // namespace

#endif // IVW_BASICMESH_H

