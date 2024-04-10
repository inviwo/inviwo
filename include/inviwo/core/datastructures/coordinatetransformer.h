/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
 * Data         Model      ModelMatrix        const SpatialEntity*             entity_
 * Model        World      WorldMatrix        const SpatialEntity*             entity_
 * World        View       ViewMatrix         const Camera*                    camera_
 * View         Clip       ProjectionMatrix   const Camera*                    camera_
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

enum class CoordinateSpace {
    Data, Model, World, Index, Clip, View
};

namespace util {

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

IVW_CORE_API std::string_view enumToStr(CoordinateSpace s);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, CoordinateSpace s);


class IVW_CORE_API SpatialCoordinateTransformer {
public:
    SpatialCoordinateTransformer() = default;
    virtual ~SpatialCoordinateTransformer() = default;
    virtual SpatialCoordinateTransformer* clone() const = 0;
    /**
     * Returns the matrix transformation mapping from \p from coordinates
     * to \p to coordinates
     */
    virtual glm::mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getModelToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to world space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual glm::mat4 getModelToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual glm::mat4 getDataToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat4 getDataToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual glm::mat4 getWorldToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getWorldToDataMatrix() const = 0;
    /**
     * Transforms the given position \p pos from \p from coordinates to \p to coordinates. The
     * resulting position is divided by w.
     */
    virtual glm::vec3 transformPosition(const vec3& pos, CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Transforms the given position \p pos from \p from coordinates to \p to coordinates using homogeneous coordinates
     */
    virtual glm::vec4 transformPositionHomogeneous(const vec4& pos, CoordinateSpace from, CoordinateSpace to) const;
    /**
     * Transforms the given \p normal from \p from coordinates to \p to coordinates. Only considers
     * transformations between supported by this SpatialCoordinateTransformer. That is Data to
     * Model, Model to World, Data to World and their inverse, camera or index coordinates are
     * not supported.
     */
    virtual glm::vec3 transformNormal(const vec3& normal, CoordinateSpace from, CoordinateSpace to) const;

protected:
    SpatialCoordinateTransformer(const SpatialCoordinateTransformer&) = default;
    SpatialCoordinateTransformer(SpatialCoordinateTransformer&&) = delete;
    SpatialCoordinateTransformer& operator=(const SpatialCoordinateTransformer&) = delete;
    SpatialCoordinateTransformer& operator=(SpatialCoordinateTransformer&&) = delete;
};

class IVW_CORE_API StructuredCoordinateTransformer : public SpatialCoordinateTransformer {
public:
    StructuredCoordinateTransformer() = default;
    virtual ~StructuredCoordinateTransformer() = default;
    virtual StructuredCoordinateTransformer* clone() const override = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual glm::mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const override;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getModelToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to voxel index coordinates, i.e. from (data min, data max) to [0, number of voxels)
     */
    virtual glm::mat4 getModelToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual glm::mat4 getTextureToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual glm::mat4 getDataToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual glm::mat4 getTextureToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat4 getTextureToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to model space coordinates, i.e. from [0, number of voxels) to (data min, data max)
     */
    virtual glm::mat4 getIndexToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getIndexToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getIndexToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to world space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual glm::mat4 getIndexToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getWorldToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual glm::mat4 getWorldToIndexMatrix() const = 0;
    
protected:
    StructuredCoordinateTransformer(const StructuredCoordinateTransformer&) = default;
    StructuredCoordinateTransformer(StructuredCoordinateTransformer&&) = delete;
    StructuredCoordinateTransformer& operator=(const StructuredCoordinateTransformer&) = delete;
    StructuredCoordinateTransformer& operator=(StructuredCoordinateTransformer&&) = delete;
};

class IVW_CORE_API SpatialCameraCoordinateTransformer : public SpatialCoordinateTransformer {
public:
    SpatialCameraCoordinateTransformer() = default;
    virtual ~SpatialCameraCoordinateTransformer() = default;
    virtual SpatialCameraCoordinateTransformer* clone() const override = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual glm::mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const override;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to model space coordinates, i.e. from [-1,1] to (data min, data max)
     */
    virtual glm::mat4 getClipToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to raw data numbers, i.e. from [-1,1] to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getClipToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to view space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual glm::mat4 getClipToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to world space coordinates, i.e. from [-1,1] to (-inf, inf)
     */
    virtual glm::mat4 getClipToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to clip space coordinates, i.e. from (data min, data max) to [-1,1]
     */
    virtual glm::mat4 getModelToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to view space coordinates, i.e. from (data min, data max) to (-inf, inf)
     */
    virtual glm::mat4 getModelToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to clip space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [-1,1]
     */
    virtual glm::mat4 getDataToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to view space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat4 getDataToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual glm::mat4 getViewToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to model space coordinates, i.e. from (-inf, inf) to (data min, data max)
     */
    virtual glm::mat4 getViewToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getViewToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to world space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual glm::mat4 getViewToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to clip space coordinates, i.e. from (-inf, inf) to [-1,1]
     */
    virtual glm::mat4 getWorldToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to view space coordinates, i.e. from (-inf, inf) to (-inf, inf)
     */
    virtual glm::mat4 getWorldToViewMatrix() const = 0;

protected:
    SpatialCameraCoordinateTransformer(const SpatialCameraCoordinateTransformer&) = default;
    SpatialCameraCoordinateTransformer(SpatialCameraCoordinateTransformer&&) = delete;
    SpatialCameraCoordinateTransformer& operator=(const SpatialCameraCoordinateTransformer&) = delete;
    SpatialCameraCoordinateTransformer& operator=(SpatialCameraCoordinateTransformer&&) = delete;
};

class IVW_CORE_API StructuredCameraCoordinateTransformer : public SpatialCameraCoordinateTransformer {
public:
    StructuredCameraCoordinateTransformer() = default;
    virtual ~StructuredCameraCoordinateTransformer() = default;
    
    virtual StructuredCameraCoordinateTransformer* clone() const override = 0;
    /**
     * Returns the matrix transformation mapping from "from" coordinates
     * to "to" coordinates
     */
    virtual glm::mat4 getMatrix(CoordinateSpace from, CoordinateSpace to) const override;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to raw data numbers, i.e. from [-1,1] to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getClipToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from clip space coordinates
     * to voxel index coordinates, i.e. from [-1,1] to [0, number of voxels)
     */
    virtual glm::mat4 getClipToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to raw data numbers, i.e. from (data min, data max) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getModelToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from model space coordinates
     * to voxel index coordinates, i.e. from (data min, data max) to [0, number of voxels)
     */
    virtual glm::mat4 getModelToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to clip space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [-1,1]
     */
    virtual glm::mat4 getTextureToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to model space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (data min, data max)
     */
    virtual glm::mat4 getTextureToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to view space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat4 getTextureToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual glm::mat4 getDataToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to voxel index coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to [0, number of voxels)
     */
    virtual glm::mat4 getTextureToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from raw data numbers
     * to world space coordinates, i.e. from generally (-inf, inf), ([0,1] for textures) to (-inf, inf)
     */
    virtual glm::mat4 getTextureToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getViewToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from view space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual glm::mat4 getViewToIndexMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to clip space coordinates, i.e. from [0, number of voxels) to [-1,1]
     */
    virtual glm::mat4 getIndexToClipMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to model space coordinates, i.e. from [0, number of voxels) to (data min, data max)
     */
    virtual glm::mat4 getIndexToModelMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getIndexToDataMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to raw data numbers, i.e. from [0, number of voxels) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getIndexToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to view space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual glm::mat4 getIndexToViewMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from voxel index coordinates
     * to world space coordinates, i.e. from [0, number of voxels) to (-inf, inf)
     */
    virtual glm::mat4 getIndexToWorldMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to raw data numbers, i.e. from (-inf, inf) to generally (-inf, inf), ([0,1] for textures)
     */
    virtual glm::mat4 getWorldToTextureMatrix() const = 0;
    /**
     * Returns the matrix transformation mapping from world space coordinates
     * to voxel index coordinates, i.e. from (-inf, inf) to [0, number of voxels)
     */
    virtual glm::mat4 getWorldToIndexMatrix() const = 0;
    
protected:
    StructuredCameraCoordinateTransformer(const StructuredCameraCoordinateTransformer&) = default;
    StructuredCameraCoordinateTransformer(StructuredCameraCoordinateTransformer&&) = delete;
    StructuredCameraCoordinateTransformer& operator=(const StructuredCameraCoordinateTransformer&) = delete;
    StructuredCameraCoordinateTransformer& operator=(StructuredCameraCoordinateTransformer&&) = delete;
};

class SpatialEntity;

class IVW_CORE_API SpatialCoordinateTransformerImpl : public SpatialCoordinateTransformer {
public:
    SpatialCoordinateTransformerImpl(const SpatialEntity& entity);

    virtual SpatialCoordinateTransformerImpl* clone() const;
    virtual ~SpatialCoordinateTransformerImpl() = default;

    void setEntity(const SpatialEntity& entity);

    virtual glm::mat4 getDataToModelMatrix() const;
    virtual glm::mat4 getDataToWorldMatrix() const;
    virtual glm::mat4 getModelToDataMatrix() const;
    virtual glm::mat4 getModelToWorldMatrix() const;
    virtual glm::mat4 getWorldToDataMatrix() const;
    virtual glm::mat4 getWorldToModelMatrix() const;

protected:
    SpatialCoordinateTransformerImpl(const SpatialCoordinateTransformerImpl& rhs) = default;
    SpatialCoordinateTransformerImpl(SpatialCoordinateTransformerImpl&& rhs) = delete;
    SpatialCoordinateTransformerImpl& operator=(const SpatialCoordinateTransformerImpl& that) = delete;
    SpatialCoordinateTransformerImpl& operator=(SpatialCoordinateTransformerImpl&& that) = delete;
    
    virtual glm::mat4 getModelMatrix() const;
    virtual glm::mat4 getWorldMatrix() const;

private:
    const SpatialEntity* entity_;
};

template <unsigned int N>
class StructuredGridEntity;

template <unsigned int N>
class StructuredCoordinateTransformerImpl : public StructuredCoordinateTransformer {
public:
    StructuredCoordinateTransformerImpl(const StructuredGridEntity<N>& entity);
    virtual StructuredCoordinateTransformerImpl* clone() const override;
    virtual ~StructuredCoordinateTransformerImpl() = default;

    void setEntity(const StructuredGridEntity<N>& entity);

    virtual glm::mat4 getDataToIndexMatrix() const override;
    virtual glm::mat4 getDataToModelMatrix() const override;
    virtual glm::mat4 getDataToWorldMatrix() const override;
    virtual glm::mat4 getIndexToDataMatrix() const override;
    virtual glm::mat4 getIndexToModelMatrix() const override;
    virtual glm::mat4 getIndexToTextureMatrix() const override;
    virtual glm::mat4 getIndexToWorldMatrix() const override;
    virtual glm::mat4 getModelToDataMatrix() const override;
    virtual glm::mat4 getModelToIndexMatrix() const override;
    virtual glm::mat4 getModelToTextureMatrix() const override;
    virtual glm::mat4 getModelToWorldMatrix() const override;
    virtual glm::mat4 getTextureToIndexMatrix() const override;
    virtual glm::mat4 getTextureToModelMatrix() const override;
    virtual glm::mat4 getTextureToWorldMatrix() const override;
    virtual glm::mat4 getWorldToDataMatrix() const override;
    virtual glm::mat4 getWorldToIndexMatrix() const override;
    virtual glm::mat4 getWorldToModelMatrix() const override;
    virtual glm::mat4 getWorldToTextureMatrix() const override;

protected:
    StructuredCoordinateTransformerImpl(const StructuredCoordinateTransformerImpl& rhs) = default;
    StructuredCoordinateTransformerImpl( StructuredCoordinateTransformerImpl&& rhs) = delete;
    StructuredCoordinateTransformerImpl& operator=(const StructuredCoordinateTransformerImpl& that) = delete;
    StructuredCoordinateTransformerImpl& operator=( StructuredCoordinateTransformerImpl&& that) = delete;

    virtual glm::mat4 getIndexMatrix() const;
    virtual glm::mat4 getModelMatrix() const;
    virtual glm::mat4 getWorldMatrix() const;

private:
    const StructuredGridEntity<N>* entity_;
};


class IVW_CORE_API SpatialCameraCoordinateTransformerImpl : public SpatialCameraCoordinateTransformer {
public:
    SpatialCameraCoordinateTransformerImpl(const SpatialEntity& entity, const Camera& camera);
    virtual SpatialCameraCoordinateTransformerImpl* clone() const override;
    virtual ~SpatialCameraCoordinateTransformerImpl() = default;

    void setEntity(const SpatialEntity& entity);
    void setCamera(const Camera& camera);

    virtual glm::mat4 getClipToDataMatrix() const override;
    virtual glm::mat4 getClipToModelMatrix() const override;
    virtual glm::mat4 getClipToViewMatrix() const override;
    virtual glm::mat4 getClipToWorldMatrix() const override;
    virtual glm::mat4 getDataToClipMatrix() const override;
    virtual glm::mat4 getDataToModelMatrix() const override;
    virtual glm::mat4 getDataToViewMatrix() const override;
    virtual glm::mat4 getDataToWorldMatrix() const override;
    virtual glm::mat4 getModelToClipMatrix() const override;
    virtual glm::mat4 getModelToDataMatrix() const override;
    virtual glm::mat4 getModelToViewMatrix() const override;
    virtual glm::mat4 getModelToWorldMatrix() const override;
    virtual glm::mat4 getViewToClipMatrix() const override;
    virtual glm::mat4 getViewToDataMatrix() const override;
    virtual glm::mat4 getViewToModelMatrix() const override;
    virtual glm::mat4 getViewToWorldMatrix() const override;
    virtual glm::mat4 getWorldToClipMatrix() const override;
    virtual glm::mat4 getWorldToDataMatrix() const override;
    virtual glm::mat4 getWorldToModelMatrix() const override;
    virtual glm::mat4 getWorldToViewMatrix() const override;

protected:
    SpatialCameraCoordinateTransformerImpl(const SpatialCameraCoordinateTransformerImpl& rhs) = default;
    SpatialCameraCoordinateTransformerImpl(SpatialCameraCoordinateTransformerImpl&& rhs) = delete;
    SpatialCameraCoordinateTransformerImpl& operator=(const SpatialCameraCoordinateTransformerImpl& that) = delete;
    SpatialCameraCoordinateTransformerImpl& operator=(SpatialCameraCoordinateTransformerImpl&& that) = delete;

    virtual glm::mat4 getModelMatrix() const;
    virtual glm::mat4 getWorldMatrix() const;
    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix() const;

private:
    const SpatialEntity* entity_;
    const Camera* camera_;
};

template <unsigned int N>
class StructuredCameraCoordinateTransformerImpl : public StructuredCameraCoordinateTransformer {
public:
    StructuredCameraCoordinateTransformerImpl(const StructuredGridEntity<N>& entity, const Camera& camera);
    virtual StructuredCameraCoordinateTransformerImpl* clone() const override;
    virtual ~StructuredCameraCoordinateTransformerImpl() = default;

    void setEntity(const StructuredGridEntity<N>& entity);
    void setCamera(const Camera& camera);

    virtual glm::mat4 getClipToDataMatrix() const override;
    virtual glm::mat4 getClipToIndexMatrix() const override;
    virtual glm::mat4 getClipToModelMatrix() const override;
    virtual glm::mat4 getClipToTextureMatrix() const override;
    virtual glm::mat4 getClipToViewMatrix() const override;
    virtual glm::mat4 getClipToWorldMatrix() const override;
    virtual glm::mat4 getDataToClipMatrix() const override;
    virtual glm::mat4 getDataToIndexMatrix() const override;
    virtual glm::mat4 getDataToModelMatrix() const override;
    virtual glm::mat4 getDataToViewMatrix() const override;
    virtual glm::mat4 getDataToWorldMatrix() const override;
    virtual glm::mat4 getIndexToClipMatrix() const override;
    virtual glm::mat4 getIndexToDataMatrix() const override;
    virtual glm::mat4 getIndexToModelMatrix() const override;
    virtual glm::mat4 getIndexToTextureMatrix() const override;
    virtual glm::mat4 getIndexToViewMatrix() const override;
    virtual glm::mat4 getIndexToWorldMatrix() const override;
    virtual glm::mat4 getModelToClipMatrix() const override;
    virtual glm::mat4 getModelToDataMatrix() const override;
    virtual glm::mat4 getModelToIndexMatrix() const override;
    virtual glm::mat4 getModelToTextureMatrix() const override;
    virtual glm::mat4 getModelToViewMatrix() const override;
    virtual glm::mat4 getModelToWorldMatrix() const override;
    virtual glm::mat4 getTextureToClipMatrix() const override;
    virtual glm::mat4 getTextureToIndexMatrix() const override;
    virtual glm::mat4 getTextureToModelMatrix() const override;
    virtual glm::mat4 getTextureToViewMatrix() const override;
    virtual glm::mat4 getTextureToWorldMatrix() const override;
    virtual glm::mat4 getViewToClipMatrix() const override;
    virtual glm::mat4 getViewToDataMatrix() const override;
    virtual glm::mat4 getViewToIndexMatrix() const override;
    virtual glm::mat4 getViewToModelMatrix() const override;
    virtual glm::mat4 getViewToTextureMatrix() const override;
    virtual glm::mat4 getViewToWorldMatrix() const override;
    virtual glm::mat4 getWorldToClipMatrix() const override;
    virtual glm::mat4 getWorldToDataMatrix() const override;
    virtual glm::mat4 getWorldToIndexMatrix() const override;
    virtual glm::mat4 getWorldToModelMatrix() const override;
    virtual glm::mat4 getWorldToTextureMatrix() const override;
    virtual glm::mat4 getWorldToViewMatrix() const override;

protected:
    StructuredCameraCoordinateTransformerImpl(const StructuredCameraCoordinateTransformerImpl& rhs) = default;
    StructuredCameraCoordinateTransformerImpl(StructuredCameraCoordinateTransformerImpl&& rhs) = delete;
    StructuredCameraCoordinateTransformerImpl& operator=(const StructuredCameraCoordinateTransformerImpl& that) = delete;
    StructuredCameraCoordinateTransformerImpl& operator=(StructuredCameraCoordinateTransformerImpl&& that) = delete;
    
    virtual glm::mat4 getIndexMatrix() const;
    virtual glm::mat4 getModelMatrix() const;
    virtual glm::mat4 getWorldMatrix() const;
    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix() const;

private:
    const StructuredGridEntity<N>* entity_;
    const Camera* camera_;
};

extern template class IVW_CORE_TMPL_EXP StructuredCoordinateTransformerImpl<2>;
extern template class IVW_CORE_TMPL_EXP StructuredCoordinateTransformerImpl<3>;
extern template class IVW_CORE_TMPL_EXP StructuredCameraCoordinateTransformerImpl<2>;
extern template class IVW_CORE_TMPL_EXP StructuredCameraCoordinateTransformerImpl<3>;

/*
 *  Implementations StructuredCoordinateTransformerImpl
 */
template<unsigned int N>
StructuredCoordinateTransformerImpl<N>::StructuredCoordinateTransformerImpl(const StructuredGridEntity<N>& entity)
    : StructuredCoordinateTransformer()
    , entity_{&entity} {}

template<unsigned int N>
StructuredCoordinateTransformerImpl<N>* StructuredCoordinateTransformerImpl<N>::clone() const {
    return new StructuredCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getIndexMatrix() const {
    return entity_->getIndexMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

template <unsigned int N>
void StructuredCoordinateTransformerImpl<N>::setEntity(const StructuredGridEntity<N>& entity) {
    entity_ = &entity;
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getDataToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getIndexToDataMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getIndexToModelMatrix() const {
    return getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getIndexToTextureMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getIndexToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getModelToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getModelToTextureMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getTextureToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getTextureToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getTextureToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getWorldToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCoordinateTransformerImpl<N>::getWorldToTextureMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

/*
 *  Implementations StructuredCameraCoordinateTransformerImpl
 */
template<unsigned int N>
StructuredCameraCoordinateTransformerImpl<N>::StructuredCameraCoordinateTransformerImpl(const StructuredGridEntity<N>& entity, const Camera& camera)
    : StructuredCameraCoordinateTransformer()
    , entity_{&entity}
    , camera_{&camera} {}

template<unsigned int N>
StructuredCameraCoordinateTransformerImpl<N>* StructuredCameraCoordinateTransformerImpl<N>::clone() const {
    return new StructuredCameraCoordinateTransformerImpl<N>(*this);
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexMatrix() const {
    return entity_->getIndexMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelMatrix() const {
    return entity_->getModelMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewMatrix() const {
    return camera_->getViewMatrix();
}
template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getProjectionMatrix() const {
    return camera_->getProjectionMatrix();
}

template <unsigned int N>
void StructuredCameraCoordinateTransformerImpl<N>::setEntity(const StructuredGridEntity<N>& entity) {
    entity_ = &entity;
}
template <unsigned int N>
void StructuredCameraCoordinateTransformerImpl<N>::setCamera(const Camera& camera) {
    camera_ = &camera;
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToDataMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToModelMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToTextureMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToViewMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getClipToWorldMatrix() const {
    return glm::inverse(getProjectionMatrix()*getViewMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getDataToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToDataMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToModelMatrix() const {
    return getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToTextureMatrix() const {
    return glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getIndexToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix()*glm::inverse(getIndexMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToTextureMatrix() const {
    return glm::inverse(getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToIndexMatrix() const {
    return getIndexMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToModelMatrix() const {
    return getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToViewMatrix() const {
    return getViewMatrix()*getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getTextureToWorldMatrix() const {
    return getWorldMatrix()*getModelMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToClipMatrix() const {
    return getProjectionMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToDataMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToModelMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToTextureMatrix() const {
    return glm::inverse(getViewMatrix()*getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getViewToWorldMatrix() const {
    return glm::inverse(getViewMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToClipMatrix() const {
    return getProjectionMatrix()*getViewMatrix();
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToIndexMatrix() const {
    return getIndexMatrix()*glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToTextureMatrix() const {
    return glm::inverse(getWorldMatrix()*getModelMatrix());
}

template <unsigned int N>
glm::mat4 StructuredCameraCoordinateTransformerImpl<N>::getWorldToViewMatrix() const {
    return getViewMatrix();
}

// clang-format on
}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::CoordinateSpace> : inviwo::FlagFormatter<inviwo::CoordinateSpace> {};
#endif
