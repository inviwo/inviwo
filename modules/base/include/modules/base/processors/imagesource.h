/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/ports/imageport.h>             // for ImageOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/buttonproperty.h>   // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>     // for FileProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for IntSize2Property
#include <inviwo/core/util/fileextension.h>          // for FileExtension, operator==, operator<<
#include <modules/base/processors/datasource.h>

namespace inviwo {

class InviwoApplication;

class IVW_MODULE_BASE_API ImageSource : public DataSource<Image, ImageOutport, Layer> {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    ImageSource(InviwoApplication* app, const std::filesystem::path& filePath = "");
    virtual ~ImageSource() = default;

    virtual std::shared_ptr<Image> transform(std::shared_ptr<Layer> layer) override;

    virtual void dataLoaded(std::shared_ptr<Image> data) override;
    virtual void dataDeserialized(std::shared_ptr<Image> data) override;

private:
    IntSize2Property dimensions_;
};

}  // namespace inviwo
