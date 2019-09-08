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

#ifndef IVW_CAMERAINTERACTIONWIDGET_H
#define IVW_CAMERAINTERACTIONWIDGET_H

#include <modules/userinterfacegl/userinterfaceglmodule.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

class MouseEvent;
class Mesh;
class MeshDrawerGL;
class PickingObject;
class Image;
class ImageGL;

/** \docpage{org.inviwo.CameraWidget, Camera Interaction Processor}
 * ![](org.inviwo.CameraWidget.png?classIdentifier=org.inviwo.CameraWidget)
 * This processor provides a widget for manipulating the camera orientation with a mouse. The widget
 * is rendered on top of the input image. It also provides the rotation in matrix form.
 *
 * ### Inports
 *   * __ImageInport__ Input image
 *
 * ### Outports
 *   * __ImageOutport__ Output image
 *
 * ### Properties
 *   * __Enabled (settings)__ Enables interactions with the widget
 *   * __Invert Directions (settings)__ Inverts the rotation directions
 *   * __Camera Roll (settings)__ Shows an additional widget for rolling the camera
 *   * __Camera Dolly (settings)__ Shows an additional widget for camera dolly
 *   * __Speed Scaling (settings)__ Scaling factor (sensitivity) for rotation with a mouse drag
 *   * __Angle per click (settings)__  Rotation angle in degrees when a rotation is triggered by a
 * mouse click
 *   * __Scaling (appearance)__ Scales the size of the widget (a factor of 1 corresponds to 300
 * pixel)
 *   * __Position (appearance)__  Positioning of the interaction widget within the input image
 *   * __Anchor (appearance)__ Anchor position of the widget
 *   * __Show Cube (appearance)__ Toggles a cube behind the widget for showing the camera
 * orientation
 *   * __Cube Color (appearance)__ Custom color for the cube
 *   * __RGB Axis Coloring (appearance)__ Map red, green, and blue to the respective orientation
 * arrows of the widget
 *   * __User Color (appearance)__ Apply a custom color onto the entire widget
 *   * __Camera (output)__  Camera affected by the widget interaction
 *   * __Rotation Matrix (output)__  Matrix representing the camera orientation
 */

/**
 * \class CameraWidget
 * \brief provides a mesh-based interaction widget for the camera rotation
 */
class IVW_MODULE_USERINTERFACEGL_API CameraWidget : public Processor {
public:
    enum class Interaction { HorizontalRotation, VerticalRotation, FreeRotation, Roll, Zoom, None };

    CameraWidget();
    virtual ~CameraWidget();

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void updateWidgetTexture(const ivec2 &widgetSize);
    void drawWidgetTexture();

    void objectPicked(PickingEvent *p);
    void saveInitialCameraState();
    void loadMesh();

    void interaction(Interaction dir, dvec2 mouseDelta);
    void singleStepInteraction(Interaction dir, bool clockwise);

    void rotation(Interaction dir, dvec2 mouseDelta);
    void singleStepRotation(Interaction dir, bool clockwise);
    void zoom(dvec2 delta);
    void singleStepZoom(bool zoomIn);

    void updateOutput(const mat4 &rotation);

    static int interactionDirectionToInt(Interaction dir);
    static Interaction intToInteractionDirection(int dir);
    vec3 getObjectRotationAxis(const vec3 &rotAxis) const;

    ImageInport inport_;
    ImageOutport outport_;

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

    CompositeProperty interactions_;
    ButtonProperty rotateUpBtn_;
    ButtonProperty rotateDownBtn_;
    ButtonProperty rotateLeftBtn_;
    ButtonProperty rotateRightBtn_;

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

    // number of available interaction elements. Each interaction widget has two trigger areas to
    // distinguish the direction of rotation when clicked
    static const int numInteractionWidgets = 10;  //!< horizontal (left, right), vertical (up,
                                                  //!< down), center (2), roll (left, right), zoom
                                                  //!< (out, in)

    // meshes of the interaction widgets.
    // 1) camera widget
    // 2) camera widget including widget for "camera roll"
    // 3) camera zoom buttons
    // 4) cube
    std::array<std::unique_ptr<MeshDrawerGL>, 4> meshDrawers_;
    std::array<std::shared_ptr<const Mesh>, 4> meshes_;

    struct PickIDs {
        std::size_t id;
        Interaction dir;
        bool clockwise;
    };
    std::array<PickIDs, numInteractionWidgets> pickingIDs_;

    // UI state and textures
    bool isMouseBeingPressedAndHold_;
    bool mouseWasMoved_;
    int currentPickingID_;

    // initial state of camera when an interaction is triggered to keep the rotation axis consistent
    struct InitialState {
        vec3 camDir = vec3(0.0f, 0.0f, -1.0f);
        vec3 camUp = vec3(0.0f, 1.0f, 0.0f);
        vec3 camRight = vec3(1.0f, 0.0f, 0.0f);
        double zoom_ = 1.0f;
    } initialState_;

    std::unique_ptr<Image> widgetImage_;  //!< the widget is rendered into this image, which is then
                                          //!< drawn on top of the input image
    ImageGL *widgetImageGL_;  //!< keep an ImageGL representation around to avoid overhead
};

}  // namespace inviwo

#endif  // IVW_CAMERAINTERACTIONWIDGET_H
