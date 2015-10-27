/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeSlice, Volume Slice}
 * ![](org.inviwo.VolumeSlice.png?classIdentifier=org.inviwo.VolumeSlice)
 * Outputs a slice from a volume, CPU-based
 *
 * ### Inports
 *   * __VolumeInport__ The input volume.
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 *
 * ### Properties
 *   * __sliceAlongAxis_ Defines the volume axis for the output slice.
 *   * __sliceNumber_ Defines the slice number for the output slice.
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
    struct VolumeSliceDispatcher {
        using type = std::shared_ptr<Image>;
        template <class T>
        std::shared_ptr<Image> dispatch(const Volume& vol,
                                        CartesianCoordinateAxis axis, size_t slice,
                                        std::shared_ptr<Image> img);
    };

    void eventShiftSlice(Event*);
    void eventStepSliceUp(Event*);
    void eventStepSliceDown(Event*);
    void eventGestureShiftSlice(Event*);

    VolumeInport inport_;
    ImageOutport outport_;
    std::shared_ptr<Image> image_;

    OptionPropertyInt sliceAlongAxis_;
    IntProperty sliceNumber_;

    BoolProperty handleInteractionEvents_;

    EventProperty mouseShiftSlice_;

    EventProperty stepSliceUp_;
    EventProperty stepSliceDown_;

    EventProperty gestureShiftSlice_;
};

template <class T>
std::shared_ptr<Image> VolumeSlice::VolumeSliceDispatcher::dispatch(
    const Volume& vol, CartesianCoordinateAxis axis, size_t slice,
    std::shared_ptr<Image> img) {
    // D = type of the data
    typedef typename T::type D;

    std::shared_ptr<Image> image;

    const DataFormatBase* format = vol.getDataFormat();
    const size3_t voldim = vol.getDimensions();

    // Calculate image dimensions
    size2_t dim;
    switch (axis) {
        case CartesianCoordinateAxis::X:
            dim = size2_t(voldim.z, voldim.y);
            break;
        case CartesianCoordinateAxis::Y:
            dim = size2_t(voldim.x, voldim.z);
            break;
        case CartesianCoordinateAxis::Z:
            dim = size2_t(voldim.x, voldim.y);
            break;
    }

    // Check that the format is right
    if (img && format == img->getDataFormat() && dim == img->getDimensions()) {
        image = img;
    } else {
        image = std::make_shared<Image>(dim, format);
    }

    // Make sure there is a ImageRAM in image, and get LayerRAM
    LayerRAMPrecision<D>* layer = dynamic_cast<LayerRAMPrecision<D>*>(
        image->getEditableRepresentation<ImageRAM>()->getColorLayerRAM());

    if (!layer) return nullptr;

    D* layerdata = static_cast<D*>(layer->getData());
    const D* voldata = static_cast<const D*>(vol.getRepresentation<VolumeRAM>()->getData());

    size_t offsetVolume;
    size_t offsetImage;
    switch (axis) {
        case CartesianCoordinateAxis::X: {
            slice = std::min(slice, static_cast<size_t>(voldim.x - 1));

            for (size_t i = 0; i < voldim.z; i++) {
                for (size_t j = 0; j < voldim.y; j++) {
                    offsetVolume = (i * voldim.x * voldim.y) + (j * voldim.x) + slice;
                    offsetImage = (j * voldim.z) + i;
                    layerdata[offsetImage] = voldata[offsetVolume];
                }
            }

            break;
        }
        case CartesianCoordinateAxis::Y: {
            slice = std::min(slice, static_cast<size_t>(voldim.y - 1));

            size_t dataSize = voldim.x * static_cast<size_t>(format->getSize());
            size_t initialStartPos = slice * voldim.x;
            for (size_t j = 0; j < voldim.z; j++) {
                offsetVolume = (j * voldim.x * voldim.y) + initialStartPos;
                offsetImage = j * voldim.x;
                std::memcpy(layerdata + offsetImage, voldata + offsetVolume, dataSize);
            }
            break;
        }
        case CartesianCoordinateAxis::Z: {
            slice = std::min(slice, static_cast<size_t>(voldim.z - 1));

            size_t dataSize = voldim.x * voldim.y * static_cast<size_t>(format->getSize());
            size_t initialStartPos = slice * voldim.x * voldim.y;

            std::memcpy(layerdata, voldata + initialStartPos, dataSize);

            break;
        }
    }
    return image;
}
}
#endif  // IVW_VOLUMESLICE_H
