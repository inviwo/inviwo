/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/fmtutils.h>

#include <glm/matrix.hpp>

#include <iosfwd>

namespace inviwo {
// clang-format off
/**
 * This file is auto generated using tools/codegen/coordinatetransforms.nb
 *
 * Space   Description               Range
 * ===================================================================================
 * Data    raw data numbers          generally (-inf, inf), ([0,1] for textures)
 * Model   model space coordinates   (data min, data max)
 * World   world space coordinates   (-inf, inf)
 * View    view space coordinates    (-inf, inf)
 * Clip    clip space coordinates    [-1,1]
 * Index   voxel index coordinates   [0, number of voxels)
 *
 * From Space   To Space   Transform          Entity                           Member
 * ===================================================================================
 * Data         Model      ModelMatrix        const SpatialEntity<N>*          entity_
 * Model        World      WorldMatrix        const SpatialEntity<N>*          entity_
 * World        View       ViewMatrix         const CameraND<N>*               camera_
 * View         Clip       ProjectionMatrix   const CameraND<N>*               camera_
 * Data         Index      IndexMatrix        const StructuredGridEntity<N>*   entity_
 *
 *
 *  ┌───────────────────────────────────────────────────────────┐
 *  │                          Spatial                          │
 *  │               ModelM.               WorldM.               │
 *  │   ┌────────┐──────────▶┌────────┐───────────▶┌────────┐   │
 *  │   │        │           │        │            │        │   │
 *  │   │  Data  │           │ Model  │            │ World  │   │
 *  │   │        │ ModelM.-1 │        │  WorldM.-1 │        │   │
 *  │   └────────┘◀──────────└────────┘◀───────────└────────┘   │
 *  │   │        ▲                                 │        ▲   │
 *  └───┼────────┼─────────────────────────────────┼─────   ┼───┘
 *      │      I │                                 │      V │
 *    I │      n │                               V │      i │
 *    n │      d │                               i │      e │
 *    d │      e │                               e │      w │
 *    e │      x │                               w │      M │
 *    x │      M │                               M │      - │
 *    M │      - │                                 │      1 │
 *      │      1 │                                 │        │
 *  ┌───┼────────┼────┐                        ┌───┼────────┼────────────────────────────┐
 *  │   │        │    │                        │   │        │                            │
 *  │   ▼        │    │                        │   ▼        │  ProjectionM               │
 *  │   ┌────────┐    │                        │   ┌────────┐──────────────▶┌────────┐   │
 *  │   │        │    │                        │   │        │               │        │   │
 *  │   │ Index  │    │                        │   │  View  │               │  Clip  │   │
 *  │   │        │    │                        │   │        │ ProjectionM-1 │        │   │
 *  │   └────────┘    │                        │   └────────┘◀──────────────└────────┘   │
 *  │                 │                        │                                         │
 *  │   Structured    │                        │                 Camera                  │
 *  └─────────────────┘                        └─────────────────────────────────────────┘
 */

template <unsigned int N>
class SpatialEntity;

template <unsigned int N>
class StructuredGridEntity;

enum class CoordinateSpace {
    Data, Model, World, Index, Clip, View
};

namespace util {

class Camera2D {
public:
    Camera2D() : view_(1.0f), projection_(1.0f) {}
    const glm::mat3& getViewMatrix() const { return view_; }
    const glm::mat3& getProjectionMatrix() const { return projection_; }

private:
    glm::mat3 view_;
    glm::mat3 projection_;
};

template <unsigned int N>
struct cameratype {};

template <>
struct cameratype<2> {
    typedef Camera2D type;
};

template <>
struct cameratype<3> {
    typedef Camera type;
};

template <size_t N>
constexpr auto generateTransforms(std::array<CoordinateSpace, N> spaces) {
    static_assert(N > 0);
    std::array<std::pair<CoordinateSpace, CoordinateSpace>, N * (N-1)> res;
    size_t count = 0;
    for (auto from : spaces) {
        for (auto to : spaces) {
            if (from != to) {
                res[count].first = from;
                res[count].second = to;
                ++count;
            }
        }
    }
    return res;
}

} //  namespace util

template <unsigned int N>
using CameraND = typename util::cameratype<N>::type;

IVW_CORE_API std::string_view enumToStr(CoordinateSpace s);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, CoordinateSpace s);

template<unsigned int N>
class SpatialCoordinateTransformer {
public:
    virtual ~SpatialCoordinateTransformer() = default;
    virtual SpatialCoordinateTransformer<N>* clone() const = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual glm::mat<N + 1, N + 1, float> getMatrix(CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getModelToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to world space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getModelToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getDataToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getDataToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getWorldToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getWorldToDataMatrix() const = 0;
};

template<unsigned int N>
class StructuredCoordinateTransformer : public SpatialCoordinateTransformer<3> {
public:
    virtual ~StructuredCoordinateTransformer() = default;
    virtual StructuredCoordinateTransformer<N>* clone() const = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getModelToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getModelToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to voxel index coordinates, i.e. from (data min, data max) to [0, number of voxels)
     */
    virtual mat4 getModelToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to world space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual mat4 getModelToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual mat4 getDataToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual mat4 getTextureToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual mat4 getDataToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual mat4 getTextureToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getDataToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getTextureToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to model space coordinates, i.e. from [0, number of voxels) to (data min, data max)
     */
    virtual mat4 getIndexToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getIndexToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getIndexToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to world space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual mat4 getIndexToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual mat4 getWorldToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getWorldToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getWorldToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual mat4 getWorldToIndexMatrix() const = 0;
};

template<unsigned int N>
class SpatialCameraCoordinateTransformer : public SpatialCoordinateTransformer<N> {
public:
    virtual ~SpatialCameraCoordinateTransformer() = default;
    virtual SpatialCameraCoordinateTransformer<N>* clone() const = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual glm::mat<N + 1, N + 1, float> getMatrix(CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to model space coordinates, i.e. from [-1,1] to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getClipToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to raw data numbers, i.e. from [-1,1] to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getClipToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to view space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getClipToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to world space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getClipToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to clip space coordinates, i.e. from (data min, data max) to [-1,1]
     */
    virtual glm::mat<N+1, N+1, float> getModelToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getModelToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to view space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getModelToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to world space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getModelToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to clip space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [-1,1]
     */
    virtual glm::mat<N+1, N+1, float> getDataToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getDataToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to view space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getDataToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getDataToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual glm::mat<N+1, N+1, float> getViewToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getViewToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getViewToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to world space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getViewToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual glm::mat<N+1, N+1, float> getWorldToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual glm::mat<N+1, N+1, float> getWorldToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat<N+1, N+1, float> getWorldToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to view space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual glm::mat<N+1, N+1, float> getWorldToViewMatrix() const = 0;
};

template<unsigned int N>
class StructuredCameraCoordinateTransformer : public SpatialCameraCoordinateTransformer<3> {
public:
    virtual ~StructuredCameraCoordinateTransformer() = default;
    virtual StructuredCameraCoordinateTransformer<N>* clone() const = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to model space coordinates, i.e. from [-1,1] to (data min, data max)
     */
    virtual mat4 getClipToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to raw data numbers, i.e. from [-1,1] to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getClipToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to raw data numbers, i.e. from [-1,1] to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getClipToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to view space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual mat4 getClipToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to voxel index coordinates, i.e. from [-1,1] to [0, number of voxels)
     */
    virtual mat4 getClipToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to world space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual mat4 getClipToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to clip space coordinates, i.e. from (data min, data max) to [-1,1]
     */
    virtual mat4 getModelToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getModelToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getModelToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to view space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual mat4 getModelToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to voxel index coordinates, i.e. from (data min, data max) to [0, number of voxels)
     */
    virtual mat4 getModelToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to world space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual mat4 getModelToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to clip space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [-1,1]
     */
    virtual mat4 getDataToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to clip space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [-1,1]
     */
    virtual mat4 getTextureToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual mat4 getDataToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual mat4 getTextureToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to view space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getDataToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to view space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getTextureToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual mat4 getDataToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual mat4 getTextureToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getDataToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual mat4 getTextureToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual mat4 getViewToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual mat4 getViewToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getViewToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getViewToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual mat4 getViewToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to world space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual mat4 getViewToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to clip space coordinates, i.e. from [0, number of voxels) to [-1,1]
     */
    virtual mat4 getIndexToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to model space coordinates, i.e. from [0, number of voxels) to (data min, data max)
     */
    virtual mat4 getIndexToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getIndexToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getIndexToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to view space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual mat4 getIndexToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to world space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual mat4 getIndexToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual mat4 getWorldToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual mat4 getWorldToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getWorldToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual mat4 getWorldToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to view space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual mat4 getWorldToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual mat4 getWorldToIndexMatrix() const = 0;
};

#include <warn/push>
#include <warn/ignore/switch-enum>
template<unsigned int N>
glm::mat<N + 1, N + 1, float> SpatialCoordinateTransformer<N>::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}", from);
    }
}

template<unsigned int N>
mat4 StructuredCoordinateTransformer<N>::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::Data:
                    return getIndexToDataMatrix();
                case CoordinateSpace::Model:
                    return getIndexToModelMatrix();
                case CoordinateSpace::World:
                    return getIndexToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Index:
                    return getModelToIndexMatrix();
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Index:
                    return getWorldToIndexMatrix();
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}", from);
    }
}

template<unsigned int N>
glm::mat<N + 1, N + 1, float> SpatialCameraCoordinateTransformer<N>::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        case CoordinateSpace::View:
            switch (to) {
                case CoordinateSpace::Data:
                    return getViewToDataMatrix();
                case CoordinateSpace::Model:
                    return getViewToModelMatrix();
                case CoordinateSpace::World:
                    return getViewToWorldMatrix();
                case CoordinateSpace::View:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::View, to);
            }
        case CoordinateSpace::Clip:
            switch (to) {
                case CoordinateSpace::Data:
                    return getClipToDataMatrix();
                case CoordinateSpace::Model:
                    return getClipToModelMatrix();
                case CoordinateSpace::World:
                    return getClipToWorldMatrix();
                case CoordinateSpace::View:
                    return getClipToViewMatrix();
                case CoordinateSpace::Clip:
                    return glm::mat<N + 1, N + 1, float>(1.0f);
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}", from);
    }
}

template<unsigned int N>
mat4 StructuredCameraCoordinateTransformer<N>::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return mat4(1.0f);
                case CoordinateSpace::Data:
                    return getIndexToDataMatrix();
                case CoordinateSpace::Model:
                    return getIndexToModelMatrix();
                case CoordinateSpace::World:
                    return getIndexToWorldMatrix();
                case CoordinateSpace::View:
                    return getIndexToViewMatrix();
                case CoordinateSpace::Clip:
                    return getIndexToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return mat4(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Index:
                    return getModelToIndexMatrix();
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return mat4(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Index:
                    return getWorldToIndexMatrix();
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return mat4(1.0f);
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        case CoordinateSpace::View:
            switch (to) {
                case CoordinateSpace::Index:
                    return getViewToIndexMatrix();
                case CoordinateSpace::Data:
                    return getViewToDataMatrix();
                case CoordinateSpace::Model:
                    return getViewToModelMatrix();
                case CoordinateSpace::World:
                    return getViewToWorldMatrix();
                case CoordinateSpace::View:
                    return mat4(1.0f);
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::View, to);
            }
        case CoordinateSpace::Clip:
            switch (to) {
                case CoordinateSpace::Index:
                    return getClipToIndexMatrix();
                case CoordinateSpace::Data:
                    return getClipToDataMatrix();
                case CoordinateSpace::Model:
                    return getClipToModelMatrix();
                case CoordinateSpace::World:
                    return getClipToWorldMatrix();
                case CoordinateSpace::View:
                    return getClipToViewMatrix();
                case CoordinateSpace::Clip:
                    return mat4(1.0f);
                default:
                    throw Exception(IVW_CONTEXT, "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}", from);
    }
}
#include <warn/pop>


template<unsigned int N>
class SpatialCoordinateTransformerImpl : public SpatialCoordinateTransformer<N> {
public:
    SpatialCoordinateTransformerImpl(const SpatialEntity<N>& entity);
    SpatialCoordinateTransformerImpl(const SpatialCoordinateTransformerImpl<N>& rhs) = default;
    SpatialCoordinateTransformerImpl<N>& operator=(const SpatialCoordinateTransformerImpl<N>& that) = default;
    virtual SpatialCoordinateTransformerImpl<N>* clone() const;
    virtual ~SpatialCoordinateTransformerImpl(){}

    void setEntity(const SpatialEntity<N>& entity);

    virtual glm::mat<N+1, N+1, float> getDataToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToModelMatrix() const;

protected:
    virtual glm::mat<N+1, N+1, float> getModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldMatrix() const;

private:
    const SpatialEntity<N>* entity_;
};


template<unsigned int N>
class StructuredCoordinateTransformerImpl : public StructuredCoordinateTransformer<N> {
public:
    StructuredCoordinateTransformerImpl(const SpatialEntity<3>& entity);
    StructuredCoordinateTransformerImpl(const StructuredCoordinateTransformerImpl<N>& rhs) = default;
    StructuredCoordinateTransformerImpl<N>& operator=(const StructuredCoordinateTransformerImpl<N>& that) = default;
    virtual StructuredCoordinateTransformerImpl<N>* clone() const;
    virtual ~StructuredCoordinateTransformerImpl(){}

    void setEntity(const StructuredGridEntity<3>& entity);

    virtual glm::mat<N+1, N+1, float> getDataToIndexMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getIndexToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getIndexToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getIndexToTextureMatrix() const;
    virtual glm::mat<N+1, N+1, float> getIndexToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToIndexMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToTextureMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getTextureToIndexMatrix() const;
    virtual glm::mat<N+1, N+1, float> getTextureToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getTextureToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToIndexMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToTextureMatrix() const;

protected:
    virtual glm::mat<N+1, N+1, float> getIndexMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldMatrix() const;

private:
    const SpatialEntity<3>* entity_;
};


template<unsigned int N>
class SpatialCameraCoordinateTransformerImpl : public SpatialCameraCoordinateTransformer<N> {
public:
    SpatialCameraCoordinateTransformerImpl(const SpatialEntity<N>& entity, const CameraND<N>& camera);
    SpatialCameraCoordinateTransformerImpl(const SpatialCameraCoordinateTransformerImpl<N>& rhs) = default;
    SpatialCameraCoordinateTransformerImpl<N>& operator=(const SpatialCameraCoordinateTransformerImpl<N>& that) = default;
    virtual SpatialCameraCoordinateTransformerImpl<N>* clone() const;
    virtual ~SpatialCameraCoordinateTransformerImpl(){}

    void setEntity(const SpatialEntity<N>& entity);
    void setCamera(const CameraND<N>& camera);

    virtual glm::mat<N+1, N+1, float> getClipToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getClipToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getClipToViewMatrix() const;
    virtual glm::mat<N+1, N+1, float> getClipToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToClipMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToViewMatrix() const;
    virtual glm::mat<N+1, N+1, float> getDataToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToClipMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToViewMatrix() const;
    virtual glm::mat<N+1, N+1, float> getModelToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getViewToClipMatrix() const;
    virtual glm::mat<N+1, N+1, float> getViewToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getViewToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getViewToWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToClipMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToDataMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldToViewMatrix() const;

protected:
    virtual glm::mat<N+1, N+1, float> getModelMatrix() const;
    virtual glm::mat<N+1, N+1, float> getWorldMatrix() const;
    virtual glm::mat<N+1, N+1, float> getViewMatrix() const;
    virtual glm::mat<N+1, N+1, float> getProjectionMatrix() const;

private:
    const SpatialEntity<N>* entity_;
    const CameraND<N>* camera_;
};


template<unsigned int N>
class StructuredCameraCoordinateTransformerImpl : public StructuredCameraCoordinateTransformer<N> {
public:
    StructuredCameraCoordinateTransformerImpl(const SpatialEntity<3>& entity, const CameraND<3>& camera);
    StructuredCameraCoordinateTransformerImpl(const StructuredCameraCoordinateTransformerImpl<N>& rhs) = default;
    StructuredCameraCoordinateTransformerImpl<N>& operator=(const StructuredCameraCoordinateTransformerImpl<N>& that) = default;
    virtual StructuredCameraCoordinateTransformerImpl<N>* clone() const;
    virtual ~StructuredCameraCoordinateTransformerImpl(){}

    void setEntity(const SpatialEntity<3>& entity);
    void setCamera(const CameraND<3>& camera);

    virtual mat4 getClipToDataMatrix() const;
    virtual mat4 getClipToIndexMatrix() const;
    virtual mat4 getClipToModelMatrix() const;
    virtual mat4 getClipToTextureMatrix() const;
    virtual mat4 getClipToViewMatrix() const;
    virtual mat4 getClipToWorldMatrix() const;
    virtual mat4 getDataToClipMatrix() const;
    virtual mat4 getDataToIndexMatrix() const;
    virtual mat4 getDataToModelMatrix() const;
    virtual mat4 getDataToViewMatrix() const;
    virtual mat4 getDataToWorldMatrix() const;
    virtual mat4 getIndexToClipMatrix() const;
    virtual mat4 getIndexToDataMatrix() const;
    virtual mat4 getIndexToModelMatrix() const;
    virtual mat4 getIndexToTextureMatrix() const;
    virtual mat4 getIndexToViewMatrix() const;
    virtual mat4 getIndexToWorldMatrix() const;
    virtual mat4 getModelToClipMatrix() const;
    virtual mat4 getModelToDataMatrix() const;
    virtual mat4 getModelToIndexMatrix() const;
    virtual mat4 getModelToTextureMatrix() const;
    virtual mat4 getModelToViewMatrix() const;
    virtual mat4 getModelToWorldMatrix() const;
    virtual mat4 getTextureToClipMatrix() const;
    virtual mat4 getTextureToIndexMatrix() const;
    virtual mat4 getTextureToModelMatrix() const;
    virtual mat4 getTextureToViewMatrix() const;
    virtual mat4 getTextureToWorldMatrix() const;
    virtual mat4 getViewToClipMatrix() const;
    virtual mat4 getViewToDataMatrix() const;
    virtual mat4 getViewToIndexMatrix() const;
    virtual mat4 getViewToModelMatrix() const;
    virtual mat4 getViewToTextureMatrix() const;
    virtual mat4 getViewToWorldMatrix() const;
    virtual mat4 getWorldToClipMatrix() const;
    virtual mat4 getWorldToDataMatrix() const;
    virtual mat4 getWorldToIndexMatrix() const;
    virtual mat4 getWorldToModelMatrix() const;
    virtual mat4 getWorldToTextureMatrix() const;
    virtual mat4 getWorldToViewMatrix() const;

protected:
    virtual mat4 getIndexMatrix() const;
    virtual mat4 getModelMatrix() const;
    virtual mat4 getWorldMatrix() const;
    virtual mat4 getViewMatrix() const;
    virtual mat4 getProjectionMatrix() const;

private:
    const SpatialEntity<3>* entity_;
    const CameraND<3>* camera_;
};


/*********************************************************************************
 *  Implementations
 *  SpatialCoordinateTransformerImpl
 *********************************************************************************/

template<unsigned int N>
SpatialCoordinateTransformerImpl<N>::SpatialCoordinateTransformerImpl(const SpatialEntity<N>& entity)
    : SpatialCoordinateTransformer<N>()
    , entity_{&entity} {}

template<unsigned int N>
SpatialCoordinateTransformerImpl<N>* SpatialCoordinateTransformerImpl<N>::clone() const {
    return new SpatialCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

template <unsigned int N>
void SpatialCoordinateTransformerImpl<N>::setEntity(const SpatialEntity<N>& entity) {
    entity_ = &entity;
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}


/*********************************************************************************
 *  Implementations
 *  StructuredCoordinateTransformerImpl
 *********************************************************************************/

template<unsigned int N>
StructuredCoordinateTransformerImpl<N>::StructuredCoordinateTransformerImpl(const SpatialEntity<3>& entity)
    : StructuredCoordinateTransformer<N>()
    , entity_{&entity} {}

template<unsigned int N>
StructuredCoordinateTransformerImpl<N>* StructuredCoordinateTransformerImpl<N>::clone() const {
    return new StructuredCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getIndexMatrix() const {
    return entity_->getIndexMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

template <unsigned int N>
void StructuredCoordinateTransformerImpl<N>::setEntity(const StructuredGridEntity<3>& entity) {
    entity_ = &entity;
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getDataToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getIndexToDataMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getIndexToModelMatrix() const {
    return getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getIndexToTextureMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getIndexToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getModelToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getModelToTextureMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getTextureToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getTextureToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getTextureToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getWorldToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> StructuredCoordinateTransformerImpl<N>::getWorldToTextureMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}


/*********************************************************************************
 *  Implementations
 *  SpatialCameraCoordinateTransformerImpl
 *********************************************************************************/

template<unsigned int N>
SpatialCameraCoordinateTransformerImpl<N>::SpatialCameraCoordinateTransformerImpl(const SpatialEntity<N>& entity, const CameraND<N>& camera)
    : SpatialCameraCoordinateTransformer<N>()
    , entity_{&entity}
    , camera_{&camera} {}

template<unsigned int N>
SpatialCameraCoordinateTransformerImpl<N>* SpatialCameraCoordinateTransformerImpl<N>::clone() const {
    return new SpatialCameraCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getViewMatrix() const {
    return camera_->getViewMatrix();
}
template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getProjectionMatrix() const {
    return camera_->getProjectionMatrix();
}

template <unsigned int N>
void SpatialCameraCoordinateTransformerImpl<N>::setEntity(const SpatialEntity<N>& entity) {
    entity_ = &entity;
}
template <unsigned int N>
void SpatialCameraCoordinateTransformerImpl<N>::setCamera(const CameraND<N>& camera) {
    camera_ = &camera;
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getClipToDataMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getClipToModelMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getClipToViewMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getClipToWorldMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getDataToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getDataToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getModelToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getModelToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getViewToClipMatrix() const {
    return getProjectionMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getViewToDataMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getViewToModelMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getViewToWorldMatrix() const {
    return glm::inverse(getViewMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getWorldToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix();
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

template <unsigned int N>
glm::mat<N+1, N+1, float> SpatialCameraCoordinateTransformerImpl<N>::getWorldToViewMatrix() const {
    return getViewMatrix();
}


/*********************************************************************************
 *  Implementations
 *  StructuredCameraCoordinateTransformerImpl
 *********************************************************************************/

template<unsigned int N>
StructuredCameraCoordinateTransformerImpl<N>::StructuredCameraCoordinateTransformerImpl(const SpatialEntity<3>& entity, const CameraND<3>& camera)
    : StructuredCameraCoordinateTransformer<N>()
    , entity_{&entity}
    , camera_{&camera} {}

template<unsigned int N>
StructuredCameraCoordinateTransformerImpl<N>* StructuredCameraCoordinateTransformerImpl<N>::clone() const {
    return new StructuredCameraCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexMatrix() const {
    return entity_->getIndexMatrix();
}
template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}
template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewMatrix() const {
    return camera_->getViewMatrix();
}
template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getProjectionMatrix() const {
    return camera_->getProjectionMatrix();
}

template <unsigned int N>
void StructuredCameraCoordinateTransformerImpl<N>::setEntity(const SpatialEntity<3>& entity) {
    entity_ = &entity;
}
template <unsigned int N>
void StructuredCameraCoordinateTransformerImpl<N>::setCamera(const CameraND<3>& camera) {
    camera_ = &camera;
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToDataMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToModelMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToTextureMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToViewMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToWorldMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToDataMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToModelMatrix() const {
    return getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToTextureMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToTextureMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToClipMatrix() const {
    return getProjectionMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToDataMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToModelMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToTextureMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToWorldMatrix() const {
    return glm::inverse(getViewMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix();
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToTextureMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToViewMatrix() const {
    return getViewMatrix();
}

// clang-format on
}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::CoordinateSpace> : inviwo::FlagFormatter<inviwo::CoordinateSpace> {};
#endif
