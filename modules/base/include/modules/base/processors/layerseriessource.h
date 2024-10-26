/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#pragma once

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/properties/buttonproperty.h>       // for ButtonProperty
#include <inviwo/core/properties/filepatternproperty.h>  // for FilePatternProperty
#include <inviwo/core/properties/ordinalproperty.h>      // for IntProperty
#include <inviwo/core/properties/stringproperty.h>       // for StringProperty
#include <inviwo/core/util/fileextension.h>              // for FileExtension

namespace inviwo {

class InviwoApplication;

class IVW_MODULE_BASE_API LayerSeriesSource : public Processor {
public:
    LayerSeriesSource(InviwoApplication* app);

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void onFindFiles();
    bool isValidImageFile(const std::filesystem::path& path);
    void updateProperties();
    void updateFileName();

    LayerOutport outport_;
    FilePatternProperty filePattern_;
    ButtonProperty findFilesButton_;
    IntProperty currentIndex_;
    StringProperty fileName_;

    std::vector<FileExtension> validExtensions_;
    std::vector<std::filesystem::path> fileList_;
};

}  // namespace inviwo
