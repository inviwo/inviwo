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

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/document.h>

namespace inviwo {

Mesh::Mesh(DrawType dt, ConnectivityType ct) : Mesh{MeshInfo{dt, ct}} {}

Mesh::Mesh(Mesh::MeshInfo meshInfo)
    : DataGroup<Mesh, MeshRepresentation>()
    , SpatialEntity<3>{}
    , MetaDataOwner{}
    , meshInfo_{meshInfo} {}

Mesh::Mesh(const Mesh& rhs)
    : DataGroup<Mesh, MeshRepresentation>(rhs)
    , SpatialEntity<3>(rhs)
    , MetaDataOwner(rhs)
    , meshInfo_(rhs.meshInfo_) {
    for (const auto& elem : rhs.buffers_) {
        buffers_.emplace_back(elem.first, std::shared_ptr<BufferBase>(elem.second->clone()));
    }
    for (const auto& elem : rhs.indices_) {
        indices_.emplace_back(elem.first, std::shared_ptr<IndexBuffer>(elem.second->clone()));
    }
}

Mesh& Mesh::operator=(const Mesh& that) {
    if (this != &that) {
        DataGroup<Mesh, MeshRepresentation>::operator=(that);
        SpatialEntity<3>::operator=(that);
        MetaDataOwner::operator=(that);

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
        meshInfo_ = that.meshInfo_;
    }
    return *this;
}

Mesh* Mesh::clone() const { return new Mesh(*this); }

const Mesh::BufferVector& Mesh::getBuffers() const { return buffers_; }

const Mesh::IndexVector& Mesh::getIndexBuffers() const { return indices_; }

void Mesh::addBuffer(BufferInfo info, std::shared_ptr<BufferBase> att) {
    buffers_.emplace_back(info, att);
}

void Mesh::addBuffer(BufferType type, std::shared_ptr<BufferBase> att) {
    buffers_.emplace_back(BufferInfo(type), att);
}

void Mesh::removeBuffer(size_t idx) {
    if (idx < buffers_.size()) {
        buffers_.erase(buffers_.begin() + idx);
    }
}

void Mesh::replaceBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> att) {
    if (idx < buffers_.size()) {
        buffers_[idx] = std::make_pair(info, att);
    }
}

void Mesh::setBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> att) {
    LogWarn("Deprecated: Mesh::setBuffer() has been renamed to Mesh::replaceBuffer()");
    replaceBuffer(idx, info, att);
}

void Mesh::addIndices(MeshInfo info, std::shared_ptr<IndexBuffer> ind) {
    indices_.push_back(std::make_pair(info, ind));
}

std::shared_ptr<inviwo::IndexBufferRAM> Mesh::addIndexBuffer(DrawType dt, ConnectivityType ct) {
    auto indicesRam = std::make_shared<IndexBufferRAM>();
    auto indices = std::make_shared<IndexBuffer>(indicesRam);
    addIndices(Mesh::MeshInfo(dt, ct), indices);
    return indicesRam;
}

void Mesh::removeIndexBuffer(size_t idx) {
    if (idx < indices_.size()) {
        indices_.erase(indices_.begin() + idx);
    }
}

void Mesh::reserveSizeInVertexBuffer(size_t size) {
    for (auto& buf : buffers_) {
        buf.second->getEditableRepresentation<BufferRAM>()->reserve(size);
    }
}

void Mesh::reserveIndexBuffers(size_t size) { indices_.reserve(size); }

const BufferBase* Mesh::getBuffer(size_t idx) const {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return buffers_[idx].second.get();
}

std::pair<const BufferBase*, int> Mesh::findBuffer(BufferType type) const {
    auto it = std::find_if(buffers_.begin(), buffers_.end(),
                           [&](const auto& item) { return item.first.type == type; });
    if (it != buffers_.end()) {
        return {it->second.get(), it->first.location};
    } else {
        return {nullptr, 0};
    }
}

bool Mesh::hasBuffer(BufferType type) const { return findBuffer(type).first != nullptr; }

Mesh::BufferInfo Mesh::getBufferInfo(size_t idx) const {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return buffers_[idx].first;
}

const IndexBuffer* Mesh::getIndices(size_t idx) const {
    if (idx >= indices_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return indices_[idx].second.get();
}

BufferBase* Mesh::getBuffer(size_t idx) {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return buffers_[idx].second.get();
}

BufferBase* Mesh::getBuffer(BufferType type) {
    auto it = std::find_if(buffers_.begin(), buffers_.end(),
                           [&](const auto& item) { return item.first.type == type; });
    if (it != buffers_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

IndexBuffer* Mesh::getIndices(size_t idx) {
    if (idx >= indices_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return indices_[idx].second.get();
}

Mesh::MeshInfo Mesh::getDefaultMeshInfo() const { return meshInfo_; }

Mesh::MeshInfo Mesh::getIndexMeshInfo(size_t idx) const {
    if (idx >= indices_.size()) {
        throw RangeException("Index out of range", IVW_CONTEXT);
    }
    return indices_[idx].first;
}

size_t Mesh::getNumberOfBuffers() const { return buffers_.size(); }

size_t Mesh::getNumberOfIndicies() const { return indices_.size(); }

void Mesh::append(const Mesh& mesh) {
    if (buffers_.size() != mesh.buffers_.size()) {
        throw Exception("Mismatched meshed, number of buffer does not match", IVW_CONTEXT);
    }
    for (size_t i = 0; i < buffers_.size(); ++i) {
        if (buffers_[i].first != mesh.buffers_[i].first ||
            buffers_[i].second->getDataFormat() != mesh.buffers_[i].second->getDataFormat()) {
            throw Exception("Mismatched meshed, buffer types does not match", IVW_CONTEXT);
        }
    }
    size_t size = buffers_[0].second->getSize();
    for (size_t i = 0; i < buffers_.size(); ++i) {
        buffers_[i].second->append(*(mesh.buffers_[i].second));
    }
    for (auto indbuffer : mesh.indices_) {
        const auto& inds = indbuffer.second->getRAMRepresentation()->getDataContainer();

        std::vector<std::uint32_t> newInds;
        newInds.reserve(inds.size());
        std::transform(inds.begin(), inds.end(), std::back_inserter(newInds),
                       [&](auto& i) { return i + static_cast<uint32_t>(size); });
        addIndices(indbuffer.first, util::makeIndexBuffer(std::move(newInds)));
    }
}

const SpatialCameraCoordinateTransformer<3>& Mesh::getCoordinateTransformer(
    const Camera& camera) const {
    return SpatialEntity<3>::getCoordinateTransformer(camera);
}

uvec3 Mesh::colorCode = uvec3(188, 188, 101);
const std::string Mesh::classIdentifier = "org.inviwo.Mesh";
const std::string Mesh::dataName = "Mesh";

Document Mesh::getInfo() const {
    const int maxLines = 20;

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Mesh", {{"style", "color:white;"}});

    utildoc::TableBuilder tb(doc.handle(), P::end());

    tb(H(std::string("Buffers (") + std::to_string(buffers_.size()) + ")"));

    // show all the buffers
    for (const auto& elem : buffers_) {
        std::stringstream ss1;
        ss1 << elem.first << ", " << elem.second->getBufferUsage();
        std::stringstream ss2;
        ss2 << " (" << elem.second->getSize() << ")";
        tb(ss1.str(), ss2.str());
    }

    // ensure that at least one index buffer is shown
    int maxPrintableIndexBuf = std::max<int>(maxLines - static_cast<int>(buffers_.size()), 1);
    int numIndexBuffers = static_cast<int>(indices_.size());

    if (!indices_.empty()) {
        std::stringstream ss;
        ss << "Indexbuffers (" << indices_.size() << ")";
        tb(H(ss.str()));
    }
    int line = 0;
    for (const auto& elem : indices_) {
        std::stringstream ss1;
        ss1 << elem.first.dt << ", " << elem.first.ct;
        std::stringstream ss2;
        ss2 << " (" << elem.second->getSize() << ")";
        tb(ss1.str(), ss2.str());
        ++line;
        if (line >= maxPrintableIndexBuf) break;
    }
    if (maxPrintableIndexBuf < numIndexBuffers) {
        std::stringstream ss;
        ss << "... (" << (numIndexBuffers - maxPrintableIndexBuf) << " additional buffers)";
        tb(ss.str());
    }

    return doc;
}

template class IVW_CORE_TMPL_INST DataReaderType<Mesh>;
template class IVW_CORE_TMPL_INST DataWriterType<Mesh>;

namespace meshutil {

bool hasPickIDBuffer(const Mesh* mesh) {
    if (!mesh) return false;
    return mesh->hasBuffer(BufferType::PickingAttrib);
}

bool hasRadiiBuffer(const Mesh* mesh) {
    if (!mesh) return false;
    return mesh->hasBuffer(BufferType::RadiiAttrib);
}

}  // namespace meshutil

}  // namespace inviwo
