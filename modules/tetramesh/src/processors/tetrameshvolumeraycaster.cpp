/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <inviwo/tetramesh/processors/tetrameshvolumeraycaster.h>
#include <inviwo/tetramesh/datastructures/tetramesh.h>
#include <inviwo/tetramesh/util/tetrameshutils.h>

#include <inviwo/core/interaction/events/mousebuttons.h>        // for MouseButton, MouseButton::...
#include <inviwo/core/interaction/events/mouseevent.h>          // for MouseEvent
#include <inviwo/core/interaction/events/pickingevent.h>        // for PickingEvent
#include <inviwo/core/interaction/pickingmapper.h>              // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>               // for PickingState, PickingState...
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/sharedopenglresources.h>

namespace inviwo {

namespace detail {

std::function<std::optional<mat4>()> boundingBox(const TetraMeshInport& tetra) {
    return [port = &tetra]() -> std::optional<mat4> {
        if (port->hasData()) {
            auto data = port->getData();
            return data->getBoundingBox();
        } else {
            return std::nullopt;
        }
    };
}

}  // namespace detail

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TetraMeshVolumeRaycaster::processorInfo_{
    "org.inviwo.TetraMeshVolumeRaycaster",  // Class identifier
    "TetraMesh Volume Raycaster",           // Display name
    "Unstructured Grids",                   // Category
    CodeState::Stable,                      // Code state
    Tags::GL | Tag{"Unstructured"},         // Tags
    R"(
Creates an OpenGL representation of a tetrahedral grid and renders it using volume rendering. This
processor requires OpenGL 4.3 since it relies on Shader Storage Buffer Objects.)"_unindentHelp};

const ProcessorInfo TetraMeshVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

TetraMeshVolumeRaycaster::TetraMeshVolumeRaycaster()
    : Processor{}
    , inport_{"inport", "Tetra mesh data used for volume rendering"_help}
    , imageInport_{"background", "Optional background image"_help}
    , outport_{"outport", "Rendered output image"_help}
    , camera_{"camera", "Camera", detail::boundingBox(inport_)}
    , trackball_{&camera_}
    , lighting_{"lighting", "Lighting", &camera_}
    , tf_{"transferFunction", "Transfer Function"}
    , opacityScaling_{"opacityScaling", "Opacity Scaling",
                      util::ordinalScale(1.0f, 10.0f)
                          .setInc(0.01f)
                          .set("Scaling factor for the opacity in the transfer function."_help)}
    , maxSteps_{"maxSteps", "Max Steps",
                util::ordinalCount(10000).set(
                    "Upper limit of tetrahedras a ray can traverse."_help)}
    , pickingOutput_{"pickingout", "Picking Result", "", InvalidationLevel::Valid, PropertySemantics::Multiline }
    , shader_{"tetramesh_traversal.vert", "tetramesh_traversal.frag", Shader::Build::No}
    , picking_(this, 1, [&](PickingEvent* p) { handlePickingEvent(p); })
    , cutplane_{
          "cutplane",
          "Cutplane",
          "Cuts the tetramesh volume along the x, y, and z axis"_help,
          vec3(1.0f),
          {vec3(-1.0f), ConstraintBehavior::Ignore},
          {vec3(1.0f), ConstraintBehavior::Ignore}}
    , enableIsoSurface_{"enableIsoSurface", "Enable ISO Surface", false}
    , isoValue_{"isoValue", "ISO Value", 0.5f, 0.f, 1.f}
    , prevDataRange_{0.0, 1.0}
    , isoColor_{"isoColor", "ISO Color", vec4(1.0, 1.0, 0.6, 1.0)} {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    addPorts(inport_, imageInport_, outport_);

    imageInport_.setOptional(true);
    imageInport_.onConnect([&]() { invalidate(InvalidationLevel::InvalidResources); });
    imageInport_.onDisconnect([&]() { invalidate(InvalidationLevel::InvalidResources); });

    addProperties(tf_, opacityScaling_, maxSteps_, camera_, lighting_, trackball_);

    lighting_.shadingMode_.setSelectedValue(ShadingMode::None);
    lighting_.shadingMode_.setCurrentStateAsDefault();

    isoColor_.setSemantics(PropertySemantics::Color);

    if (OpenGLCapabilities::getOpenGLVersion() < 430 &&
        !OpenGLCapabilities::isExtensionSupported("ARB_shader_storage_buffer_object")) {
        LogError(
            "OpenGL v4.3 or Shader Storage Buffer Objects (ARB_shader_storage_buffer_object) "
            "required by this processor");

        isReady_.setUpdate([]() { return false; });
    }

    addProperties(enableIsoSurface_, isoValue_, isoColor_, cutplane_, pickingOutput_);
}

void TetraMeshVolumeRaycaster::handlePickingEvent(PickingEvent* p) {
    if ((p->getState() & PickingState::Updated) && (p->getEvent()->hash() == MouseEvent::chash())) {
        p->markAsUsed();
        auto me = p->getEventAs<MouseEvent>();
        if ((me->buttonState() & MouseButton::Left) &&
            me->state() & (MouseState::Press | MouseState::Move)) {
            // Get the normalized screen space coordinates (0,1)
            const dvec2 pos = p->getPosition();
            auto img = outport_.getData();
            const size2_t size = img->getDimensions();
            // Map screen space coordinate to integer texture coordinates
            size2_t cords = size2_t(std::lround(pos.x * size.x), std::lround(pos.y * size.y));
            const dvec4 v = img->readPixel(cords, LayerType::Color, 1);

            dvec3 radarOrigin = dvec3(-143.65316218500305, -1673.6645787018295, 6153.016403143832);
            dmat4 translation = glm::translate(dmat4(1.0), radarOrigin);
            dvec4 pointPos = dvec4(v.y, v.z, v.w, 1.0);
            dvec3 worldPos = dvec3(translation * pointPos);

            double r = std::sqrt(glm::dot(worldPos, worldPos));
            double theta = std::acos(worldPos.z / r);
            double phi = std::atan2(worldPos.y, worldPos.x);
            double earthRadius = 6371;
            r -= earthRadius;
            phi = glm::degrees(phi);
            phi = std::fmod(phi + 360.0, 360);
            theta = 90.0 - glm::degrees(theta);

            pickingOutput_.set(
                fmt::format(
                    "Value:      {:.2f}\n"
                    "Latitude:   {:.4f}\n"
                    "Longitude:  {:.4f}\n"
                    "Height:     {:.2f}\n"
                    "x: {:.f}\n"
                    "y: {:.f}\n"
                    "z: {:.f}",
                    v.x, theta, phi, r, v.y, v.z, v.w
            ));
        }
    }

}

void TetraMeshVolumeRaycaster::initializeResources() {
    utilgl::addDefines(shader_, camera_, lighting_);
    utilgl::addShaderDefinesBGPort(shader_, imageInport_);

    int layerID = 2;
    auto frag = shader_.getFragmentShaderObject();
    frag->addOutDeclaration("out_picking_data", layerID++);

    auto out = outport_.getData();
    if (out->getNumberOfColorLayers() != (layerID))
    {
        // Add extra color layer to output image
        auto image = std::make_shared<Image>(out->getDimensions(), out->getDataFormat());
        Layer layer = Layer(out->getDimensions(), DataVec4Float32::get(), LayerType::Color);
        image->addColorLayer(std::make_shared<Layer>(layer));
        outport_.setData(image);
    }

    shader_.build();
}

void TetraMeshVolumeRaycaster::process() {
    if (inport_.isChanged() || !mesh_) {
        const auto& tetraMesh = *inport_.getData();

        tetraMesh.get(tetraNodes_, tetraNodeIds_);
        auto opposingFaces = utiltetra::getOpposingFaces(tetraNodeIds_);

        buffers_.upload(tetraNodes_, tetraNodeIds_, opposingFaces);
        mesh_ = utiltetra::createBoundaryMesh(tetraMesh, tetraNodes_, tetraNodeIds_,
                                              utiltetra::getBoundaryFaces(opposingFaces));

        // Update the isoValue min/max range to the new data, keeps the slider at the same % as prev
        const dvec2 dataRange{inport_.getData()->getDataRange()};
        const float percent = (isoValue_ - prevDataRange_.x) / (prevDataRange_.y - prevDataRange_.x);
        const float newValue = percent * (dataRange.y - dataRange.x) + dataRange.x;

        isoValue_.set(newValue, dataRange.x, dataRange.y, Defaultvalues<float>::getInc());
        prevDataRange_ = dataRange;
    }

    {
        // pre-multiply the background image while copying it to the current target since the
        // raycasting also blends pre-multiplied background colors.
        utilgl::BlendModeState blendmode{GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ZERO};
        utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_,
                                                   ImageType::ColorDepthPicking);
    }

    if (imageInport_.hasData()) {
        // clear depth buffer since copy-source always copies all layers otherwise the background
        // depth might interfere with the raycasting
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    shader_.activate();
    buffers_.bind();

    TextureUnitContainer texContainer;
    utilgl::setUniforms(shader_, camera_, lighting_, opacityScaling_, maxSteps_);
    utilgl::setShaderUniforms(shader_, *mesh_, "geometry");
    utilgl::setShaderUniforms(shader_, cutplane_, "cutplane");
    utilgl::setShaderUniforms(shader_, enableIsoSurface_, "enableIsoSurface");
    utilgl::setShaderUniforms(shader_, isoValue_, "isoValue");
    utilgl::setShaderUniforms(shader_, isoColor_, "isoColor");
    shader_.setUniform("pickingID", static_cast<int>(picking_.getPickingId(0)));
    utilgl::bindAndSetUniforms(shader_, texContainer, tf_);
    if (imageInport_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, texContainer, imageInport_, ImageType::ColorDepthPicking);
    }

    const dvec2 dataRange{inport_.getData()->getDataRange()};
    const double scalingFactor = 1.0 / (dataRange.y - dataRange.x);
    const double offset = -dataRange.x;
    shader_.setUniform("tfValueScaling", static_cast<float>(scalingFactor));
    shader_.setUniform("tfValueOffset", static_cast<float>(offset));

    {
        utilgl::CullFaceState cf(GL_BACK);
        utilgl::GlBoolState cull(GL_CULL_FACE, true);
        utilgl::GlBoolState blend(GL_BLEND, false);

        auto drawer = MeshDrawerGL::getDrawObject(mesh_.get());
        drawer.draw();
    }

    buffers_.unbind();
    shader_.deactivate();

    if (imageInport_.hasData()) {
        // update depth buffer with the depth of the background image
        utilgl::DepthMaskState depthmask{true};
        utilgl::GlBoolState depthtest{GL_DEPTH_TEST, true};
        utilgl::DepthFuncState depthfunc{GL_LESS};
        utilgl::ColorMaskState colormask{bvec4{false}};
        TextureUnit depthUnit;
        auto depthLayer = imageInport_.getData()->getDepthLayer()->getRepresentation<LayerGL>();

        auto copyShader = SharedOpenGLResources::getPtr()->getImageCopyShader(1);
        copyShader->activate();
        depthLayer->bindTexture(depthUnit);
        copyShader->setUniform("depth_", depthUnit);

        auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
        utilgl::Enable<MeshGL> enable(rect);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        copyShader->deactivate();
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
