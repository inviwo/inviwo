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

/**
 * \defgroup datastructures Datastructures
 */

/**
 * \defgroup DecoratedMesh DecoratedMesh
 * \ingroup datastructures
 */

/**
 * Namespace for buffer traits
 * \ingroup DecoratedMesh
 */
namespace buffertraits {

    namespace detail{
        template <typename T, typename... Ts> struct get_index;

        template <typename T, typename... Ts>
        struct get_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

        template <typename T, typename Tail, typename... Ts>
        struct get_index<T, Tail, Ts...> :
            std::integral_constant<std::size_t, 1 + get_index<T, Ts...>::value> {};

#if 1 // explicit error case, but you already have error without that. 
        template <typename T>
        struct get_index<T>
        {
            // condition is always false, but should be dependant of T
            static_assert(sizeof(T) == 0, "element not found");
        };
#endif
    
    
    }

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

//! BufferTrait for Positions Buffers
using PositionsBuffer = BufferTrait<float, 3, static_cast<int>(BufferType::PositionAttrib)>;
//! BufferTrait for Normal Buffers
using NormalBuffer = BufferTrait<float, 3, static_cast<int>(BufferType::NormalAttrib)>;
//! BufferTrait for Colors Buffers
using ColorsBuffer = BufferTrait<float, 4, static_cast<int>(BufferType::ColorAttrib)>;
//! BufferTrait for Texcoord Buffers
using TexcoordBuffer = BufferTrait<float, 3, static_cast<int>(BufferType::TexcoordAttrib)>;
//! BufferTrait for Curvature Buffers
using CurvatureBuffer = BufferTrait<float, 1, static_cast<int>(BufferType::CurvatureAttrib)>;
//! BufferTrait for Index Buffers
using IndexBuffer = BufferTrait<uint32_t, 1, static_cast<int>(BufferType::IndexAttrib)>;
//! BufferTrait for Radii Buffer
using RadiiBuffer = BufferTrait<float, 1, static_cast<int>(BufferType::NumberOfBufferTypes), 5>;
}  // namespace buffertraits
   /**
    * \ingroup datastructures
    * \ingroup DecoratedMesh
    *
    * DecoratedMesh is a templated data structure for creating meshes with a custom amount of vertex
    * buffers. It uses a variadic set of BufferTraits to define its interface. For example, a Mesh with
    * a position and color per vertex would be defined as
    * `DecoratedMesh<PositionsBufferTrait,ColorsBufferTrait>`. Depending on the Traits specified in the
    * declaration the interface towards the class is updated: for example, a mesh with a position and
    * a color could be used as:
    *
    * \code{.cpp}
    * using MyMesh = DecoratedMesh<PositionsBufferTrait,ColorsBufferTrait>;
    * MyMesh mesh;
    * mesh.addVertex(vec3(0.0f), vec4(1,0,0,1) );
    * mesh.addVertex(vec3(1.0f), vec4(0,1,0,1) );
    * \endcode
    *
    * If texture coordinates is also needed for each vertex in the mesh then one could instead use:
    *
    * \code{.cpp}
    * using MyMesh = DecoratedMesh<PositionsBufferTrait, TexcoordBufferTrait, ColorsBufferTrait>;
    * MyMesh mesh;
    * mesh.addVertex(vec3(0.0f), vec3(0.0f), vec4(1,0,0,1) );
    * mesh.addVertex(vec3(1.0f), vec3(1.0f), vec4(0,1,0,1) );
    * \endcode
    *
    * For meshes when more than a few vertices is added is more efficient to add all vertices to the
    * mesh at the same time. This can be done by using a std::vector containing
    * DecoratedMesh::Vertex instead, as described by the following example:
    *
    * \code{.cpp}
    * using MyMesh = DecoratedMesh<PositionsBufferTrait,ColorsBufferTrait>;
    * std::vector<MyMesh::Vertex> vertices;
    * vertices.emplace_back(vec3(0.0f), vec4(1,0,0,1));
    * vertices.emplace_back(vec3(1.0f), vec4(0,1,0,1));
    * MyMesh mesh;
    * mesh.addVertices(vertices);
    * \endcode
    *
    * When creating meshes it is very common to also have Index buffers, in addition to the vertex
    * buffers. To add a index buffer to the mesh you can use the function addIndexBuffer as demonstrate
    * by the following example.
    *
    * \code{.cpp}
    * using MyMesh = DecoratedMesh<PositionsBufferTrait,ColorsBufferTrait>;
    * MyMesh mesh;
    * // Add vertices as above
    * auto ib = mesh.addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    * ib->add({0,1}); // Create a line between vertex 0 and 1
    * ib->add({1,2}); // Create another line between vertex 1 and 2
    * \endcode
    *
    */
template <typename... BufferTraits>
class DecoratedMesh : public Mesh {
public:
#if _MSC_VER <= 1900
    using Vertex = std::tuple<BufferTraits...>;
#else
    template <typename T>
    using TypeAlias = typename T::type;
    using Vertex = std::tuple<TypeAlias<BufferTraits>...>;
#endif

    DecoratedMesh() { addBuffersImpl<0, BufferTraits...>(); }
    DecoratedMesh(const DecoratedMesh &rhs) : Mesh(rhs) { copyConstrHelper<0, BufferTraits...>(); }
    DecoratedMesh &operator=(const DecoratedMesh &that) {
        if (this != &that) {
            Mesh::operator=(that);
            copyConstrHelper<0, BufferTraits...>();
        }
        return *this;
    }

    virtual DecoratedMesh *clone() const override { return new DecoratedMesh(*this); }

    virtual ~DecoratedMesh() {}

    void addVertices(const std::vector<Vertex> &vertices) {
        addVerticesImpl<0, BufferTraits...>(vertices);
    }
#if _MSC_VER <= 1900
    void addVertex(BufferTraits... args) { addVertexImpl<0>(args...); }
#else
    void addVertex(TypeAlias<BufferTraits>... args) { addVertexImpl<0>(args...); }
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

    template<typename BT>
    auto getTypedBuffer() {
        return getTypedBuffer<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
    }
    




    template <unsigned I>
    auto getTypedBuffer() const {
        return std::get<I>(buffers_).buffer_;
    }

    template<typename BT>
    auto getTypedBuffer() const {
        return getTypedBuffer<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
    }





    template <unsigned I>
    auto getTypedEditableRAMRepresentation() {
        return getTypedBuffer<I>()->getEditableRAMRepresentation();
    }


    template<typename BT>
    auto getEditableRAMRepresentation() {
        return getEditableRAMRepresentation<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
    }






    template <unsigned I>
    auto getTypedRAMRepresentation() const {
        return getTypedBuffer<I>()->getRAMRepresentation();
    }


    template<typename BT>
    auto getTypedRAMRepresentation() {
        return getTypedRAMRepresentation<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
    }






    template <unsigned I>
    auto &getTypedDataContainer() {
        return getTypedEditableRAMRepresentation<I>()->getDataContainer();
    }

    template<typename BT>
    auto &getTypedDataContainer() {
        return getTypedDataContainer<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
    }




    template <unsigned I>
    auto &getTypedDataContainer() const {
        return getTypedRAMRepresentation<I>()->getDataContainer();
    }


    template<typename BT>
    auto &getTypedDataContainer() const {
        return getTypedDataContainer<buffertraits::detail::get_index<BT,BufferTraits...>::value>();
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
            std::static_pointer_cast<Buffer<typename T::type>>(Mesh::buffers_[I].second);
        copyConstrHelper<I + 1, ARGS...>();
    }

private:
    template <typename T>
    struct BufferHolder {
        using BUF = inviwo::Buffer<typename T::type>;
        std::shared_ptr<BUF> buffer_{std::make_shared<BUF>()};
    };

    using BufferTuple = std::tuple<BufferHolder<BufferTraits>...>;
    BufferTuple buffers_;
};

using SphereMesh = DecoratedMesh<buffertraits::PositionsBuffer, buffertraits::RadiiBuffer,
                                 buffertraits::ColorsBuffer>;

using ColoredMesh = DecoratedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

}  // namespace inviwo

#endif  // IVW_DECORATEDMESH_H
