/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include "tonemappingcpuprocessor.h"
#include <inviwo/core/datastructures/image/imageram.h>
#include <modules/freeimage/freeimageutils.h>

namespace inviwo {

ProcessorClassIdentifier(ToneMappingCPUProcessor, "org.inviwo.ToneMappingCPU");
ProcessorDisplayName(ToneMappingCPUProcessor,  "Tone Mapping");
ProcessorTags(ToneMappingCPUProcessor, Tags::CPU);
ProcessorCategory(ToneMappingCPUProcessor, "Image Processing");
ProcessorCodeState(ToneMappingCPUProcessor, CODE_STATE_BROKEN);

ToneMappingCPUProcessor::ToneMappingCPUProcessor()
    : Processor(),
      inport_("image.inport"),
      outport_("image.outport", COLOR_ONLY, DataVec3UINT8::get()),
      toneMappingMethod_("toneMappingMethod", "Tone Mapping Method"),
      drago03Gamma_("drago03Gamma", "Gamma", 2.2, 0.0, 5.0),
      drago03Exposure_("drago03Exposure", "Exposure", 0.0, 0.0, 5.0),
      reinhard05Intensity_("reinhard05Intensity", "Intensity", 0.0, 0.0, 5.0),
      reinhard05Contrast_("reinhard05Contrast", "Contrast", 0.0, 0.0, 5.0),
      fattal02Saturation_("fattal02Saturation", "Saturation", 0.5, 0.0, 5.0),
      fattal02Attenuation_("fattal02Attenuation", "Attenuation", 0.85, 0.0, 5.0)
{
    addPort(inport_);
    addPort(outport_);

    toneMappingMethod_.addOption("DRAGO03",          "Adaptive logarithmic mapping",     "FITMO_DRAGO03");
    toneMappingMethod_.addOption("REINHARD05",       "Dynamic range reduction",          "FITMO_REINHARD05");
    toneMappingMethod_.addOption("FATTAL02",         "Gradient domain compression",      "FITMO_FATTAL02");
    toneMappingMethod_.addOption("PASS",             "Pass-through",                     "PASS");
    toneMappingMethod_.setSelectedIndex(0);
    toneMappingMethod_.onChange(this, &ToneMappingCPUProcessor::toneMappingMethodChanged);
    addProperty(toneMappingMethod_);

    addProperty(drago03Gamma_);
    addProperty(drago03Exposure_);

    addProperty(reinhard05Intensity_);
    addProperty(reinhard05Contrast_);

    addProperty(fattal02Saturation_);
    addProperty(fattal02Attenuation_);

    setAllPropertiesCurrentStateAsDefault();
}

ToneMappingCPUProcessor::~ToneMappingCPUProcessor() {}

void ToneMappingCPUProcessor::initialize() {
    Processor::initialize();
    toneMappingMethodChanged();
}

void ToneMappingCPUProcessor::deinitialize() {
    Processor::deinitialize();
}

void ToneMappingCPUProcessor::process() {
    if(passSelected_){
        outport_.setData(inport_.getData()->clone());
        return;
    }

    const ImageRAM* inImageRam = inport_.getData()->getRepresentation<ImageRAM>();
    if(inImageRam){
        FREE_IMAGE_TMO tmo;
        double first = 0.0;
        double second = 0.0;

        if(drago03Selected_){
            tmo = FITMO_DRAGO03;
            first = drago03Gamma_.get();
            second = drago03Exposure_.get();
        }
        else if(reinhard05Selected_){
            tmo = FITMO_REINHARD05;
            first = reinhard05Intensity_.get();
            second = reinhard05Contrast_.get();
        }
        else{
            tmo = FITMO_FATTAL02;
            first = fattal02Saturation_.get();
            second = fattal02Attenuation_.get();
        }

        FIBITMAP* bitmap = FreeImageUtils::createBitmapFromData(inImageRam->getColorLayerRAM());
        FreeImage_ToneMapping(bitmap, tmo, first, second);

        if(outport_.getData()->getDataFormat() != DataVec3UINT8::get() || outport_.getData()->getDimensions() != inImageRam->getDimensions()){
            outport_.setData(new Image(outport_.getData()->getDimensions(), COLOR_ONLY, DataVec3UINT8::get()));
        }
        
        ImageRAM* outImageRam = outport_.getData()->getEditableRepresentation<ImageRAM>();
        LayerRAM* out = outImageRam->getColorLayerRAM();
        FreeImageUtils::copyBitmapToData(bitmap, out);

        FreeImage_Unload(bitmap);
    }
}

void ToneMappingCPUProcessor::toneMappingMethodChanged() {
    drago03Selected_ = (toneMappingMethod_.getSelectedIdentifier() == "DRAGO03");
    reinhard05Selected_ = (toneMappingMethod_.getSelectedIdentifier() == "REINHARD05");
    fattal02Selected_ = (toneMappingMethod_.getSelectedIdentifier() == "FATTAL02");
    passSelected_ = (toneMappingMethod_.getSelectedIdentifier() == "PASS");

    drago03Gamma_.setVisible(drago03Selected_);
    drago03Exposure_.setVisible(drago03Selected_);

    reinhard05Intensity_.setVisible(reinhard05Selected_);
    reinhard05Contrast_.setVisible(reinhard05Selected_);

    fattal02Saturation_.setVisible(fattal02Selected_);
    fattal02Attenuation_.setVisible(fattal02Selected_);
}

} // namespace
