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

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>


namespace inviwo {
BasicMesh::BasicMesh() : Mesh() {
    vertices_ = std::make_shared<Buffer<vec3>>();
    texCoords_ = std::make_shared<Buffer<vec3>>();
    colors_ = std::make_shared<Buffer<vec4>>();
    normals_ = std::make_shared<Buffer<vec3>>();

    addBuffer(BufferType::PositionAttrib, vertices_);   // pos 0
    addBuffer(BufferType::TexcoordAttrib, texCoords_);  // pos 1
    addBuffer(BufferType::ColorAttrib, colors_);        // pos 2
    addBuffer(BufferType::NormalAttrib, normals_);      // pos 3
}

BasicMesh::BasicMesh(const BasicMesh& rhs) : Mesh(rhs) {
    vertices_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[0].second);
    texCoords_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[1].second);
    colors_ = std::static_pointer_cast<Buffer<vec4>>(buffers_[2].second);
    normals_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[3].second);
}


BasicMesh& BasicMesh::operator=(const BasicMesh& that) {
    if (this != &that) {
        Mesh::operator=(that);
        vertices_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[0].second);
        texCoords_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[1].second);
        colors_ = std::static_pointer_cast<Buffer<vec4>>(buffers_[2].second);
        normals_ = std::static_pointer_cast<Buffer<vec3>>(buffers_[3].second);
    }
    return *this;
}

BasicMesh* BasicMesh::clone() const {
    return new BasicMesh(*this); 
}

uint32_t BasicMesh::addVertex(vec3 pos, vec3 normal, vec3 texCoord, vec4 color) {
    getEditableVerticesRAM()->add(pos);
    getEditableTexCoordsRAM()->add(texCoord);
    getEditableColorsRAM()->add(color);
    getEditableNormalsRAM()->add(normal);
    return static_cast<uint32_t>(getVertices()->getSize() - 1);
}

void BasicMesh::addVertices(const std::vector<Vertex> &data) {
    auto v = getEditableVerticesRAM();
    auto t = getEditableTexCoordsRAM();
    auto c = getEditableColorsRAM();
    auto n = getEditableNormalsRAM();

    v->getDataContainer().reserve(data.size() + v->getDataContainer().size());
    t->getDataContainer().reserve(data.size() + t->getDataContainer().size());
    c->getDataContainer().reserve(data.size() + c->getDataContainer().size());
    n->getDataContainer().reserve(data.size() + n->getDataContainer().size());

    for (const auto& elem : data) {
        v->add(elem.pos);
        n->add(elem.normal);
        t->add(elem.tex);
        c->add(elem.color);
    }
}

void BasicMesh::setVertex(size_t index, vec3 pos, vec3 normal, vec3 texCoord, vec4 color) {
    getEditableVerticesRAM()->set(index, pos);
    getEditableTexCoordsRAM()->set(index, texCoord);
    getEditableColorsRAM()->set(index, color);
    getEditableNormalsRAM()->set(index, normal);
}

void BasicMesh::setVertexPosition(size_t index, vec3 pos) {
    getEditableVerticesRAM()->set(index, pos);
}

void BasicMesh::setVertexNormal(size_t index, vec3 normal) {
    getEditableNormalsRAM()->set(index, normal);
}

void BasicMesh::setVertexTexCoord(size_t index, vec3 texCoord) {
    getEditableTexCoordsRAM()->set(index, texCoord);
}

void BasicMesh::setVertexColor(size_t index, vec4 color) {
    getEditableColorsRAM()->set(index, color);
}

IndexBufferRAM* BasicMesh::addIndexBuffer(DrawType dt, ConnectivityType ct) {
    auto indicesRam = std::make_shared<IndexBufferRAM>();
    auto indices = std::make_shared<IndexBuffer>(indicesRam);
    addIndicies(Mesh::MeshInfo(dt, ct), indices);
    return indicesRam.get();
}

void BasicMesh::append(const BasicMesh* mesh) {
    if (mesh) Mesh::append(*mesh);
}

const Buffer<vec3>* BasicMesh::getVertices() const { return vertices_.get(); }

const Buffer<vec3>* BasicMesh::getTexCoords() const { return texCoords_.get(); }

const Buffer<vec4>* BasicMesh::getColors() const { return colors_.get(); }

const Buffer<vec3>* BasicMesh::getNormals() const { return normals_.get(); }

Buffer<vec3>* BasicMesh::getEditableVertices() { return vertices_.get(); }

Buffer<vec3>* BasicMesh::getEditableTexCoords() { return texCoords_.get(); }

Buffer<vec4>* BasicMesh::getEditableColors() { return colors_.get(); }

Buffer<vec3>* BasicMesh::getEditableNormals() { return normals_.get(); }

const Vec3BufferRAM* BasicMesh::getVerticesRAM() const { return vertices_->getRAMRepresentation(); }
const Vec3BufferRAM* BasicMesh::getTexCoordsRAM() const {
    return texCoords_->getRAMRepresentation();
}
const Vec4BufferRAM* BasicMesh::getColorsRAM() const { return colors_->getRAMRepresentation(); }
const Vec3BufferRAM* BasicMesh::getNormalsRAM() const { return normals_->getRAMRepresentation(); }

Vec3BufferRAM* BasicMesh::getEditableVerticesRAM() {
    return vertices_->getEditableRAMRepresentation();
}

Vec3BufferRAM* BasicMesh::getEditableTexCoordsRAM() {
    return texCoords_->getEditableRAMRepresentation();
}

Vec4BufferRAM* BasicMesh::getEditableColorsRAM() { return colors_->getEditableRAMRepresentation(); }
Vec3BufferRAM* BasicMesh::getEditableNormalsRAM() {
    return normals_->getEditableRAMRepresentation();
}

}  // namespace
