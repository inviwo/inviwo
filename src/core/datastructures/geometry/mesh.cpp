/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/util/docutils.h>
#include <inviwo/core/util/exception.h>

#include <fmt/format.h>
#include <ranges>

namespace inviwo {

Mesh::Mesh(DrawType dt, ConnectivityType ct) : Mesh{MeshInfo{.dt = dt, .ct = ct}} {}

Mesh::Mesh(MeshInfo meshInfo)
    : DataGroup<Mesh, MeshRepresentation>()
    , SpatialEntity{}
    , MetaDataOwner{}
    , axes{util::defaultAxes<3>()}
    , meshInfo_{meshInfo} {}

Mesh::Mesh(const BufferVector& buffers, const IndexVector& indices)
    : Mesh{{.dt = DrawType::NotSpecified, .ct = ConnectivityType::None}} {
    addBuffers(buffers);
    addIndices(indices);
}

Mesh::Mesh(const Mesh& rhs)
    : DataGroup<Mesh, MeshRepresentation>(rhs)
    , SpatialEntity(rhs)
    , MetaDataOwner(rhs)
    , axes{rhs.axes}
    , meshInfo_(rhs.meshInfo_) {
    for (const auto& [info, buffer] : rhs.buffers_) {
        buffers_.emplace_back(info, std::shared_ptr<BufferBase>(buffer->clone()));
    }
    for (const auto& [info, buffer] : rhs.indices_) {
        indices_.emplace_back(info, std::shared_ptr<IndexBuffer>(buffer->clone()));
    }
}

Mesh::Mesh(const Mesh& rhs, NoData)
    : DataGroup<Mesh, MeshRepresentation>(rhs)
    , SpatialEntity(rhs)
    , MetaDataOwner(rhs)
    , axes{rhs.axes}
    , meshInfo_(rhs.meshInfo_) {}

Mesh& Mesh::operator=(const Mesh& that) {
    if (this != &that) {
        DataGroup<Mesh, MeshRepresentation>::operator=(that);
        SpatialEntity::operator=(that);
        MetaDataOwner::operator=(that);
        axes = that.axes;

        BufferVector buffers;
        IndexVector indices;

        for (const auto& [info, buffer] : that.buffers_) {
            buffers.emplace_back(info, std::shared_ptr<BufferBase>(buffer->clone()));
        }
        for (const auto& [info, buffer] : that.indices_) {
            indices.emplace_back(info, std::shared_ptr<IndexBuffer>(buffer->clone()));
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

void Mesh::addBuffer(BufferInfo info, std::shared_ptr<BufferBase> buffer) {
    auto it = std::ranges::find_if(
        buffers_, [&](const auto& item) { return item.first.location == info.location; });

    if (it == buffers_.end()) {
        buffers_.emplace_back(info, buffer);
    } else {
        throw Exception(SourceContext{}, "Location '{}' already used in Mesh", info.location);
    }
}

void Mesh::addBuffer(BufferType type, std::shared_ptr<BufferBase> buffer) {
    addBuffer(BufferInfo(type), std::move(buffer));
}

void Mesh::addBuffers(const BufferVector& buffers) {
    for (auto&& [info, buffer] : buffers) {
        addBuffer(info, buffer);
    }
}

auto Mesh::removeBuffer(size_t idx) -> std::pair<BufferInfo, std::shared_ptr<BufferBase>> {
    if (idx < buffers_.size()) {
        auto old = buffers_[idx];
        buffers_.erase(buffers_.begin() + idx);
        return old;
    } else {
        return {};
    }
}

auto Mesh::removeBuffer(BufferBase* buffer) -> std::pair<BufferInfo, std::shared_ptr<BufferBase>> {
    if (auto it = std::ranges::find_if(
            buffers_, [&](const auto& item) { return item.second.get() == buffer; });
        it != buffers_.end()) {
        auto old = *it;
        buffers_.erase(it);
        return old;
    }
    return {};
}

auto Mesh::replaceBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> buff)
    -> std::pair<BufferInfo, std::shared_ptr<BufferBase>> {
    if (idx < buffers_.size()) {
        auto old = buffers_[idx];
        if (old.first.location != info.location) {
            if (auto it = std::ranges::find_if(
                    buffers_,
                    [&](const auto& item) { return item.first.location == info.location; });
                it != buffers_.end()) {
                throw Exception(SourceContext{}, "Location '{}' already used in Mesh",
                                info.location);
            }
        }

        buffers_[idx] = std::make_pair(info, buff);
        return old;
    } else {
        addBuffer(info, buff);
        return {};
    }
}

auto Mesh::replaceBuffer(BufferBase* oldbuffer, BufferInfo info, std::shared_ptr<BufferBase> buff)
    -> std::pair<BufferInfo, std::shared_ptr<BufferBase>> {
    if (auto it = std::ranges::find_if(
            buffers_, [&](const auto& item) { return item.second.get() == oldbuffer; });
        it != buffers_.end()) {
        auto old = *it;
        if (old.first.location != info.location) {
            if (auto locit = std::ranges::find_if(
                    buffers_,
                    [&](const auto& item) { return item.first.location == info.location; });
                locit != buffers_.end()) {
                throw Exception(SourceContext{}, "Location '{}' already used in Mesh",
                                info.location);
            }
        }

        *it = std::make_pair(info, buff);
        return old;
    } else {
        addBuffer(info, buff);
        return {};
    }
}

void Mesh::setBuffer(size_t idx, BufferInfo info, std::shared_ptr<BufferBase> buffer) {
    replaceBuffer(idx, info, std::move(buffer));
}

void Mesh::addIndices(MeshInfo info, std::shared_ptr<IndexBuffer> ind) {
    indices_.emplace_back(info, ind);
}

void Mesh::addIndices(const IndexVector& indices) {
    for (auto&& [info, buffer] : indices) {
        addIndices(info, buffer);
    }
}

std::shared_ptr<IndexBufferRAM> Mesh::addIndexBuffer(DrawType dt, ConnectivityType ct) {
    auto indicesRam = std::make_shared<IndexBufferRAM>();
    auto indices = std::make_shared<IndexBuffer>(indicesRam);
    addIndices({.dt = dt, .ct = ct}, indices);
    return indicesRam;
}

void Mesh::removeIndexBuffer(size_t idx) {
    if (idx < indices_.size()) {
        indices_.erase(indices_.begin() + idx);
    }
}

void Mesh::reserveSizeInVertexBuffer(size_t size) {
    for (const auto& [_, buf] : buffers_) {
        buf->getEditableRepresentation<BufferRAM>()->reserve(size);
    }
}

void Mesh::reserveIndexBuffers(size_t size) { indices_.reserve(size); }

const BufferBase* Mesh::getBuffer(size_t idx) const {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range");
    }
    return buffers_[idx].second.get();
}

std::pair<const BufferBase*, int> Mesh::findBuffer(BufferType type) const {
    if (auto it = std::ranges::find_if(buffers_,
                                       [&](const auto& item) { return item.first.type == type; });
        it != buffers_.end()) {
        return {it->second.get(), it->first.location};
    } else {
        return {nullptr, 0};
    }
}

std::pair<BufferBase*, int> Mesh::findBuffer(BufferType type) {
    if (auto it = std::ranges::find_if(buffers_,
                                       [&](const auto& item) { return item.first.type == type; });
        it != buffers_.end()) {
        return {it->second.get(), it->first.location};
    } else {
        return {nullptr, 0};
    }
}

bool Mesh::hasBuffer(BufferType type) const { return findBuffer(type).first != nullptr; }

Mesh::BufferInfo Mesh::getBufferInfo(size_t idx) const {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range");
    }
    return buffers_[idx].first;
}

Mesh::BufferInfo Mesh::getBufferInfo(BufferBase* buffer) const {
    if (auto it = std::ranges::find_if(
            buffers_, [&](const auto& item) { return item.second.get() == buffer; });
        it != buffers_.end()) {
        return it->first;
    } else {
        throw Exception("Buffer not found in Mesh");
    }
}

void Mesh::setBufferInfo(size_t idx, BufferInfo info) {
    if (idx < buffers_.size()) {
        if (auto locit = std::ranges::find_if(
                buffers_, [&](const auto& item) { return item.first.location == info.location; });
            &*locit != &buffers_[idx] && locit != buffers_.end()) {
            throw Exception(SourceContext{}, "Location '{}' already used in Mesh", info.location);
        }

        buffers_[idx].first = info;
    } else {
        throw RangeException("Index out of range");
    }
}

void Mesh::setBufferInfo(BufferBase* buffer, BufferInfo info) {
    if (auto it = std::ranges::find_if(
            buffers_, [&](const auto& item) { return item.second.get() == buffer; });
        it != buffers_.end()) {
        if (auto locit = std::ranges::find_if(
                buffers_, [&](const auto& item) { return item.first.location == info.location; });
            locit != it && locit != buffers_.end()) {
            throw Exception(SourceContext{}, "Location '{}' already used in Mesh", info.location);
        }
        it->first = info;
    } else {
        throw Exception("Buffer not found in Mesh");
    }
}

const IndexBuffer* Mesh::getIndices(size_t idx) const {
    if (idx >= indices_.size()) {
        throw RangeException("Index out of range");
    }
    return indices_[idx].second.get();
}

BufferBase* Mesh::getBuffer(size_t idx) {
    if (idx >= buffers_.size()) {
        throw RangeException("Index out of range");
    }
    return buffers_[idx].second.get();
}

BufferBase* Mesh::getBuffer(BufferType type) {
    if (auto it = std::ranges::find_if(buffers_,
                                       [&](const auto& item) { return item.first.type == type; });
        it != buffers_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

IndexBuffer* Mesh::getIndices(size_t idx) {
    if (idx >= indices_.size()) {
        throw RangeException(SourceContext{}, "Index out of range");
    }
    return indices_[idx].second.get();
}

Mesh::MeshInfo Mesh::getDefaultMeshInfo() const { return meshInfo_; }

void Mesh::setDefaultMeshInfo(MeshInfo info) { meshInfo_ = info; }

Mesh::MeshInfo Mesh::getIndexMeshInfo(size_t idx) const {
    if (idx >= indices_.size()) {
        throw RangeException("Index out of range");
    }
    return indices_[idx].first;
}

size_t Mesh::getNumberOfBuffers() const { return buffers_.size(); }

size_t Mesh::getNumberOfIndicies() const { return indices_.size(); }

size_t Mesh::getNumberOfIndices() const { return indices_.size(); }

void Mesh::append(const Mesh& mesh) {
    if (buffers_.size() != mesh.buffers_.size()) {
        throw Exception("Mismatched meshed, number of buffer does not match");
    }
    for (size_t i = 0; i < buffers_.size(); ++i) {
        if (buffers_[i].first != mesh.buffers_[i].first ||
            buffers_[i].second->getDataFormat() != mesh.buffers_[i].second->getDataFormat()) {
            throw Exception("Mismatched meshed, buffer types does not match");
        }
    }
    size_t size = buffers_[0].second->getSize();
    for (size_t i = 0; i < buffers_.size(); ++i) {
        buffers_[i].second->append(*(mesh.buffers_[i].second));
    }
    for (const auto& [meshInfo, indbuffer] : mesh.indices_) {
        const auto& inds = indbuffer->getRAMRepresentation()->getDataContainer();

        std::vector<std::uint32_t> newInds;
        newInds.reserve(inds.size());
        std::ranges::transform(inds, std::back_inserter(newInds),
                               [&](auto& i) { return i + static_cast<uint32_t>(size); });
        addIndices(meshInfo, util::makeIndexBuffer(std::move(newInds)));
    }
}

const Axis* Mesh::getAxis(size_t index) const {
    if (index >= 3) {
        return nullptr;
    }
    return &axes[index];
}

Document Mesh::getInfo() const {
    const int maxLines = 20;

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Mesh", {{"style", "color:white;"}});

    utildoc::TableBuilder tb(doc.handle(), P::end());

    tb(H("Axis 1"), fmt::format("{}{: [}", axes[0].name, axes[0].unit));
    tb(H("Axis 2"), fmt::format("{}{: [}", axes[1].name, axes[1].unit));
    tb(H("Axis 3"), fmt::format("{}{: [}", axes[2].name, axes[2].unit));

    tb(H("Model Matrix"), getModelMatrix());
    tb(H("World Matrix"), getWorldMatrix());

    // show all the buffers
    utildoc::TableBuilder tb2(doc.handle(), P::end());
    tb2(H(fmt::format("Buffers ({})", buffers_.size())));
    for (const auto& [bufferInfo, buffer] : buffers_) {
        tb2(fmt::format("{}", bufferInfo), buffer->getDataFormat()->getString(),
            fmt::format("{}", buffer->getBufferUsage()), fmt::format(" ({})", buffer->getSize()));
    }

    // ensure that at least one index buffer is shown
    const auto maxPrintableIndexBuf =
        std::max<int>(maxLines - static_cast<int>(buffers_.size()), 1);
    const auto numIndexBuffers = static_cast<int>(indices_.size());

    utildoc::TableBuilder tb3(doc.handle(), P::end());
    if (!indices_.empty()) {
        tb3(H(fmt::format("Indexbuffers ({})", indices_.size())));
    }
    for (const auto& [info, buffer] : indices_ | std::views::take(maxPrintableIndexBuf)) {
        tb3(fmt::format("{}", info.dt), fmt::format("{}", info.ct),
            fmt::format("({})", buffer->getSize()));
    }
    if (maxPrintableIndexBuf < numIndexBuffers) {
        tb3(fmt::format("... ({} additional buffers)", numIndexBuffers - maxPrintableIndexBuf));
    }

    return doc;
}

std::ostream& operator<<(std::ostream& ss, Mesh::BufferInfo info) {
    ss << info.type << " (location = " << info.location << ")";
    return ss;
}

std::string format_as(const Mesh::BufferInfo& info) {
    return fmt::format("{} (location = {})", info.type, info.location);
}

template class IVW_CORE_TMPL_INST DataReaderType<Mesh>;
template class IVW_CORE_TMPL_INST DataWriterType<Mesh>;

template class IVW_CORE_TMPL_INST DataInport<Mesh>;
template class IVW_CORE_TMPL_INST DataInport<Mesh, 0, false>;
template class IVW_CORE_TMPL_INST DataInport<Mesh, 0, true>;
template class IVW_CORE_TMPL_INST DataInport<DataSequence<Mesh>>;
template class IVW_CORE_TMPL_INST DataInport<DataSequence<Mesh>, 0, false>;
template class IVW_CORE_TMPL_INST DataInport<DataSequence<Mesh>, 0, true>;
template class IVW_CORE_TMPL_INST DataOutport<Mesh>;
template class IVW_CORE_TMPL_INST DataOutport<DataSequence<Mesh>>;

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
