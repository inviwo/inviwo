/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_ORIENTATIONINDICATOR_H
#define IVW_ORIENTATIONINDICATOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {

class Mesh;

/** \docpage{org.inviwo.OrientationIndicator, Orientation Marker}
 * ![](org.inviwo.OrientationIndicator.png?classIdentifier=org.inviwo.OrientationIndicator)
 * Generates a mesh with three arrows indication the direction of the three coordinate axes.
 *
 * ### Outport
 *   * __mesh__ The generated indicator mesh
 *
 * ### Properties
 *   * __Base Color__ Color of the central sphere
 *   * __X axis Color__ Color of the first arrow
 *   * __Y axis Color__ Color of the second arrow
 *   * __Z axis Color__ Color of the third arrow
 *   * __Scale__ Overall scaling factor for the widget
 *   * __Radius__ Radius of arrows
 *   * __Axis scale__ scaling factors for each arrow.
 *   * __Location Type__ Locate the widget in image space (2D) or world space (3D)
 *   * __View Coords__ Where to put the widget in normalized image coordinates
 *   * __Offset__ Where to put the widget in world space
 */
class IVW_MODULE_BASE_API OrientationIndicator : public Processor {
public:
    enum class Location { ThreeD, TwoD };

    OrientationIndicator();
    virtual ~OrientationIndicator() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void updateMesh();

    MeshOutport outport_;

    FloatVec4Property baseColor_;
    FloatVec4Property xColor_;
    FloatVec4Property yColor_;
    FloatVec4Property zColor_;

    FloatProperty scale_;
    FloatVec3Property axisScale_;
    FloatProperty radius_;

    CompositeProperty location_;
    TemplateOptionProperty<Location> locationType_;
    FloatVec2Property viewCoords_;
    CameraProperty cam_;

    FloatVec3Property offset_;

    std::shared_ptr<Mesh> mesh_;
};

}  // namespace inviwo

#endif  // IVW_ORIENTATIONINDICATOR_H
