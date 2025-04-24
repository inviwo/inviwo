/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>

#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/timer.h>
#include <modules/opengl/shader/shader.h>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace inviwo {

class Image;
class ImageGL;
class Mesh;
class MeshDrawerGL;
class PickingEvent;

/**
 * \class CameraWidget
 * \brief provides a mesh-based interaction widget for the camera rotation
 */
class IVW_MODULE_USERINTERFACEGL_API CameraWidget : public Processor {
public:
    enum class Interaction : std::uint8_t { Yaw, Pitch, Roll, FreeRotation, Zoom, None };
    enum class RotationAxis : std::uint8_t { Yaw, Pitch, Roll };

    CameraWidget();
    ~CameraWidget();

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void invokeEvent(Event* event) override;

private:
    // initial state of camera when an interaction is triggered to keep the rotation axis consistent
    struct CameraState {
        vec3 dir = vec3(0.0f, 0.0f, -1.0f);
        vec3 up = vec3(0.0f, 1.0f, 0.0f);
        vec3 right = vec3(1.0f, 0.0f, 0.0f);
        double zoom = 1.0f;
    };

    void updateWidgetTexture(const ivec2& widgetSize);
    void drawWidgetTexture();

    void objectPicked(PickingEvent* p);
    static CameraState cameraState(const Camera& cam);
    void loadMesh();

    void dragInteraction(Interaction dir, dvec2 mouseDelta);
    void stepInteraction(Interaction dir, bool clockwise);

    void freeRotation(dvec2 mouseDelta);
    void axisRotation(RotationAxis dir, dvec2 mouseDelta);
    void rotation(vec3 axis, float distance);
    void stepRotation(RotationAxis dir, bool clockwise);
    void dragZoom(dvec2 delta);
    void stepZoom(bool zoomIn);

    void updateOutput(const mat4& rotation);

    std::vector<ButtonGroupProperty::Button> buttons();

    static vec3 rotationAxis(RotationAxis rot, bool alignToObject, const CameraState& cam);

    void startStopAnimation(bool start);
    void animate();

    ImageInport inport_;
    ImageOutport outport_;

    ButtonGroupProperty cameraActions_;
    BoolProperty visible_;
    CompositeProperty settings_;
    BoolProperty enabled_;
    BoolProperty invertDirections_;
    BoolProperty useObjectRotAxis_;
    BoolProperty showRollWidget_;
    BoolProperty showDollyWidget_;
    FloatProperty speed_;
    FloatProperty angleIncrement_;
    IntProperty minTouchMovement_;

    CompositeProperty appearance_;
    FloatProperty scaling_;
    FloatVec2Property position_;
    FloatVec2Property anchorPos_;
    BoolProperty showCube_;
    BoolCompositeProperty customColorComposite_;
    BoolProperty axisColoring_;
    FloatVec4Property userColor_;
    FloatVec4Property cubeColor_;

    CompositeProperty outputProps_;
    CameraProperty camera_;
    FloatMat4Property rotMatrix_;

    CompositeProperty internalProps_;
    PerspectiveCamera internalCamera_;
    SimpleLightingProperty lightingProperty_;

    PickingMapper picking_;
    Shader shader_;
    Shader cubeShader_;
    Shader overlayShader_;

    // meshes of the interaction widgets.
    // 1) camera widget
    // 2) camera widget including widget for "camera roll"
    // 3) camera zoom buttons
    // 4) cube
    std::array<std::unique_ptr<MeshDrawerGL>, 4> meshDrawers_;
    std::array<std::shared_ptr<const Mesh>, 4> meshes_;

    struct Widget {
        Interaction dir;
        bool clockwise;
    };

    // Each interaction widget has two trigger areas to
    // distinguish the direction of rotation when clicked.
    static constexpr std::array widgets_ = {
        Widget{Interaction::Yaw, true},          Widget{Interaction::Yaw, false},
        Widget{Interaction::Pitch, true},        Widget{Interaction::Pitch, false},
        Widget{Interaction::FreeRotation, true}, Widget{Interaction::FreeRotation, false},
        Widget{Interaction::Roll, true},         Widget{Interaction::Roll, false},
        Widget{Interaction::Zoom, true},         Widget{Interaction::Zoom, false}};

    // UI state and textures
    struct Picking {
        void objectPicked(PickingEvent* e, CameraWidget& camera);

    private:
        bool isMouseBeingPressedAndHold{false};
        bool mouseWasMoved{false};
        int currentPickingID{-1};
    };
    Picking pickingState_;

    CameraState initialState_;

    // Ensure that the Image and ImageGL are always in sync.
    // By returning a pair we ensure we can never return an Image and a nullptr ImageGL,
    // which can happen if we run into any OpenGL error.
    std::tuple<std::unique_ptr<Image>, ImageGL*> createWidgetImage(const ivec2& widgetSize);

    std::unique_ptr<Image> widgetImage_;  //!< the widget is rendered into this image, which is then
                                          //!< drawn on top of the input image
    ImageGL* widgetImageGL_;  //!< keep an ImageGL representation around to avoid overhead

    struct Animate {
        explicit Animate(CameraWidget& cameraWidget);

        enum class Mode : std::uint8_t { Continuous, Swing };

        using ms = typename Timer::Milliseconds;
        CameraWidget* widget;
        BoolCompositeProperty props;
        IntProperty fps;
        OptionProperty<RotationAxis> type;
        OptionProperty<Mode> mode;
        FloatProperty increment;
        FloatProperty amplitude;
        EventProperty playPause;
        vec3 axis;
        Timer timer;
        bool paused;
        float currentDirection;
        float counter;
        vec3 viewDir;
        vec3 lookTo;
        vec3 lookUp;

        void startStopAnimation(bool start);
        void animate();
        void invokeEvent(Event* e);
    };

    Animate animate_;
};

}  // namespace inviwo
