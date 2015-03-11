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

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>


namespace inviwo {
BasicMesh::BasicMesh()
    : Mesh() {
    addAttribute(new Position3dBuffer()); // pos 0
    addAttribute(new TexCoord3dBuffer()); // pos 1
    addAttribute(new ColorBuffer());      // pos 2
    addAttribute(new NormalBuffer());     // pos 3 
}

BasicMesh::BasicMesh(const BasicMesh& rhs) 
    : Mesh(rhs) {}

BasicMesh& BasicMesh::operator=(const BasicMesh& that) {
    if (this != &that) {
        Mesh::operator=(that);
    }
    return *this;
}

BasicMesh* BasicMesh::clone() const {
    return new BasicMesh(*this);
}

BasicMesh::~BasicMesh() {
}

void BasicMesh::addVertex(vec3 pos, vec3 normal, vec3 texCoord, vec4 color) {
    static_cast<Position3dBuffer*>(attributes_[0])
        ->getEditableRepresentation<Position3dBufferRAM>()
        ->add(pos);
    static_cast<TexCoord3dBuffer*>(attributes_[1])
        ->getEditableRepresentation<TexCoord3dBufferRAM>()
        ->add(texCoord);
    static_cast<ColorBuffer*>(attributes_[2])->getEditableRepresentation<ColorBufferRAM>()->add(
        color);
    static_cast<NormalBuffer*>(attributes_[3])->getEditableRepresentation<NormalBufferRAM>()->add(
        normal);
}

IndexBufferRAM* BasicMesh::addIndexBuffer(GeometryEnums::DrawType dt,
                                          GeometryEnums::ConnectivityType ct) {
    IndexBuffer* indices_ = new IndexBuffer();
    addIndicies(Mesh::AttributesInfo(dt, ct), indices_);
    return indices_->getEditableRepresentation<IndexBufferRAM>();
}

std::string BasicMesh::getDataInfo() const {
    std::stringstream ss;
    ss  << "<table border='0' cellspacing='0' cellpadding='0' style='border-color:white;white-space:pre;'>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Type</td><td><nobr>Basic mesh</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Vertiecs</td><td><nobr>" << attributes_[0]->getSize() << "</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Normal</td><td><nobr>" << attributes_[3]->getSize() << "</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Texture Coords</td><td><nobr>" << attributes_[1]->getSize() << "</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Colors</td><td><nobr>" << attributes_[2]->getSize() << "</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Index buffers</td><td><nobr>" << getNumberOfIndicies() << "</nobr></td></tr>\n"
        << "</tr></table>\n";

    return ss.str();
}

void BasicMesh::append(const BasicMesh* mesh) {
    const Position3dBufferRAM* pos = mesh->getVertices()->getRepresentation<Position3dBufferRAM>();
    const NormalBufferRAM* norm = mesh->getNormals()->getRepresentation<NormalBufferRAM>();
    const TexCoord3dBufferRAM* tex = mesh->getTexCoords()->getRepresentation<TexCoord3dBufferRAM>();
    const ColorBufferRAM* col = mesh->getColors()->getRepresentation<ColorBufferRAM>();

    size_t size = attributes_[0]->getSize();

    static_cast<Position3dBuffer*>(attributes_[0])
        ->getEditableRepresentation<Position3dBufferRAM>()
        ->append(pos->getDataContainer());
    static_cast<TexCoord3dBuffer*>(attributes_[1])
        ->getEditableRepresentation<TexCoord3dBufferRAM>()
        ->append(tex->getDataContainer());
    static_cast<ColorBuffer*>(attributes_[2])->getEditableRepresentation<ColorBufferRAM>()->append(
        col->getDataContainer());
    static_cast<NormalBuffer*>(attributes_[3])
        ->getEditableRepresentation<NormalBufferRAM>()
        ->append(norm->getDataContainer());

    for (auto buffer : mesh->indexAttributes_) {
        IndexBufferRAM* ind = addIndexBuffer(buffer.first.dt, buffer.first.ct);

        const std::vector<unsigned int>* newinds =
            buffer.second->getRepresentation<IndexBufferRAM>()->getDataContainer();

        for (const auto& newind : *newinds) {
            ind->add(static_cast<const unsigned int>(newind + size));
        }
    }
}

const Position3dBuffer* BasicMesh::getVertices() const {
    return static_cast<Position3dBuffer*>(attributes_[0]);
}

Position3dBuffer* BasicMesh::getVertices() {
	return static_cast<Position3dBuffer*>(attributes_[0]);
}

const TexCoord3dBuffer* BasicMesh::getTexCoords() const {
    return static_cast<TexCoord3dBuffer*>(attributes_[1]);
}

TexCoord3dBuffer* BasicMesh::getTexCoords() {
	return static_cast<TexCoord3dBuffer*>(attributes_[1]);
}

const ColorBuffer* BasicMesh::getColors() const {
    return static_cast<ColorBuffer*>(attributes_[2]);
}

ColorBuffer* BasicMesh::getColors() {
	return static_cast<ColorBuffer*>(attributes_[2]);
}

const NormalBuffer* BasicMesh::getNormals() const {
    return static_cast<NormalBuffer*>(attributes_[3]);
}

NormalBuffer* BasicMesh::getNormals() {
	return static_cast<NormalBuffer*>(attributes_[3]);
}
    
    
vec3 BasicMesh::orthvec(const vec3& vec){
    vec3 u(1.0f, 0.0f, 0.0f);
    vec3 n = glm::normalize(vec);
    float p = glm::dot(u, n);
    if (std::abs(p) != 1.0f) {
        return glm::normalize(u - p*n);
    }else{
        return vec3(0.0f,1.0f,0.0f);
    }
}

vec3 BasicMesh::calcnormal(const vec3& r, const vec3& p) {
    return glm::normalize(r + glm::dot(r, r - p) / glm::dot(p - r, p - r)* (p-r));
}

vec3 BasicMesh::tospherical(const vec2& v) {
    return vec3(std::sin(v.x)*std::cos(v.y), std::sin(v.x)*std::sin(v.y), std::cos(v.x));
}

BasicMesh* BasicMesh::disk(const vec3& center,
                           const vec3& normal,
                           const vec4& color,
                           const float& radius,
                           const size_t& segments) {

    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);
    vec3 orth = orthvec(normal);
    
    mesh->addVertex(center, normal, vec3(0.5f,0.5f,0.0f), color);
    
    vec3 tc = vec3(0.5f,0.5f,0.0f);
    vec3 tn = vec3(0.0f,0.0f,1.0f);
    vec3 to = vec3(0.5f,0.0f,0.0f);
    vec3 p;
    vec3 t;
    double angle = 2.0*M_PI / segments;
    for (size_t i = 0; i < segments; ++i) {
        p = center + radius * glm::rotate(orth, static_cast<float>(i * angle), normal);
        t = tc + glm::rotate(to,static_cast<float>(i * angle), tn);
        mesh->addVertex(p, normal, t, color);
        inds->add(0);
        inds->add(static_cast<const unsigned int>(1+i));
        inds->add(static_cast<const unsigned int>(1+( (i+1) % segments)));
    }
    return mesh;
}
    
BasicMesh* BasicMesh::cone(const vec3& start,
                           const vec3& stop,
                           const vec4& color,
                           const float& radius,
                           const size_t& segments) {
    vec3 tc = vec3(0.5f, 0.5f, 0.0f);
    vec3 tn = vec3(0.0f, 0.0f, 1.0f);
    vec3 to = vec3(0.5f, 0.0f, 0.0f);

    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);
    vec3 normal = glm::normalize(stop-start);
    vec3 orth = orthvec(normal);
    double angle = 2.0*M_PI / segments;
    for (size_t i = 0; i < segments; ++i) {
        mesh->addVertex(stop, 
                        calcnormal(glm::rotate(orth, static_cast<float>((i + 0.5) * angle), normal), stop - start),
                        tc, 
                        color);

        mesh->addVertex(start + radius*glm::rotate(orth, static_cast<float>(i * angle), normal),
                        calcnormal(glm::rotate(orth, static_cast<float>(i * angle), normal), stop - start),
                        tc + to*glm::rotate(orth, static_cast<float>(i * angle), tn),
                        color);

        mesh->addVertex(start + radius*glm::rotate(orth, static_cast<float>((i+1.0) * angle), normal),
                        calcnormal(glm::rotate(orth, static_cast<float>((i+1.0) * angle), normal), stop - start),
                        tc + to*glm::rotate(orth, static_cast<float>((i+1.0) * angle), tn),
                        color);

        inds->add(static_cast<const unsigned int>(i * 3 + 0));
        inds->add(static_cast<const unsigned int>(i * 3 + 1));
        inds->add(static_cast<const unsigned int>(i * 3 + 2));
    }

    return mesh;
}

BasicMesh* BasicMesh::cylinder(const vec3& start,
                               const vec3& stop,
                               const vec4& color,
                               const float& radius,
                               const size_t& segments) {
    
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
      
    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);
    vec3 normal = glm::normalize(stop-start);
    vec3 orth = orthvec(normal);
    vec3 o;                                   
    double angle = 2.0*M_PI / segments;
    float j;
    
    for (size_t i = 0; i < segments; ++i) {
        j = static_cast<float>(i);
        o = glm::rotate(orth, static_cast<float>(i * angle), normal);
        mesh->addVertex(start + radius*o, o, vec3(j/segments,0.0f,0.0f), color);
        mesh->addVertex(stop + radius*o, o, vec3(j/segments,1.0f,0.0f), color);

        inds->add(static_cast<const unsigned int>(i * 2 + 1));
        inds->add(static_cast<const unsigned int>(i * 2 + 0));
        inds->add(static_cast<const unsigned int>(((i + 1) * 2) % (2*segments) + 0));

        inds->add(static_cast<const unsigned int>(i * 2 + 1));
        inds->add(static_cast<const unsigned int>(((i + 1) * 2) % (2*segments) + 0));
        inds->add(static_cast<const unsigned int>(((i + 1) * 2) % (2*segments) + 1));
    }
    //inds->add(static_cast<const unsigned int>(1));
    //inds->add(static_cast<const unsigned int>(0));
    
    BasicMesh* startcap = disk(start, -normal, color, radius, segments);
    BasicMesh* endcap = disk(stop, normal, color, radius, segments);

    mesh->append(startcap);
    mesh->append(endcap);

    delete startcap;
    delete endcap;
    
    return mesh;
}

    
BasicMesh* BasicMesh::line(const vec3& start, const vec3& stop, const vec3& normal, const vec4& color /*= vec4(1.0f, 0.0f, 0.0f, 1.0f)*/, const float&width /*= 1.0f*/, const ivec2& inres /*= ivec2(1)*/) {
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);

    vec3 right = orthvec(normal);
    vec3 up = glm::cross(right, normal);
    vec3 direction = stop - start;

    vec3 startCornerPoint = start - 0.5f * width * up;
    ivec2 res = inres + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(
                startCornerPoint + static_cast<float>(i) / static_cast<float>(inres.x) * direction +
                static_cast<float>(j) / static_cast<float>(inres.y) * width * up,
                normal, vec3(static_cast<float>(i) / static_cast<float>(inres.x),
                static_cast<float>(j) / static_cast<float>(inres.y), 0.0f),
                color);

            if (i != inres.x && j != inres.y) {
                inds->add(i + res.x * j);
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * (j + 1));

                inds->add(i + 1 + res.x * j);
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + res.x * (j + 1));
            }
        }
    }

    return mesh;
}

BasicMesh* BasicMesh::arrow(const vec3& start,
                            const vec3& stop,
                            const vec4& color,
                            const float& radius,
                            const float& arrowfraction,
                            const float& arrowRadius,
                            const size_t& segments) {
    
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));

    vec3 mid = start + (1-arrowfraction)*(stop-start);
    BasicMesh* cylinderpart = cylinder(start, mid, color, radius, segments);
    mesh->append(cylinderpart);
    delete cylinderpart;
    
    BasicMesh* diskpart = disk(mid, start-mid, color, arrowRadius, segments);
    mesh->append(diskpart);
    delete diskpart;
    
    BasicMesh* conepart = cone(mid, stop, color, arrowRadius, segments);
    mesh->append(conepart);
    delete conepart;
    
    return mesh;
}
    
BasicMesh* BasicMesh::colorsphere(const vec3& center,
                                  const float& radius) {
     
    std::vector<vec2> spheremesh;
    spheremesh.push_back(vec2(M_PI_2, 0.0f));
    spheremesh.push_back(vec2(M_PI_2, M_PI_2));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(std::atan(6.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), 0.0f));
    spheremesh.push_back(vec2(M_PI_2, std::atan(1.0f / 6.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(2.0f) / 5.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(2.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f*std::sqrt(5.0f)), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan((2.0f*std::sqrt(2.0f)) / 3.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(3.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f*std::sqrt(2.0f)), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(4.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f*std::sqrt(5.0f)), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(5.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(5.0f)));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(6.0f)));
    spheremesh.push_back(vec2(std::atan(6.0f), M_PI_2));

    std::vector<uvec3> sphereind;
    sphereind.push_back(uvec3(0, 10, 4));
    sphereind.push_back(uvec3(4, 11, 5));
    sphereind.push_back(uvec3(5, 12, 6));
    sphereind.push_back(uvec3(6, 13, 7));
    sphereind.push_back(uvec3(7, 14, 8));
    sphereind.push_back(uvec3(8, 15, 9));
    sphereind.push_back(uvec3(9, 16, 2));
    sphereind.push_back(uvec3(10, 17, 11));
    sphereind.push_back(uvec3(11, 18, 12));
    sphereind.push_back(uvec3(12, 19, 13));
    sphereind.push_back(uvec3(13, 20, 14));
    sphereind.push_back(uvec3(14, 21, 15));
    sphereind.push_back(uvec3(15, 22, 16));
    sphereind.push_back(uvec3(17, 23, 18));
    sphereind.push_back(uvec3(18, 24, 19));
    sphereind.push_back(uvec3(19, 25, 20));
    sphereind.push_back(uvec3(20, 26, 21));
    sphereind.push_back(uvec3(21, 27, 22));
    sphereind.push_back(uvec3(23, 28, 24));
    sphereind.push_back(uvec3(24, 29, 25));
    sphereind.push_back(uvec3(25, 30, 26));
    sphereind.push_back(uvec3(26, 31, 27));
    sphereind.push_back(uvec3(28, 32, 29));
    sphereind.push_back(uvec3(29, 33, 30));
    sphereind.push_back(uvec3(30, 34, 31));
    sphereind.push_back(uvec3(32, 35, 33));
    sphereind.push_back(uvec3(33, 36, 34));
    sphereind.push_back(uvec3(35, 1, 36));
    sphereind.push_back(uvec3(11, 4, 10));
    sphereind.push_back(uvec3(12, 5, 11));
    sphereind.push_back(uvec3(13, 6, 12));
    sphereind.push_back(uvec3(14, 7, 13));
    sphereind.push_back(uvec3(15, 8, 14));
    sphereind.push_back(uvec3(16, 9, 15));
    sphereind.push_back(uvec3(18, 11, 17));
    sphereind.push_back(uvec3(19, 12, 18));
    sphereind.push_back(uvec3(20, 13, 19));
    sphereind.push_back(uvec3(21, 14, 20));
    sphereind.push_back(uvec3(22, 15, 21));
    sphereind.push_back(uvec3(24, 18, 23));
    sphereind.push_back(uvec3(25, 19, 24));
    sphereind.push_back(uvec3(26, 20, 25));
    sphereind.push_back(uvec3(27, 21, 26));
    sphereind.push_back(uvec3(29, 24, 28));
    sphereind.push_back(uvec3(30, 25, 29));
    sphereind.push_back(uvec3(31, 26, 30));
    sphereind.push_back(uvec3(33, 29, 32));
    sphereind.push_back(uvec3(34, 30, 33));
    sphereind.push_back(uvec3(36, 33, 35));


    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                BasicMesh* temp = new BasicMesh();
                IndexBufferRAM* inds = temp->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                vec4 color((quad + 1.0f) / 2.0f, 1.0f);
                for (auto& elem : spheremesh) {
                    normal = quad * tospherical(elem);
                    vertex = center + radius*normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    temp->addVertex(vertex, normal, tcoord, color);
                }

                if (quad.x*quad.y*quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add(elem.x);
                        inds->add(elem.y);
                        inds->add(elem.z);
                    }
                } else {
                    for (auto& elem : sphereind) {
                        inds->add(elem.z);
                        inds->add(elem.y);
                        inds->add(elem.x);
                    }
                }
                mesh->append(temp);
                delete temp;
            }
        }
    } 
    return mesh;
}

static vec3 V(const mat4& m,const vec3 v){
    vec4 V = m * vec4(v,1);
    return V.xyz() / V.w;
}

static vec3 N(const mat4& m,const vec3 v){
    vec4 V = m * vec4(v,0);
    return V.xyz();
}

BasicMesh* BasicMesh::box(const mat4& m, const vec4 &color){
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1));
    
    IndexBufferRAM* indices = mesh->addIndexBuffer(GeometryEnums::TRIANGLES,GeometryEnums::NONE);

    //Front back
    mesh->addVertex(V(m,vec3(0,0,0)),N(m,vec3(0,0,-1)),vec3(0,0,0),color);    
    mesh->addVertex(V(m,vec3(1,0,0)),N(m,vec3(0,0,-1)),vec3(1,0,0),color);    
    mesh->addVertex(V(m,vec3(1,1,0)),N(m,vec3(0,0,-1)),vec3(1,1,0),color);    
    mesh->addVertex(V(m,vec3(0,1,0)),N(m,vec3(0,0,-1)),vec3(0,1,0),color);    
    int o = 0;
    indices->add(0);
    indices->add(2);    
    indices->add(1);
    indices->add(0);
    indices->add(3);    
    indices->add(2);

    mesh->addVertex(V(m,vec3(0,0,1)),N(m,vec3(0,0,1)),vec3(0,0,1),color);    
    mesh->addVertex(V(m,vec3(1,0,1)),N(m,vec3(0,0,1)),vec3(1,0,1),color);    
    mesh->addVertex(V(m,vec3(1,1,1)),N(m,vec3(0,0,1)),vec3(1,1,1),color);    
    mesh->addVertex(V(m,vec3(0,1,1)),N(m,vec3(0,0,1)),vec3(0,1,1),color);    
    o += 4;
    indices->add(0+o); 
    indices->add(1+o);   
    indices->add(2+o);
    indices->add(0+o);    
    indices->add(2+o);
    indices->add(3+o);



    //Right left
    mesh->addVertex(V(m,vec3(0,0,0)),N(m,vec3(-1,0,0)),vec3(0,0,0),color);       
    mesh->addVertex(V(m,vec3(0,1,0)),N(m,vec3(-1,0,0)),vec3(0,1,0),color);    
    mesh->addVertex(V(m,vec3(0,1,1)),N(m,vec3(-1,0,0)),vec3(0,1,1),color);    
    mesh->addVertex(V(m,vec3(0,0,1)),N(m,vec3(-1,0,0)),vec3(0,0,1),color);    
    o += 4;
    indices->add(0+o); 
    indices->add(2+o);   
    indices->add(1+o);
    indices->add(0+o);  
    indices->add(3+o);  
    indices->add(2+o);

    mesh->addVertex(V(m,vec3(1,0,0)),N(m,vec3(1,0,0)),vec3(1,0,0),color);      
    mesh->addVertex(V(m,vec3(1,1,0)),N(m,vec3(1,0,0)),vec3(1,1,0),color);    
    mesh->addVertex(V(m,vec3(1,1,1)),N(m,vec3(1,0,0)),vec3(1,1,1),color);    
    mesh->addVertex(V(m,vec3(1,0,1)),N(m,vec3(1,0,0)),vec3(1,0,1),color);    
    o += 4;
    indices->add(0+o);   
    indices->add(1+o);
    indices->add(2+o); 
    indices->add(0+o);  
    indices->add(2+o);
    indices->add(3+o);  



    //top bottom
    mesh->addVertex(V(m,vec3(0,1,0)),N(m,vec3(0,1,0)),vec3(0,1,0),color);    
    mesh->addVertex(V(m,vec3(1,1,0)),N(m,vec3(0,1,0)),vec3(1,1,0),color);    
    mesh->addVertex(V(m,vec3(1,1,1)),N(m,vec3(0,1,0)),vec3(1,1,1),color);    
    mesh->addVertex(V(m,vec3(0,1,1)),N(m,vec3(0,1,0)),vec3(0,1,1),color);    
    o += 4;
    indices->add(0+o);   
    indices->add(2+o); 
    indices->add(1+o);
    indices->add(0+o);  
    indices->add(3+o);  
    indices->add(2+o);

    mesh->addVertex(V(m,vec3(0,0,0)),N(m,vec3(0,-1,0)),vec3(0,-1,0),color);     
    mesh->addVertex(V(m,vec3(1,0,0)),N(m,vec3(0,-1,0)),vec3(1,-1,0),color);    
    mesh->addVertex(V(m,vec3(1,0,1)),N(m,vec3(0,-1,0)),vec3(1,-1,1),color);    
    mesh->addVertex(V(m,vec3(0,0,1)),N(m,vec3(0,-1,0)),vec3(0,-1,1),color);    
    o += 4;
    indices->add(0+o);    
    indices->add(1+o);
    indices->add(2+o);
    indices->add(0+o);    
    indices->add(2+o);
    indices->add(3+o);

    return mesh;
}

BasicMesh* BasicMesh::coordindicator(const vec3& center,
                                     const float& size) {

    size_t segments = 16;
    float bsize = size*1.0f;
    float fsize = size*1.2f;
    float radius = size*0.08f;
    float arrowpart = 0.12f;
    float arrowRadius = 2.0f*radius;
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
   
    BasicMesh* xarrow = arrow(center - vec3(bsize, 0.0f, 0.0f),
                              center + vec3(fsize, 0.0f, 0.0f),
                              vec4(1.0f, 0.0f, 0.0f, 1.0f),
                              radius,
                              arrowpart,
                              arrowRadius,
                              segments);
    BasicMesh* yarrow = arrow(center - vec3(0.0f, bsize, 0.0f),
                              center + vec3(0.0f, fsize, 0.0f),
                              vec4(0.0f, 1.0f, 0.0f, 1.0f),
                              radius,
                              arrowpart,
                              arrowRadius,
                              segments);
    BasicMesh* zarrow = arrow(center - vec3(0.0f, 0.0f, bsize),
                              center + vec3(0.0f, 0.0f, fsize),
                              vec4(0.0f, 0.0f, 1.0f, 1.0f),
                              radius,
                              arrowpart,
                              arrowRadius,
                              segments);

    BasicMesh* sphere = colorsphere(center, 0.7f*size);

    mesh->append(sphere);
    mesh->append(xarrow);
    mesh->append(yarrow);
    mesh->append(zarrow);

    return mesh;
}

BasicMesh* BasicMesh::boundingbox(const mat4& basisandoffset, const vec4& color) {
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(basisandoffset);

    mesh->addVertex(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), color);
    mesh->addVertex(vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 0.0), color);
    mesh->addVertex(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), color);
    mesh->addVertex(vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), color);
    mesh->addVertex(vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), color);
    mesh->addVertex(vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color);
    mesh->addVertex(vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), color);
    mesh->addVertex(vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color);

    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::LINES, GeometryEnums::NONE);

    inds->add(0);
    inds->add(1);

    inds->add(0);
    inds->add(2);

    inds->add(0);
    inds->add(3);

    inds->add(1);
    inds->add(6);

    inds->add(1);
    inds->add(4);

    inds->add(2);
    inds->add(5);

    inds->add(2);
    inds->add(4);

    inds->add(3);
    inds->add(5);

    inds->add(3);
    inds->add(6);

    inds->add(5);
    inds->add(7);

    inds->add(6);
    inds->add(7);

    inds->add(4);
    inds->add(7);

    return mesh;
}

BasicMesh* BasicMesh::square(const vec3& pos, const vec3& normal, const glm::vec2& extent,
                             const vec4& color /*= vec4(1,1,1,1)*/,
                             const ivec2& inres /*= ivec2(1)*/) {
    BasicMesh* mesh = new BasicMesh();
    mesh->setModelMatrix(mat4(1.f));
    IndexBufferRAM* inds = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);

    vec3 right = orthvec(normal);
    vec3 up = glm::cross(right, normal);


    vec3 start = pos - 0.5f * extent.x * right - 0.5f * extent.y * up;
    ivec2 res = inres + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(
                start + static_cast<float>(i) / static_cast<float>(inres.x) * extent.x * right +
                    static_cast<float>(j) / static_cast<float>(inres.y) * extent.y * up,
                normal, vec3(static_cast<float>(i) / static_cast<float>(inres.x),
                             static_cast<float>(j) / static_cast<float>(inres.y), 0.0f),
                color);

            if (i != inres.x && j != inres.y) {
                inds->add(i + res.x * j);
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * (j + 1));

                inds->add(i + 1 + res.x * j);
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + res.x * (j + 1));
            }
        }
    }

    return mesh;
}

}  // namespace
