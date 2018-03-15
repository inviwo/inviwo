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

#ifndef IVW_TYPEDMESH_H
#define IVW_TYPEDMESH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <tuple>

namespace inviwo {

/**
 * \defgroup typedmesh TypedMesh
 * \ingroup datastructures
 *
 * \copydetails TypedMesh
 */

/**
 * Namespace for buffer traits
 * \ingroup typedmesh
 */
namespace buffertraits {

namespace detail {
template <typename T, typename... Ts>
struct get_index;

template <typename T, typename... Ts>
struct get_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename Tail, typename... Ts>
struct get_index<T, Tail, Ts...>
    : std::integral_constant<std::size_t, 1 + get_index<T, Ts...>::value> {};

}  // namespace detail

/**
 * \ingroup typedmesh
 * Struct used to tell TypedMesh what buffers to create.
 * The first two template parameters, 'typename T, unsigned DIM' is used rather than
 * ´´´Vector<DIM,T>´´´ or glm::TvecX due to compiler issues related to the clone function.
 * Similarly, The template parameter ´´´int attrib´´´ needs can't be a strongly typed enum
 * (inviwo::BufferType) and hence it is an int.
 *
 * \see TypedMesh
 * \see typedmesh
 */
template <typename T, unsigned DIM, int attrib, int location = attrib>
class BufferTrait {
public:
    using type = Vector<DIM, T>;
    static inviwo::Mesh::BufferInfo bi() {
        return {static_cast<inviwo::BufferType>(attrib), location};
    }

#if _MSC_VER <= 1900
    BufferTrait(type t) : t(t) {}
    type t;
    operator type() const { return t; }
#endif

    BufferTrait() {}
    virtual ~BufferTrait() = default;

    BufferTrait(Mesh &mesh) { mesh.addBuffer(bi(), buffer_); }

    // Convenience functions
    auto getTypedRAMRepresentation() const { return buffer_->getRAMRepresentation(); }
    auto getTypedEditableRAMRepresentation() { return buffer_->getEditableRAMRepresentation(); }
    auto &getTypedDataContainer() const { return getTypedRAMRepresentation()->getDataContainer(); }
    auto &getTypedDataContainer() {
        return getTypedEditableRAMRepresentation()->getDataContainer();
    }

    std::shared_ptr<Buffer<type>> buffer_{std::make_shared<Buffer<type>>()};
};

/**
 * \ingroup typedmesh
 * BufferTrait for Position buffers (glm::vec3)
 */
class PositionsBuffer : public BufferTrait<float, 3, static_cast<int>(BufferType::PositionAttrib)> {
public:
    using Base = BufferTrait<float, 3, static_cast<int>(BufferType::PositionAttrib)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getVertices() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableVertices() { return Base::buffer_; }

    void setVertexPosition(size_t index, vec3 pos){
        getEditableVertices()->getEditableRAMRepresentation()->set(index,pos);
    }
};

/**
 * \ingroup typedmesh
 * BufferTrait for Normal buffers (glm::vec3)
 */
class NormalBuffer : public BufferTrait<float, 3, static_cast<int>(BufferType::NormalAttrib)> {
public:
    using Base = BufferTrait<float, 3, static_cast<int>(BufferType::NormalAttrib)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getNormals() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableNormals() { return Base::buffer_; }

    void setVertexNormal(size_t index, vec3 normal){
        getEditableNormals()->getEditableRAMRepresentation()->set(index,normal);
    }
};

/**
 * \ingroup typedmesh
 * BufferTrait for Colors buffers (glm::vec4)
 */
class ColorsBuffer : public BufferTrait<float, 4, static_cast<int>(BufferType::ColorAttrib)> {
public:
    using Base = BufferTrait<float, 4, static_cast<int>(BufferType::ColorAttrib)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getColors() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableColors() { return Base::buffer_; }

    void setVertexColor(size_t index, vec4 color){
        getEditableColors()->getEditableRAMRepresentation()->set(index,color);
    }
};

/**
 * \ingroup typedmesh
 * BufferTrait for Texture Coordinate buffers (glm::vec3)
 */
class TexcoordBuffer : public BufferTrait<float, 3, static_cast<int>(BufferType::TexcoordAttrib)> {
public:
    using Base = BufferTrait<float, 3, static_cast<int>(BufferType::TexcoordAttrib)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getTexCoords() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableTexCoords() { return Base::buffer_; }

    void setVertexTexCoord(size_t index, vec3 texCoord){
        getEditableTexCoords()->getEditableRAMRepresentation()->set(index,texCoord);
    }
};

/**
 * \ingroup typedmesh
 * BufferTrait for Curvature buffers (float)
 */
class CurvatureBuffer
    : public BufferTrait<float, 1, static_cast<int>(BufferType::CurvatureAttrib)> {
public:
    using Base = BufferTrait<float, 1, static_cast<int>(BufferType::CurvatureAttrib)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getCurvatures() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableCurvatures() { return Base::buffer_; }

    void setVertexCurvature(size_t index, float curvature){
        getEditableCurvatures()->getEditableRAMRepresentation()->set(index,curvature);
    }
};

/**
 * \ingroup typedmesh
 * BufferTrait for Uint32 buffers
 */
using IndexBuffer = BufferTrait<uint32_t, 1, static_cast<int>(BufferType::IndexAttrib)>;

/**
 * \ingroup typedmesh
 * BufferTrait for radii buffers (float)
 * \see SphereMesh
 */
class RadiiBuffer
    : public BufferTrait<float, 1, static_cast<int>(BufferType::NumberOfBufferTypes)> {
public:
    using Base = BufferTrait<float, 1, static_cast<int>(BufferType::NumberOfBufferTypes)>;
    using Base::Base;

    std::shared_ptr<const Buffer<type>> getRadii() const { return Base::buffer_; }
    std::shared_ptr<Buffer<type>> getEditableRadii() { return Base::buffer_; }

    void setVertexRadius(size_t index, float radius){
        getEditableRadii()->getEditableRAMRepresentation()->set(index,radius);
    }
};
}  // namespace buffertraits

/**
 * \class TypedMesh
 * \ingroup datastructures
 * \ingroup typedmesh
 *
 *
 *  TypedMesh is a templated data structure for creating meshes with a custom amount of vertex
 * buffers. It uses a variadic set of BufferTraits to define its interface. For example, a Mesh with
 * a position and color per vertex would be defined as
 * `TypedMesh<PositionsBufferTrait,ColorsBufferTrait>`. Depending on the Traits specified in the
 * declaration the interface towards the class is updated.
 *
 * # Simple Usage
 *
 * \code{.cpp}
 * using MyMesh = TypedMesh<buffertraits::PositionsBuffer,buffertraits::ColorsBuffer>;
 * MyMesh mesh;
 * mesh.addVertex(vec3(0.0f), vec4(1,0,0,1) );
 * mesh.addVertex(vec3(1.0f), vec4(0,1,0,1) );
 * \endcode
 *
 * If texture coordinates is also needed for each vertex in the mesh then one could instead use:
 *
 * \code{.cpp}
 * using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::TexcoordBuffer,
 * buffertraits::ColorsBuffer>; MyMesh mesh; mesh.addVertex(vec3(0.0f), vec3(0.0f), vec4(1,0,0,1) );
 * mesh.addVertex(vec3(1.0f), vec3(1.0f), vec4(0,1,0,1) );
 * \endcode
 *
 * For meshes when more than a few vertices is added is more efficient to add all vertices to the
 * mesh at the same time. This can be done by using a std::vector containing
 * TypedMesh::Vertex instead, as described by the following example:
 *
 * \code{.cpp}
 * using MyMesh = TypedMesh<PositionsBufferTrait,ColorsBufferTrait>;
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
 * using MyMesh = TypedMesh<buffertraits::PositionsBuffer,buffertraits::ColorsBuffer>;
 * MyMesh mesh;
 * // Add vertices as above
 * auto ib = mesh.addIndexBuffer(DrawType::Lines, ConnectivityType::None);
 * ib->add({0,1}); // Create a line between vertex 0 and 1
 * ib->add({1,2}); // Create another line between vertex 1 and 2
 * \endcode
 *
 * # Examples
 * ## Creating a bounding box with Adjacency information
 * The following code snippet uses a SimpleMesh2 to create bounding box for a given basisandoffset
 * matrix. It is the code used in meshutil::boundingBoxAdjacency
 *
 * \snippet modules/base/algorithm/meshutils.cpp Using Simple Mesh
 *
 *
 * ## Creating camera frustum
 * The following code snippet is another example where we create a camera frustum mesh for a given
 * camera. It is the code used in meshutil::cameraFrustum
 *
 * \snippet modules/base/algorithm/meshutils.cpp Using Colored Mesh
 *
 */

// TypedMesh<buffertraits::IndexBuffer,buffertraits::PositionBuffer> test;

template <typename... BufferTraits>
class TypedMesh : public Mesh, public BufferTraits... {
public:
#if defined(_MSC_VER) && _MSC_VER <= 1900
    using Vertex = std::tuple<BufferTraits...>;
#else
    template <typename T>
    using TypeAlias = typename T::type;

    using VertexTuple = std::tuple<TypeAlias<BufferTraits>...>;
    class Vertex : public VertexTuple {
    public:
        Vertex() = default;
        Vertex(TypeAlias<BufferTraits>... vals) : VertexTuple({vals...}) { }
    };
#endif

    TypedMesh(DrawType dt = DrawType::Points, ConnectivityType ct = ConnectivityType::None)
        : Mesh(dt, ct), BufferTraits(*static_cast<Mesh *>(this))... {}

    TypedMesh(const TypedMesh &rhs) : Mesh(rhs) { copyConstrHelper<0, BufferTraits...>(); }
    TypedMesh &operator=(const TypedMesh &that) {
        if (this != &that) {
            Mesh::operator=(that);
            copyConstrHelper<0, BufferTraits...>();
        }
        return *this;
    }

    virtual TypedMesh *clone() const override { return new TypedMesh(*this); }

    virtual ~TypedMesh() = default;

    void append(const TypedMesh *mesh) {
        if (mesh) Mesh::append(*mesh);
    }

    void addVertices(const std::vector<Vertex> &vertices) {
        addVerticesImpl<0, BufferTraits...>(vertices);
    }

#if defined(_MSC_VER) && _MSC_VER <= 1900
    uint32_t addVertex(BufferTraits... args) {
        using BT = typename std::tuple_element<0, std::tuple<BufferTraits...>>::type;
        addVertexImpl<0>(args...);
        return static_cast<uint32_t>(getTypedBuffer<BT>()->getSize() - 1);
    }

    void setVertex(size_t index, BufferTraits... args) { setVertiesImpl<0>(index, args...); }
#else
    uint32_t addVertex(TypeAlias<BufferTraits>... args) {
        using BT = typename std::tuple_element<0, std::tuple<BufferTraits...>>::type;
        addVertexImpl<0>(args...);
        return static_cast<uint32_t>(getTypedBuffer<BT>()->getSize() - 1);
    }

    void setVertex(size_t index, TypeAlias<BufferTraits>... args) {
        setVertiesImpl<0>(index, args...);
    }
#endif

    template <typename BT>
    void setVertex(size_t index, const typename BT::type &v) {
        getTypedDataContainer<BT>().at(index) = v;
    }

    std::shared_ptr<IndexBufferRAM> addIndexBuffer(DrawType dt, ConnectivityType ct) {
        auto indicesRam = std::make_shared<IndexBufferRAM>();
        auto indices = std::make_shared<IndexBuffer>(indicesRam);
        addIndicies(Mesh::MeshInfo(dt, ct), indices);
        return indicesRam;
    }

    template <typename BT>
    auto getTypedBuffer() {
        return BT::buffer_;
    }

    template <typename BT>
    auto getTypedBuffer() const {
        return BT::buffer_;
    }

    template <typename BT>
    auto getTypedRAMRepresentation() const {
        return BT::getTypedRAMRepresentation();
    }

    template <typename BT>
    auto getTypedEditableRAMRepresentation() {
        return BT::getTypedEditableRAMRepresentation();
    }

    template <typename BT>
    auto &getTypedDataContainer() const {
        return BT::getTypedDataContainer();
    }

    template <typename BT>
    auto &getTypedDataContainer() {
        return BT::getTypedDataContainer();
    }

private:
    template <unsigned I, typename T>
    void addVertexImpl(T &t) {
        using BT = typename std::tuple_element<I, std::tuple<BufferTraits...>>::type;
        BT::getTypedEditableRAMRepresentation()->add(t);
    }

    template <unsigned I, typename T, typename... ARGS>
    void addVertexImpl(T &t, ARGS... args) {
        using BT = typename std::tuple_element<I, std::tuple<BufferTraits...>>::type;
        BT::getTypedEditableRAMRepresentation()->add(t);
        addVertexImpl<I + 1>(args...);
    }

    template <unsigned I, typename T>
    void setVertiesImpl(size_t index, T &t) {
        using BT = typename std::tuple_element<I, std::tuple<BufferTraits...>>::type;
        BT::getTypedDataContainer().at(index) = t;
    }

    template <unsigned I, typename T, typename... ARGS>
    void setVertiesImpl(size_t index, T &t, ARGS... args) {
        using BT = typename std::tuple_element<I, std::tuple<BufferTraits...>>::type;
        BT::getTypedDataContainer().at(index) = t;
        setVertiesImpl<I + 1>(index, args...);
    }

    template <unsigned I>
    void addVerticesImpl(const std::vector<Vertex> &vertices) {}  // sink

    template <unsigned I, typename T, typename... ARGS>
    void addVerticesImpl(const std::vector<Vertex> &vertices) {

        auto &vec = getTypedDataContainer<T>();

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
        T::buffer_ = std::static_pointer_cast<Buffer<typename T::type>>(Mesh::buffers_[I].second);
        copyConstrHelper<I + 1, ARGS...>();
    }
};

/**
 * \ingroup typedmesh
 * Type definition of a TypedMesh useful for Spheres, consists of a vec3-buffer for position, a
 * float-buffer for radii and vec4 for colors.
 */
using SphereMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::RadiiBuffer,
                                 buffertraits::ColorsBuffer>;

/**
 * \ingroup typedmesh
 * Type definition of a TypedMesh having only positions(vec3) and colors(vec4).
 * Example usage:
 * \snippet modules/base/algorithm/meshutils.cpp Using Colored Mesh
 */
using ColoredMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;


/**
 * \ingroup typedmesh
 * Type definition of a TypedMesh having positions(vec3), normals(vec3), texture
 * coordinates(vec3) and colors(vec4). Example usage:
 */
using BasicMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::NormalBuffer,
                                buffertraits::TexcoordBuffer, buffertraits::ColorsBuffer>;




using NewSimpleMesh = TypedMesh<buffertraits::PositionsBuffer,
    buffertraits::TexcoordBuffer, buffertraits::ColorsBuffer>;
}  // namespace inviwo

#endif  // IVW_TYPEDMESH_H
