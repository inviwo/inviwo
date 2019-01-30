/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_MPRENTRYEXITPOINTSPROCESSOR_H
#define IVW_MPRENTRYEXITPOINTSPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/opengl/shader/shader.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>

#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.MPREntryExitPoints, Entry exit points}
 * ![](org.inviwo.MPREntryExitPoints.png?classIdentifier=org.inviwo.MPREntryExitPoints)
 *
 * Uses a plane in volume space to generate entry and exit points
 *
 * ### Outports
 *   * __ImageOutport__ entry points
 *   * __ImageOutport__ exit points
 *
 * ### ToDo's:
 *  - rename all properties appropriately
 *  - make offset along normal in mm according to volume spacing
 * 
 */

class IVW_MODULE_BASEGL_API MPREntryExitPoints : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    
    MPREntryExitPoints();
    virtual ~MPREntryExitPoints();

    virtual void process() override;

    // override to do member renaming.
    virtual void deserialize(Deserializer& d) override;

private:
    VolumeInport volumeInport_;

    ImageOutport entryPort_;
    ImageOutport exitPort_;

    FloatProperty offset0_;
    FloatProperty offset1_;
    FloatProperty zoomFactor_;
    FloatProperty correctionAngle_;

    IntSize3Property volumeDimensions_;
    FloatVec3Property volumeSpacing_;

    FloatVec2Property cursorScreenPos_;
    vec2 cursorScreenPos_old_;

    FloatVec3Property mprP_;
    vec3 mprP_old_;
    FloatVec3Property mprBasisR_;
    FloatVec3Property mprBasisU_;
    FloatVec3Property mprBasisN_;

    Shader shader_;

    bool dirty_;

    vec3 screenPosToVolumePos(const vec2& screen_pos) const;
    vec3 screenOffsetToVolumeOffset(const vec2& screen_offset) const;

    vec2 volumePosToScreenPos(const vec3& volume_pos) const;
    vec2 volumeOffsetToScreenOffset(const vec3& volume_offset) const;
};

} // namespace

#endif // IVW_MPRENTRYEXITPOINTSPROCESSOR_H
