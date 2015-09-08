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

#ifndef IVW_IMAGESOURCESERIES_H
#define IVW_IMAGESOURCESERIES_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

class FileExtension;

/** \docpage{org.inviwo.ImageSourceSeries, Image Series Source}
 * ![](org.inviwo.ImageSourceSeries.png?classIdentifier=org.inviwo.ImageSourceSeries)
 *
 * ...
 * 
 * 
 * ### Outports
 *   * __image.outport__ ...
 * 
 * ### Properties
 *   * __Image file name__ ...
 *   * __Image file directory", "image__ ...
 *   * __Image index__ ...
 *   * __Update File List__ ...
 *
 */
class IVW_MODULE_BASE_API ImageSourceSeries : public Processor {
public:
    ImageSourceSeries();
    ~ImageSourceSeries() = default;

    InviwoProcessorInfo();

    virtual void onFindFiles();

protected:
    virtual void process();

    bool isValidImageFile(std::string);

    void updateProperties();
    void updateFileName();

private:
    ImageOutport outport_;
    ButtonProperty findFilesButton_;
    DirectoryProperty imageFileDirectory_;
    IntProperty currentImageIndex_;
    StringProperty imageFileName_;

    std::vector<FileExtension> validExtensions_;
    std::vector<std::string> fileList_;
};

} // namespace

#endif // IVW_IMAGESOURCESERIES_H
