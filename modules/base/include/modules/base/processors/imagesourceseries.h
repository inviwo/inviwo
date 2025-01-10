/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/imageport.h>                 // for ImageOutport
#include <inviwo/core/processors/processor.h>            // for Processor
#include <inviwo/core/processors/processorinfo.h>        // for ProcessorInfo
#include <inviwo/core/properties/buttonproperty.h>       // for ButtonProperty
#include <inviwo/core/properties/filepatternproperty.h>  // for FilePatternProperty
#include <inviwo/core/properties/ordinalproperty.h>      // for IntProperty
#include <inviwo/core/properties/stringproperty.h>       // for StringProperty
#include <inviwo/core/util/fileextension.h>              // for FileExtension

#include <string>  // for string
#include <vector>  // for vector

namespace inviwo {

class InviwoApplication;

/** \docpage{org.inviwo.ImageSourceSeries, Image Series Source}
 * ![](org.inviwo.ImageSourceSeries.png?classIdentifier=org.inviwo.ImageSourceSeries)
 *
 * Provides functionality to pick a single image from a list of files matching a pattern or
 * selection.
 *
 * ### Outports
 *   * __image.outport__ Selected image
 *
 * ### Properties
 *   * __File Pattern__ Pattern used for multi-file matching including path
 *   * __Image Index__  Index of selected image file
 *   * __Image File Name__  Name of the selected file (read-only)
 *   * __Update File List__ Reload the list of matching images
 *
 */
class IVW_MODULE_BASE_API ImageSourceSeries : public Processor {
public:
    ImageSourceSeries(InviwoApplication* app);
    virtual ~ImageSourceSeries() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

protected:
    virtual void onFindFiles();
    bool isValidImageFile(const std::filesystem::path& path);
    void updateProperties();
    void updateFileName();

private:
    ImageOutport outport_;
    ButtonProperty findFilesButton_;

    FilePatternProperty imageFilePattern_;
    IntProperty currentImageIndex_;
    StringProperty imageFileName_;

    std::vector<FileExtension> validExtensions_;
    std::vector<std::filesystem::path> fileList_;
};

}  // namespace inviwo
