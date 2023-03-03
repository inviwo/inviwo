/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
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

#include <modules/cimg/cimgmodule.h>

#include <inviwo/core/common/inviwoapplication.h>       // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>            // for InviwoModule
#include <inviwo/core/datastructures/image/layerram.h>  // for LayerRamResizer
#include <inviwo/core/io/datareader.h>                  // for DataReader
#include <inviwo/core/io/datawriter.h>                  // for DataWriter
#include <inviwo/core/util/logcentral.h>                // for LogCentral, LogInfo
#include <modules/cimg/cimglayerreader.h>               // for CImgLayerReader
#include <modules/cimg/cimglayerwriter.h>               // for CImgLayerWriter
#include <modules/cimg/cimgutils.h>                     // for getLibJPGVersion, getOpenEXRVersion
#include <modules/cimg/tifflayerreader.h>               // for TIFFLayerReader
#include <modules/cimg/tiffstackvolumereader.h>         // for TIFFStackVolumeReader

#include <ostream>  // for operator<<, char_traits

namespace inviwo {

class CIMGLayerRamResizer : public LayerRamResizer {
    virtual bool resize(const LayerRAM& src, LayerRAM& dst) const override {
        return cimgutil::rescaleLayerRamToLayerRam(&src, &dst);
    }
};

CImgModule::CImgModule(InviwoApplication* app)
    : InviwoModule(app, "CImg"), app_{app}, resizer_{std::make_unique<CIMGLayerRamResizer>()} {
    // Register Data Readers
    registerDataReader(std::make_unique<CImgLayerReader>());
    registerDataReader(std::make_unique<TIFFLayerReader>());
    registerDataReader(std::make_unique<TIFFStackVolumeReader>());

    // Register Data Writers
    registerDataWriter(std::make_unique<CImgLayerWriter>());

    if (!app_->getLayerRamResizer()) {
        app_->setLayerRamResizer(resizer_.get());
    }

    LogInfo("Using LibJPG Version " << cimgutil::getLibJPGVersion());
    LogInfo("Using OpenEXR Version " << cimgutil::getOpenEXRVersion());
}

CImgModule::~CImgModule() {
    if (app_->getLayerRamResizer() == resizer_.get()) {
        app_->setLayerRamResizer(nullptr);
    }
}

}  // namespace inviwo
