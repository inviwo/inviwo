/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_IMAGECONTOURPROCESSOR_H
#define IVW_IMAGECONTOURPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>

#include <modules/base/algorithm/image/imagecontour.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageContourProcessor, Image Contour Processor}
 * ![](org.inviwo.ImageContourProcessor.png?classIdentifier=org.inviwo.ImageContourProcessor)
 * Does marching squares on the image to extract a contour mesh.
 *
 * ### Inports
 *   * __Image__ Input image
 *
 * ### Outports
 *   * __Mesh__ Contour mesh
 *
 * ### Properties
 *   * __Channel__ The image channel to use compare the iso value to
 *   * __IsoValue__ The contour iso value
 *   * __Color__ The color of the resulting mesh
 */

class IVW_MODULE_BASE_API ImageContourProcessor : public Processor {
public:
    ImageContourProcessor();
    virtual ~ImageContourProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport image_;
    MeshOutport mesh_;
    IntSizeTProperty channel_;
    DoubleProperty isoValue_;
    FloatVec4Property color_;
};

}  // namespace inviwo

#endif  // IVW_IMAGECONTOURPROCESSOR_H
