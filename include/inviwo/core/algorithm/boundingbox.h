/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>

#include <functional>
#include <optional>

namespace inviwo {

class Inport;

class Mesh;
class Volume;

namespace util {

/**
 * Calculate a bounding box of the position buffer of the mesh in world space. The bounding box is
 * represented using a mat4, where all positions is between `bbox * (x,y,z,1) where x, y, and z are
 * between 0 and 1.
 */
IVW_CORE_API mat4 boundingBox(const Mesh &mesh);

/**
 * Calculate a bounding box of the position buffers of all the meshs in world space. The bounding
 * box is represented using a mat4, where all positions is between `bbox * (x,y,z,1) where x, y, and
 * z are between 0 and 1.
 */
IVW_CORE_API mat4 boundingBox(const std::vector<std::shared_ptr<const Mesh>> &meshes);

/**
 * Calculate a bounding box of the volume in world space. The bounding box is
 * represented using a mat4, where all positions is between `bbox * (x,y,z,1) where x, y, and z are
 * between 0 and 1.
 */
IVW_CORE_API mat4 boundingBox(const Volume &volume);

/**
 * Calculate a bounding box of all the volumes in world space. The bounding box is
 * represented using a mat4, where all positions is between `bbox * (x,y,z,1) where x, y, and z are
 * between 0 and 1.
 */
IVW_CORE_API mat4 boundingBox(const std::vector<std::shared_ptr<Volume>> &volumes);

/**
 * Construct function the returns the bounding box of the data in the port. I the port is empty the
 * function should return std::nullopt;
 */
/**@{*/
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(const DataInport<Mesh> &mesh);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(const DataInport<Mesh, 0> &meshes);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(
    const DataInport<Mesh, 0, true> &meshes);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(const DataOutport<Mesh> &mesh);

IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(const DataInport<Volume> &volume);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(
    const DataInport<std::vector<std::shared_ptr<Volume>>> &volumes);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(const DataOutport<Volume> &volume);
IVW_CORE_API std::function<std::optional<mat4>()> boundingBox(
    const DataOutport<std::vector<std::shared_ptr<Volume>>> &volumes);
/**@}*/

}  // namespace util

}  // namespace inviwo
