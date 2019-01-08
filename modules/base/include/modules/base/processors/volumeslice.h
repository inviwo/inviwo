/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_VOLUMESLICE_H
#define IVW_VOLUMESLICE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <modules/base/datastructures/imagereusecache.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeSlice, Volume Slice}
 * ![](org.inviwo.VolumeSlice.png?classIdentifier=org.inviwo.VolumeSlice)
 * This processor extracts an axis aligned 2D slice from an input volume.
 *
 * ### Inports
 *   * __VolumeInport__ The input volume
 *
 * ### Outports
 *   * __ImageOutport__ The extracted volume slice
 *
 * ### Properties
 *   * __sliceAlongAxis_ Defines the volume axis for the output slice
 *   * __sliceNumber_ Defines the slice number for the output slice
 */

/**
 * \brief Outputs a slice from a volume, CPU-based
 */
class IVW_MODULE_BASE_API VolumeSlice : public Processor {
public:
    VolumeSlice();
    ~VolumeSlice();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void invokeEvent(Event* event) override;

protected:
    virtual void process() override;

    void shiftSlice(int);

private:
    void eventShiftSlice(Event*);
    void eventStepSliceUp(Event*);
    void eventStepSliceDown(Event*);
    void eventGestureShiftSlice(Event*);

    VolumeInport inport_;
    ImageOutport outport_;

    ImageReuseCache imageCache_;

    TemplateOptionProperty<CartesianCoordinateAxis> sliceAlongAxis_;
    IntSizeTProperty sliceNumber_;

    BoolProperty handleInteractionEvents_;

    EventProperty mouseShiftSlice_;

    EventProperty stepSliceUp_;
    EventProperty stepSliceDown_;

    EventProperty gestureShiftSlice_;
};

}  // namespace inviwo
#endif  // IVW_VOLUMESLICE_H
