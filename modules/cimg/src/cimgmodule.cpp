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

#include <modules/cimg/cimgmodule.h>
#include <modules/cimg/cimglayerreader.h>
#include <modules/cimg/cimglayerwriter.h>
#include <modules/cimg/cimgvolumereader.h>
#include <modules/cimg/cimgutils.h>
#include <modules/cimg/tifflayerreader.h>
#include <modules/cimg/tiffstackvolumereader.h>

namespace inviwo {

CImgModule::CImgModule(InviwoApplication* app) : InviwoModule(app, "CImg") {
    // Register Data Readers
    registerDataReader(std::make_unique<CImgLayerReader>());
    registerDataReader(std::make_unique<TIFFLayerReader>());
    registerDataReader(std::make_unique<TIFFStackVolumeReader>());

    // Register Data Writers
    registerDataWriter(std::make_unique<CImgLayerWriter>());

    LogInfo("Using LibJPG Version " << cimgutil::getLibJPGVersion());
    LogInfo("Using OpenEXR Version " << cimgutil::getOpenEXRVersion());
}

}  // namespace inviwo
