/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMESLICEGL_H
#define IVW_VOLUMESLICEGL_H

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

class Mesh;

/** \docpage{org.inviwo.VolumeSliceGL, Volume Slice (GL)}
 * ![](org.inviwo.VolumeSliceGL.png?classIdentifier=org.inviwo.VolumeSliceGL)
 * This processor extracts an arbitrary 2D slice from an input volume.
 *
 * ### Inports
 *   * __volume__ The input volume
 *
 * ### Outports
 *   * __outport__ The extracted volume slice
 *
 * ### Properties
 *   * __Slice along axis__ Defines the volume axis or plane normal for the output slice
 *   * __X Volume Position__ Position of the slice if the x axis is chosen
 *   * __Y Volume Position__ Position of the slice if the y axis is chosen
 *   * __Z Volume Position__ Position of the slice if the z axis is chosen
 *   * __Plane Normal__ Defines the normal of the slice plane (if slice axis is set to "Plane
 *                      Equation")
 *   * __Plane Position__ Defines the origin of the slice plane (if slice axis is set to "Plane
 *                        Equation")
 *   * __Transformations__
 *      + __Rotation (ccw)__ Defines the rotation of the output image
 *      + __Angle__ Angle of rotation if "Free Rotation" is chosen as rotation
 *      + __Scale__ Scaling factor applied to the volume slice
 *      + __Horizontal Flip__ Flips the output image left and right
 *      + __Vertical Flip__ Flips the output image up and down
 *      + __Volume Texture Wrapping__ Texture wrapping mode used for extracting the image slice
 *          (use fill color, repeat edge values, repeat the contents, mirror contents)
 *      + __Fill Color__ Defines the color which is used if the texture wrapping  is set to "Fill
 *                       with Color"
 *   * __Position Selection__
 *      + __Enable Picking__ Enables selecting the position selection with the mouse
 *      + __Show Position Indicator__ Toggles the visibility of the position indicator
 *      + __Indicator Color__  Custom color of the position indicator
 *   * __Transfer Function Properties__
 *      + __Enable Transfer Function__ Toggles whether the transfer function is applied onto the
 *                                     extracted volume slice
 *      + __Transfer Function__ Defines the transfer function for mapping voxel values to color and
 *                              opacity
 *      + __Alpha Offset__ Offset
 *   * __World Position__ Outputs the world position of the slice plane (read-only)
 *   * __Handle interaction events__ Toggles whether this processor will handle interaction events
 *                                   like mouse buttons or key presses
 */

/**
 * \class VolumeSliceGL
 * \brief extracts an arbitrary 2D slice from an input volume
 */
class IVW_MODULE_BASEGL_API VolumeSliceGL : public Processor {
public:
    VolumeSliceGL();
    virtual ~VolumeSliceGL();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

    // Overridden to be able to turn off interaction events.
    virtual void invokeEvent(Event*) override;

    bool positionModeEnabled() const { return posPicking_.get(); }

    // override to do member renaming.
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual void process() override;

    void shiftSlice(int);

    void modeChange();
    void planeSettingsChanged();
    void updateMaxSliceNumber();

    void renderPositionIndicator();
    void updateIndicatorMesh();

    // updates the selected position, pos is given in normalized viewport coordinates, i.e. [0,1]
    void setVolPosFromScreenPos(vec2 pos);
    vec2 getScreenPosFromVolPos();

    vec3 convertScreenPosToVolume(const vec2& screenPos, bool clamp = true) const;

    void invalidateMesh();

    void sliceChange();
    void positionChange();
    void rotationModeChange();

private:
    void eventShiftSlice(Event*);
    void eventSetMarker(Event*);
    void eventStepSliceUp(Event*);
    void eventStepSliceDown(Event*);
    void eventGestureShiftSlice(Event*);
    void eventUpdateMousePos(Event*);

    void updateFromWorldPosition();

    VolumeInport inport_;
    ImageOutport outport_;
    Shader shader_;
    Shader indicatorShader_;

    CompositeProperty trafoGroup_;
    CompositeProperty pickGroup_;
    CompositeProperty tfGroup_;

    OptionPropertyInt sliceAlongAxis_;  // Axis enum
    IntProperty sliceX_;
    IntProperty sliceY_;
    IntProperty sliceZ_;

    FloatVec3Property worldPosition_;

    FloatVec3Property planeNormal_;
    FloatVec3Property planePosition_;
    FloatProperty imageScale_;
    OptionPropertyInt rotationAroundAxis_;  // Clockwise rotation around slice axis
    FloatProperty imageRotation_;
    BoolProperty flipHorizontal_;
    BoolProperty flipVertical_;
    OptionPropertyInt volumeWrapping_;
    FloatVec4Property fillColor_;

    BoolProperty posPicking_;
    BoolProperty showIndicator_;
    FloatVec4Property indicatorColor_;
    FloatProperty indicatorSize_;

    BoolProperty tfMappingEnabled_;
    TransferFunctionProperty transferFunction_;
    FloatProperty tfAlphaOffset_;

    BoolCompositeProperty sampleQuery_;
    FloatVec4Property normalizedSample_;
    FloatVec4Property volumeSample_;

    BoolProperty handleInteractionEvents_;

    EventProperty mouseShiftSlice_;
    EventProperty mouseSetMarker_;
    EventProperty mousePositionTracker_;

    EventProperty stepSliceUp_;
    EventProperty stepSliceDown_;

    EventProperty gestureShiftSlice_;

    std::unique_ptr<Mesh> meshCrossHair_;

    bool meshDirty_;
    bool updating_;

    mat4 sliceRotation_;
    mat4 inverseSliceRotation_;  // Used to calculate the slice "z position" from the plain point.
    size3_t volumeDimensions_;
    mat4 texToWorld_;
};
}  // namespace inviwo

#endif  // IVW_VOLUMESLICEGL_H
