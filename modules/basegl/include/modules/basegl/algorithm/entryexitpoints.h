/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_ENTRYEXITPOINTS_H
#define IVW_ENTRYEXITPOINTS_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/opengl/shader/shader.h>

#include <functional>

namespace inviwo {

class Camera;
class Image;
class Mesh;
class Volume;

namespace algorithm {

/**
 * \class EntryExitPointsHelper
 * \brief Helper class for creating entry and exit points for a mesh
 */
class IVW_MODULE_BASEGL_API EntryExitPointsHelper {
public:
    EntryExitPointsHelper();
    ~EntryExitPointsHelper() = default;

    /**
     * \brief computes entry and exit points for raycasting using the given \p camera and bounding
     * geometry \p mesh. The color components of \p mesh are interpreted as positions in Data
     * space.
     *
     * @param entryPoints  entry points for raycasting the bounding geometry
     * @param exitPoints   exit points for raycasting the bounding geometry
     * @param camera       camera
     * @param mesh         mesh containing the bounding geometry, color must correspond to
     *                     Data space
     * @param capNearClip  if true and the near clip plane of the camera is inside the volume, an
     *                     additional plane will be used to close the volume in front of the camera
     */
    void operator()(Image& entryPoints, Image& exitPoints, const Camera& camera, const Mesh& mesh,
                    bool capNearClip);

    /**
     * \brief computes entry and exit points for raycasting using the given \p camera and bounding
     * geometry \p mesh. Positions of \p mesh are mapped to Data space (texture coordinates) of the
     * volume by using the Model to Data space transformation of \p volume.
     *
     * @param entryPoints  entry points for raycasting the bounding geometry
     * @param exitPoints   exit points for raycasting the bounding geometry
     * @param camera       camera
     * @param volume       used to determine the transformation of mesh positions to Data space
     * @param mesh         mesh containing the bounding geometry
     * @param capNearClip  if true and the near clip plane of the camera is inside the volume, an
     *                     additional plane will be used to close the volume in front of the camera
     */
    void operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                    const Volume& volume, const Mesh& mesh, bool capNearClip);

    std::vector<std::reference_wrapper<Shader>> getShaders();

private:
    void createEntryExitPoints(Image& entryPoints, Image& exitPoints, const Camera& camera,
                               const Mesh& mesh, bool applyTrafo = false,
                               const mat4& meshDataToVolumeData = mat4(1.0f));
    void createCappedEntryExitPoints(Image& entryPoints, Image& exitPoints, const Camera& camera,
                                     const Mesh& mesh, bool applyTrafo = false,
                                     const mat4& meshDataToVolumeData = mat4(1.0f));

    Shader entryExitShader_;
    Shader meshEntryExitShader_;
    Shader nearClipShader_;

    std::unique_ptr<Image> tmpEntry_;
};

}  // namespace algorithm

}  // namespace inviwo

#endif  // IVW_ENTRYEXITPOINTS_H
