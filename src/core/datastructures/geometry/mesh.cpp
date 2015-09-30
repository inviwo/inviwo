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

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/tooltiphelper.h>

namespace inviwo {

Mesh::Mesh(DrawType dt, ConnectivityType ct)
    : DataGroup(), SpatialEntity<3>(), defaultMeshInfo_(MeshInfo(dt, ct)) {}

Mesh::Mesh(const Mesh& rhs)
    : DataGroup(rhs), SpatialEntity<3>(rhs), defaultMeshInfo_(rhs.defaultMeshInfo_) {
    for (const auto& elem : rhs.buffers_) {
        buffers_.emplace_back(elem.first, std::shared_ptr<BufferBase>(elem.second->clone()));
    }
    for (const auto& elem : rhs.indices_) {
        indices_.emplace_back(elem.first, std::shared_ptr<IndexBuffer>(elem.second->clone()));
    }
}

Mesh& Mesh::operator=(const Mesh& that) {
    if (this != &that) {
        DataGroup::operator=(that);
        SpatialEntity<3>::operator=(that);

        BufferVector buffers;
        IndexVector indices;

        for (const auto& attr : that.buffers_) {
            buffers.emplace_back(attr.first, std::shared_ptr<BufferBase>(attr.second->clone()));
        }
        for (const auto& elem : that.indices_) {
            indices.emplace_back(elem.first, std::shared_ptr<IndexBuffer>(elem.second->clone()));
        }

        std::swap(buffers, buffers_);
        std::swap(indices, indices_);
        defaultMeshInfo_ = that.defaultMeshInfo_;
    }
    return *this;
}

Mesh* Mesh::clone() const { return new Mesh(*this); }

const Mesh::BufferVector& Mesh::getBuffers() const { return buffers_; }

const Mesh::IndexVector& Mesh::getIndexBuffers() const { return indices_; }

void Mesh::addBuffer(BufferType type, std::shared_ptr<BufferBase> att) {
    buffers_.emplace_back(type, att);
}

void Mesh::setBuffer(size_t idx, BufferType type, std::shared_ptr<BufferBase> att) {
    if (idx < buffers_.size()) {
        buffers_[idx] = std::make_pair(type, att);
    }
}

void Mesh::addIndicies(MeshInfo info, std::shared_ptr<IndexBuffer> ind) {
    indices_.push_back(std::make_pair(info, ind));
}

const BufferBase* Mesh::getBuffer(size_t idx) const { return buffers_[idx].second.get(); }

const IndexBuffer* Mesh::getIndicies(size_t idx) const { return indices_[idx].second.get(); }

BufferBase* Mesh::getBuffer(size_t idx) { return buffers_[idx].second.get(); }

IndexBuffer* Mesh::getIndicies(size_t idx) { return indices_[idx].second.get(); }

Mesh::MeshInfo Mesh::getDefaultMeshInfo() const { return defaultMeshInfo_; }

Mesh::MeshInfo Mesh::getIndexMeshInfo(size_t idx) const { return indices_[idx].first; }

size_t Mesh::getNumberOfBuffers() const { return buffers_.size(); }

size_t Mesh::getNumberOfIndicies() const { return indices_.size(); }

const SpatialCameraCoordinateTransformer<3>& Mesh::getCoordinateTransformer(
    const Camera& camera) const {
    return SpatialEntity<3>::getCoordinateTransformer(camera);
}

inviwo::uvec3 Mesh::COLOR_CODE = uvec3(188, 188, 101);

const std::string Mesh::CLASS_IDENTIFIER = "org.inviwo.Mesh";

std::string Mesh::getDataInfo() const {
    ToolTipHelper t("Mesh");
    t.tableTop();

    for (const auto& elem : indices_) {
        std::stringstream ss;
        ss << elem.first.dt << " " << elem.first.ct;
        ss << " (" << elem.second->getSize() << ")";
        t.row("IndexBuffer", ss.str());
    }

    for (const auto& elem : buffers_) {
        std::stringstream ss;
        ss << elem.first << " " << elem.second->getBufferUsage();
        ss << " (" << elem.second->getSize() << ")";
        t.row("Buffer", ss.str());
    }

    t.tableBottom();
    return t;
}

}  // namespace
