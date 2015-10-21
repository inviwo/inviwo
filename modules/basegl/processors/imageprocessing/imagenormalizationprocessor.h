/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_IMAGENORMALIZATIONPROCESSOR_H
#define IVW_IMAGENORMALIZATIONPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <modules/opengl/inviwoopengl.h>
    

namespace inviwo {

/** \docpage{org.inviwo.ImageNormalization, Image Normalization}
 * ![](org.inviwo.ImageNormalization.png?classIdentifier=org.inviwo.ImageNormalization)
 *
 * ...
 * 
 * 
 * 
 * ### Properties
 *   * __Normalize Channels Individually__ ...
 *
 */
class IVW_MODULE_BASEGL_API ImageNormalizationProcessor : public ImageGLProcessor {
public:
    ImageNormalizationProcessor();
    virtual ~ImageNormalizationProcessor();

    InviwoProcessorInfo();

    void invalidateMinMax();
    void updateMinMax();

protected:
    virtual void preProcess() override;

private:
    bool minMaxInvalid_;
    BoolProperty eachChannelsIndividually_;
    BoolProperty zeroAtPoint5_;
    StringProperty minS_;
    StringProperty maxS_;
    dvec4 min_;
    dvec4 max_;
};

}  // namespace

#endif  // IVW_IMAGENORMALIZATIONPROCESSOR_H
