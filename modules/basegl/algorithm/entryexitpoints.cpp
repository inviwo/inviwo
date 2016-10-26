/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>


namespace inviwo {

namespace algorithm {

EntryExitPointsHelper::EntryExitPointsHelper(ImageOutport *entryPoints, ImageOutport *exitPoints)
    : entryExitShader_("standard.vert", "standard.frag")
    , nearClipShader_("img_identity.vert", "capnearclipping.frag")
    , entryPoints_(entryPoints)
    , exitPoints_(exitPoints) {
}

void EntryExitPointsHelper::operator()(Camera *camera, const std::shared_ptr<const Mesh> &mesh,
                                       bool capNearClip) {
    if (!entryPoints_ || !exitPoints_) {
        return;
    }

    if (capNearClip) {
        createCappedEntryExitPoints(camera, mesh);
    } else {
        createEntryExitPoints(camera, mesh);
    }
}

void EntryExitPointsHelper::createEntryExitPoints(Camera *camera,
                                                  const std::shared_ptr<const Mesh> &mesh) {
    utilgl::DepthFuncState depthfunc(GL_ALWAYS);
    utilgl::PointSizeState pointsize(1.0f);

    entryExitShader_.activate();
    mat4 modelMatrix = mesh->getCoordinateTransformer(*camera).getDataToClipMatrix();
    entryExitShader_.setUniform("dataToClip", modelMatrix);

    auto drawer = MeshDrawerGL::getDrawObject(mesh.get());

    {
        // generate exit points
        utilgl::activateAndClearTarget(*exitPoints_, ImageType::ColorDepth);
        utilgl::CullFaceState cull(GL_FRONT);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }

    {
        // generate entry points
        utilgl::activateAndClearTarget(*entryPoints_, ImageType::ColorDepth);

        utilgl::CullFaceState cull(GL_BACK);
        drawer.draw();
        entryExitShader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }
}

void EntryExitPointsHelper::createCappedEntryExitPoints(Camera *camera,
                                                        const std::shared_ptr<const Mesh> &mesh) {
    utilgl::DepthFuncState depthfunc(GL_ALWAYS);
    utilgl::PointSizeState pointsize(1.0f);

    entryExitShader_.activate();
    mat4 modelMatrix = mesh->getCoordinateTransformer(*camera).getDataToClipMatrix();
    entryExitShader_.setUniform("dataToClip", modelMatrix);

    auto drawer = MeshDrawerGL::getDrawObject(mesh.get());

    {
        // generate exit points
        utilgl::activateAndClearTarget(*exitPoints_, ImageType::ColorDepth);
        utilgl::CullFaceState cull(GL_FRONT);
        drawer.draw();
        utilgl::deactivateCurrentTarget();
    }

    {
        // generate entry points
        if (!tmpEntry_ || tmpEntry_->getDimensions() != entryPoints_->getDimensions() ||
            tmpEntry_->getDataFormat() != entryPoints_->getData()->getDataFormat()) {
            tmpEntry_.reset(
                new Image(entryPoints_->getDimensions(), entryPoints_->getData()->getDataFormat()));
        }
        utilgl::activateAndClearTarget(*tmpEntry_);

        utilgl::CullFaceState cull(GL_BACK);
        drawer.draw();
        entryExitShader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }

    // render an image plane aligned quad to cap the proxy geometry
    utilgl::activateAndClearTarget(*entryPoints_, ImageType::ColorDepth);
    nearClipShader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(nearClipShader_, units, *tmpEntry_, "entry", ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(nearClipShader_, units, *exitPoints_, ImageType::ColorDepth);

    // the rendered plane is specified in camera coordinates
    // thus we must transform from camera to world to texture coordinates
    mat4 clipToTexMat = mesh->getCoordinateTransformer(*camera).getClipToDataMatrix();
    nearClipShader_.setUniform("NDCToTextureMat", clipToTexMat);
    nearClipShader_.setUniform("nearDist", camera->getNearPlaneDist());

    utilgl::singleDrawImagePlaneRect();
    nearClipShader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace algorithm

} // namespace inviwo
