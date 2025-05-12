/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/plottinggl/processors/orthographicaxis2d.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

const ProcessorInfo OrthographicAxis2D::processorInfo_{
    "org.inviwo.OrthographicAxis2D",  // Class identifier
    "Orthographic Axis2D",            // Display name
    "Plotting",                       // Category
    CodeState::Stable,                // Code state
    Tags::GL | Tag{"Plotting"},       // Tags
    R"(This processor draws a pair of axes that are mapped to world space.
       The use case is for plotting data using a Orthographic Camera.
       )"_unindentHelp,
};

const ProcessorInfo& OrthographicAxis2D::getProcessorInfo() const { return processorInfo_; }

OrthographicAxis2D::OrthographicAxis2D()
    : Processor{}
    , inport_{"inport"}
    , mesh_{"mesh"}
    , outport_{"outport"}
    , style_{"style", "Global Style"}
    , backgroundColor_{"backgroundColor", "Background Color",
                       util::ordinalColor(1.f, 1.f, 1.f, 1.f)}
    , clipContent_{"clipContent", "Clip Content", true}
    , axis1_{"axis1", "Axis 1"}
    , axis2_{"axis2", "Axis 2", plot::AxisProperty::Orientation::Vertical}
    , margins_{"margins", "Margins", 5.0f, 5.0f, 55.0f, 65.0f}
    , axisMargin_{"axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f}
    , antialiasing_{"antialias", "Antialiasing", true}
    , camera_{"camera", "Camera"}
    , trackball_{&camera_}
    , axisRenderers_{axis1_, axis2_} {

    style_.setCollapsed(true);
    style_.registerProperties(axis1_, axis2_);
    style_.insertProperty(3, backgroundColor_);
    style_.addProperties(antialiasing_, clipContent_);

    addPorts(inport_, mesh_, outport_);
    addProperties(style_, axis1_, axis2_, margins_, axisMargin_, camera_, trackball_);

    camera_.setCamera(std::make_unique<OrthographicCamera>());
    camera_.setCurrentStateAsDefault();

    trackball_.allowHorizontalRotation_.set(false);
    trackball_.allowVerticalRotation_.set(false);
    trackball_.allowViewDirectionRotation_.set(false);

    trackball_.mouseCenteredZoom_.set(true);

    trackball_.mouseRotate_.setEventMatcher(std::make_unique<DisableEventMatcher>());
    trackball_.stepRotateUp_.setEventMatcher(std::make_unique<DisableEventMatcher>());
    trackball_.stepRotateDown_.setEventMatcher(std::make_unique<DisableEventMatcher>());
    trackball_.stepRotateLeft_.setEventMatcher(std::make_unique<DisableEventMatcher>());
    trackball_.stepRotateRight_.setEventMatcher(std::make_unique<DisableEventMatcher>());

    trackball_.mousePan_.setEventMatcher(std::make_unique<MouseEventMatcher>(
        MouseButton::Left, MouseState::Move, KeyModifiers(flags::none)));

    trackball_.setCurrentStateAsDefault();
}

void OrthographicAxis2D::process() {
    const auto dims = outport_.getDimensions();

    const auto padding = axisMargin_.get();

    const utilgl::ClearColor clearColor{backgroundColor_.get()};
    utilgl::activateAndClearTarget(outport_);
    if (auto source = inport_.getData()) {
        const auto scissors =
            [&]() -> std::optional<std::pair<utilgl::ScissorState, utilgl::GlBoolState>> {
            if (clipContent_) {
                return std::pair{
                    utilgl::ScissorState{static_cast<GLint>(margins_.getLeft() + padding),
                                         static_cast<GLint>(margins_.getBottom() + padding),
                                         static_cast<GLsizei>(dims.x - margins_.getLeft() -
                                                              margins_.getRight() - 2 * padding),
                                         static_cast<GLsizei>(dims.y - margins_.getBottom() -
                                                              margins_.getTop() - 2 * padding)},
                    utilgl::GlBoolState{GL_SCISSOR_TEST, true}};
            }
            return std::nullopt;
        }();
        source->getRepresentation<ImageGL>()->copyRepresentationsTo(
            outport_.getEditableData()->getEditableRepresentation<ImageGL>());
    }

    const size2_t lowerLeft(margins_.getLeft(), margins_.getBottom());
    const size2_t upperRight(dims.x - 1 - margins_.getRight(), dims.y - 1 - margins_.getTop());

    const auto xStart = lowerLeft + size2_t(padding, 0);
    const auto xEnd = size2_t(upperRight.x - padding, lowerLeft.y);

    const auto yStart = lowerLeft + size2_t(0, padding);
    const auto yEnd = size2_t(lowerLeft.x, upperRight.y - padding);

    const auto start =
        2.0f * vec3{dvec2{xStart.x, yStart.y} / dvec2{dims}, 0.5} - vec3{1.0, 1.0, 1.0};
    const auto end = 2.0f * vec3{dvec2{xEnd.x, yEnd.y} / dvec2{dims}, 0.5} - vec3{1.0, 1.0, 1.0};

    const auto data = mesh_.getData();

    auto w2m = data->getCoordinateTransformer().getWorldToModelMatrix();
    const auto wStart =
        w2m * vec4{camera_.get().getWorldPosFromNormalizedDeviceCoords(start), 1.0f};
    const auto wEnd = w2m * vec4{camera_.get().getWorldPosFromNormalizedDeviceCoords(end), 1.0f};

    axis1_.setRange(dvec2{wStart.x, wEnd.x});
    axis2_.setRange(dvec2{wStart.y, wEnd.y});

    axis1_.majorTicks_.tickDelta_.set(std::abs((wEnd.x - wStart.x) / 10.0));
    axis2_.majorTicks_.tickDelta_.set(std::abs((wEnd.y - wStart.y) / 10.0));

    axis1_.captionSettings_.title_.set(
        fmt::format("{}{: [}", data->getAxis(0)->name, data->getAxis(0)->unit));
    axis2_.captionSettings_.title_.set(
        fmt::format("{}{: [}", data->getAxis(1)->name, data->getAxis(1)->unit));

    const utilgl::DepthFuncState depthFunc(GL_ALWAYS);

    // draw horizontally
    axisRenderers_[0].render(dims, xStart, xEnd, antialiasing_.get());

    // draw vertically
    axisRenderers_[1].render(dims, yStart, yEnd, antialiasing_.get());
}

}  // namespace inviwo
