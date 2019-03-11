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

#include <modules/basegl/algorithm/entryexitpoints.h>

#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

namespace inviwo {

namespace algorithm {

EntryExitPointsHelper::EntryExitPointsHelper()
    : entryExitShader_("standard.vert", "standard.frag")
    , meshEntryExitShader_("meshentryexit.vert", "standard.frag")
    , nearClipShader_("img_identity.vert", "capnearclipping.frag") {}

void EntryExitPointsHelper::operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                                       const Mesh& mesh, bool capNearClip) {
    if (capNearClip) {
        createCappedEntryExitPoints(entryPoints, exitPoints, camera, mesh);
    } else {
        createEntryExitPoints(entryPoints, exitPoints, camera, mesh);
    }
}

void EntryExitPointsHelper::operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                                       const Volume& volume, const Mesh& mesh, bool capNearClip) {
    const mat4 meshDataToVolumeData =
        volume.getCoordinateTransformer(camera).getWorldToDataMatrix() *
        mesh.getCoordinateTransformer().getDataToWorldMatrix();

    if (capNearClip) {
        createCappedEntryExitPoints(entryPoints, exitPoints, camera, mesh, true,
                                    meshDataToVolumeData);
    } else {
        createEntryExitPoints(entryPoints, exitPoints, camera, mesh, true, meshDataToVolumeData);
    }
}

std::vector<std::reference_wrapper<Shader>> EntryExitPointsHelper::getShaders() {
    return {entryExitShader_, meshEntryExitShader_, nearClipShader_};
}

void EntryExitPointsHelper::createEntryExitPoints(Image& entryPoints, Image& exitPoints,
                                                  const Camera& camera, const Mesh& mesh,
                                                  bool applyTrafo,
                                                  const mat4& meshDataToVolumeData) {
    utilgl::PointSizeState pointsize(1.0f);

    Shader& shader = applyTrafo ? meshEntryExitShader_ : entryExitShader_;

    shader.activate();
    const mat4 dataToClipMatrix = mesh.getCoordinateTransformer(camera).getDataToClipMatrix();
    shader.setUniform("dataToClip", dataToClipMatrix);
    shader.setUniform("meshDataToVolData", meshDataToVolumeData);

    auto drawer = MeshDrawerGL::getDrawObject(&mesh);

    {
        // generate exit points
        utilgl::DepthFuncState depthfunc(GL_GREATER);
        utilgl::ClearDepth clearDepth(0.0f);
        utilgl::activateAndClearTarget(exitPoints, ImageType::ColorDepth);
        utilgl::CullFaceState cull(GL_FRONT);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }

    {
        // generate entry points
        utilgl::DepthFuncState depthfunc(GL_LESS);
        utilgl::activateAndClearTarget(entryPoints, ImageType::ColorDepth);

        utilgl::CullFaceState cull(GL_BACK);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }
    shader.deactivate();
}

void EntryExitPointsHelper::createCappedEntryExitPoints(Image& entryPoints, Image& exitPoints,
                                                        const Camera& camera, const Mesh& mesh,
                                                        bool applyTrafo,
                                                        const mat4& meshDataToVolumeData) {
    utilgl::PointSizeState pointsize(1.0f);

    Shader& shader = applyTrafo ? meshEntryExitShader_ : entryExitShader_;

    shader.activate();
    const mat4 dataToClipMatrix = mesh.getCoordinateTransformer(camera).getDataToClipMatrix();
    shader.setUniform("dataToClip", dataToClipMatrix);
    shader.setUniform("meshDataToVolData", meshDataToVolumeData);

    auto drawer = MeshDrawerGL::getDrawObject(&mesh);

    {
        // generate exit points
        utilgl::DepthFuncState depthfunc(GL_GREATER);
        utilgl::ClearDepth clearDepth(0.0f);
        utilgl::activateAndClearTarget(exitPoints, ImageType::ColorDepth);
        utilgl::CullFaceState cull(GL_FRONT);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }

    {
        // generate entry points
        utilgl::DepthFuncState depthfunc(GL_LESS);
        if (!tmpEntry_ || tmpEntry_->getDimensions() != entryPoints.getDimensions() ||
            tmpEntry_->getDataFormat() != entryPoints.getDataFormat()) {
            tmpEntry_.reset(new Image(entryPoints.getDimensions(), entryPoints.getDataFormat()));
        }
        utilgl::activateAndClearTarget(*tmpEntry_);

        utilgl::CullFaceState cull(GL_BACK);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }

    // render an image plane aligned quad to cap the proxy geometry
    utilgl::activateAndClearTarget(entryPoints, ImageType::ColorDepth);
    nearClipShader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(nearClipShader_, units, *tmpEntry_, "entry", ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(nearClipShader_, units, exitPoints, "exit", ImageType::ColorDepth);

    // the rendered plane is specified in camera coordinates
    // thus we must transform from camera to world to texture coordinates
    mat4 clipToTexMat = mesh.getCoordinateTransformer(camera).getClipToDataMatrix();
    nearClipShader_.setUniform("NDCToTextureMat", clipToTexMat);
    nearClipShader_.setUniform("nearDist", camera.getNearPlaneDist());

    utilgl::singleDrawImagePlaneRect();
    nearClipShader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace algorithm

}  // namespace inviwo
