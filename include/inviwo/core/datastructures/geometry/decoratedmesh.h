/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_DECORATEDMESH_H
#define IVW_DECORATEDMESH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

namespace detail {}  // namespace detail

template <typename T, unsigned DIM, int pos, int location = static_cast<int>(pos)>
struct BufferTrait {
    using type = Vector<DIM, T>;
    static constexpr inviwo::BufferType bufType = static_cast<inviwo::BufferType>(pos);
    static constexpr int bufLog = location;
    static inviwo::Mesh::BufferInfo bi() { return {bufType, location}; }

#if _MSC_VER <= 1900
    BufferTrait(type t) : t(t) {}
    type t;
    operator type() const { return t; }
#endif
};

// Default buffer traits
using PositionsBufferTrait = BufferTrait<float, 3, static_cast<int>(BufferType::PositionAttrib)>;
using NormalBufferTrait = BufferTrait<float, 3, static_cast<int>(BufferType::NormalAttrib)>;
using ColorsBufferTrait = BufferTrait<float, 4, static_cast<int>(BufferType::ColorAttrib)>;
using TexcoordBufferTrait = BufferTrait<float, 3, static_cast<int>(BufferType::TexcoordAttrib)>;
using CurvatureBufferTrait = BufferTrait<float, 1, static_cast<int>(BufferType::CurvatureAttrib)>;
using IndexBufferTrait = BufferTrait<uint32_t, 1, static_cast<int>(BufferType::IndexAttrib)>;
using RadiiBufferTrait =
    BufferTrait<float, 1, static_cast<int>(BufferType::NumberOfBufferTypes), 5>;

/**
 * \ingroup datastructures
 */
template <typename... BuffersTraits>
class DecoratedMesh : public Mesh {
public:
#if _MSC_VER <= 1900
    using Vertex = std::tuple<BuffersTraits...>;
#else
    template <typename T>
    using TypeAlias = typename T::type;
    using Vertex = std::tuple<TypeAlias<BuffersTraits>...>;
#endif
    using Slef = typename DecoratedMesh<BuffersTraits...>;

    DecoratedMesh() { addBuffersImpl<0, BuffersTraits...>(); }
    DecoratedMesh(const DecoratedMesh &rhs) : Mesh(rhs) { copyConstrHelper<0, BuffersTraits...>(); }
    DecoratedMesh &operator=(const DecoratedMesh &that) {
        if (this != &that) {
            Mesh::operator=(that);
            copyConstrHelper<0, BuffersTraits...>();
        }
        return *this;
    }

    virtual Slef *clone() const override { return new Slef(*this); }

    virtual ~DecoratedMesh() {}

    void addVertices(const std::vector<Vertex> &vertices) {
        addVerticesImpl<0, BuffersTraits...>(vertices);
    }
#if _MSC_VER <= 1900
    void addVertex(BuffersTraits... args) { addVertexImpl<0>(args...); }
#else
    void addVertex(TypeAlias<BuffersTraits>... args) { addVertexImpl<0>(args...); }
#endif

    IndexBufferRAM *addIndexBuffer(DrawType dt, ConnectivityType ct) {
        auto indicesRam = std::make_shared<IndexBufferRAM>();
        auto indices = std::make_shared<IndexBuffer>(indicesRam);
        addIndicies(Mesh::MeshInfo(dt, ct), indices);
        return indicesRam.get();
    }

    template <unsigned I>
    auto getTypedBuffer() {
        return std::get<I>(buffers_).buffer_;
    }

    template <unsigned I>
    auto getTypedBuffer() const {
        return std::get<I>(buffers_).buffer_;
    }

    template <unsigned I>
    auto getTypedEditableRAMRepresentation() {
        return getTypedBuffer<I>()->getEditableRAMRepresentation();
    }

    template <unsigned I>
    auto getTypedRAMRepresentation() const {
        return getTypedBuffer<I>()->getRAMRepresentation();
    }

    template <unsigned I>
    auto &getTypedDataContainer() {
        return getTypedEditableRAMRepresentation<I>()->getDataContainer();
    }

    template <unsigned I>
    auto &getTypedDataContainer() const {
        return getTypedRAMRepresentation<I>()->getDataContainer();
    }

private:
    template <unsigned I>
    void addBuffersImpl() {}  // sink

    template <unsigned I, typename T, typename... ARGS>
    void addBuffersImpl() {
        addBuffer(T::bi(), std::get<I>(buffers_).buffer_);
        addBuffersImpl<I + 1, ARGS...>();
    }

    template <unsigned I, typename T>
    void addVertexImpl(T &t) {
        std::get<I>(buffers_).buffer_->getEditableRAMRepresentation()->add(t);
    }

    template <unsigned I, typename T, typename... ARGS>
    void addVertexImpl(T &t, ARGS... args) {
        std::get<I>(buffers_).buffer_->getEditableRAMRepresentation()->add(t);
        addVertexImpl<I + 1>(args...);
    }

    template <unsigned I>
    void addVerticesImpl(const std::vector<Vertex> &vertices) {}  // sink

    template <unsigned I, typename T, typename... ARGS>
    void addVerticesImpl(const std::vector<Vertex> &vertices) {

        auto &vec = getTypedDataContainer<I>();

        auto neededSize = vertices.size() + vec.size();
        if (vec.capacity() < neededSize) {
            vec.reserve(neededSize);
        }
        for (auto &v : vertices) {
            vec.push_back(std::get<I>(v));
        }

        addVerticesImpl<I + 1, ARGS...>(vertices);
    }

    template <unsigned I>
    void copyConstrHelper() {}  // sink

    template <unsigned I, typename T, typename... ARGS>
    void copyConstrHelper() {
        std::get<I>(buffers_).buffer_ =
            std::static_pointer_cast<Buffer<T::type>>(Mesh::buffers_[I].second);
        copyConstrHelper<I + 1, ARGS...>();
    }

private:
    template <typename T>
    struct BufferHolder {
        using BUF = inviwo::Buffer<typename T::type>;
        std::shared_ptr<BUF> buffer_{std::make_shared<BUF>()};
    };

    using BufferTuple = std::tuple<BufferHolder<BuffersTraits>...>;
    BufferTuple buffers_;
};

using MeshInterfaceSphere =
    DecoratedMesh<PositionsBufferTrait, RadiiBufferTrait, ColorsBufferTrait>;

}  // namespace inviwo

#endif  // IVW_DECORATEDMESH_H
