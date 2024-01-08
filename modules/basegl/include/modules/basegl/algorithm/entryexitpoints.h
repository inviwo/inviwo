/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/datastructures/image/image.h>  // for Image
#include <inviwo/core/util/dispatcher.h>             // for Dispatcher
#include <inviwo/core/util/glmmat.h>                 // for mat4

#include <functional>  // for function
#include <memory>      // for shared_ptr, unique_ptr

namespace inviwo {

class Camera;
class ImageGL;
class Mesh;
class Shader;
class Volume;

namespace algorithm {

enum class IncludeNormals { Yes, No };
enum class CapNearClip { Yes, No };

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
     * @param includeNormals   add an additional color layer to the entry points with mesh normals
     */
    void operator()(Image& entryPoints, Image& exitPoints, const Camera& camera, const Mesh& mesh,
                    CapNearClip capNearClip, IncludeNormals includeNormals);

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
     * @param includeNormals   add an additional color layer to the entry points with mesh normals
     */
    void operator()(ImageGL& entryPoints, ImageGL& exitPoints, const Camera& camera,
                    const Mesh& mesh, CapNearClip capNearClip, IncludeNormals includeNormals);

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
     * @param includeNormals   add an additional color layer to the entry points with mesh normals
     */
    void operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                    const Volume& volume, const Mesh& mesh, CapNearClip capNearClip,
                    IncludeNormals includeNormals);

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
     * @param includeNormals   add an additional color layer to the entry points with mesh normals
     */
    void operator()(ImageGL& entryPoints, ImageGL& exitPoints, const Camera& camera,
                    const Volume& volume, const Mesh& mesh, CapNearClip capNearClip,
                    IncludeNormals includeNormals);

    std::shared_ptr<std::function<void()>> onReload(std::function<void()> callback);

private:
    enum class ApplyTransformation { Yes, No };
    void createEntryExitPoints(ImageGL& entryPoints, ImageGL& exitPoints, const Camera& camera,
                               const Mesh& mesh, IncludeNormals includeNormals,
                               ApplyTransformation applyTrafo,
                               const mat4& meshDataToVolumeData = mat4(1.0f));
    void createCappedEntryExitPoints(ImageGL& entryPoints, ImageGL& exitPoints,
                                     const Camera& camera, const Mesh& mesh,
                                     IncludeNormals includeNormals, ApplyTransformation applyTrafo,
                                     const mat4& meshDataToVolumeData = mat4(1.0f));

    Shader& getEntryExitShader(IncludeNormals includeNormals, ApplyTransformation applyTrafo);
    Shader& getNearClipShader(IncludeNormals includeNormals);

    static std::shared_ptr<Shader> createEntryExitShader(IncludeNormals includeNormals,
                                                         ApplyTransformation applyTrafo);
    static std::shared_ptr<Shader> createNearClipShader(IncludeNormals includeNormals);

    std::unique_ptr<Image> tmpEntry_;
    ImageGL* tmpEntryGL_ = nullptr;

    // Shaders are shared among all instances, hence one has to set all uniforms at every use.
    struct EntryExitShader {
        IncludeNormals includeNormals = IncludeNormals::No;
        ApplyTransformation applyTrafo = ApplyTransformation::No;
        std::shared_ptr<Shader> shader{nullptr};
        std::shared_ptr<std::function<void()>> reloadCallback;
    };
    EntryExitShader ees_;

    struct NearCapShader {
        IncludeNormals includeNormals = IncludeNormals::No;
        std::shared_ptr<Shader> shader{nullptr};
        std::shared_ptr<std::function<void()>> reloadCallback;
    };
    NearCapShader ncs_;

    Dispatcher<void()> onReloadCallback_;
};

}  // namespace algorithm

}  // namespace inviwo
