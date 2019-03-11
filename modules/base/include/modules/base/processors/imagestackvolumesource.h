/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>

namespace inviwo {

class FileExtension;
class Volume;
class InviwoApplication;

/** \docpage{org.inviwo.ImageStackVolumeSource, Image-Stack Volume Source}
 * Converts a stack of 2D images to a 3D volume
 * ![](org.inviwo.ImageStackVolumeSource.png?classIdentifier=org.inviwo.ImageStackVolumeSource)
 *
 * The format of the volume will be equivalent to the first input image. The volume resolution
 * will be equal to the size of the first image times the number of images. The physical size of the
 * volume is determined by the the voxel spacing property.
 *
 * The input images are converted to a volume representation based on the input channel selection.
 * Single channels, i.e. red, green, blue, alpha, and grayscale, will result in a scalar volume
 * whereas rgb and rgba will yield a vec3 or vec4 volume, respectively.
 *
 * ### Outports
 *   * __volume__ Volume generated from a stack of input images.
 *
 * ### Properties
 *   * __File Pattern__           Pattern used for multi-file matching of images
 *   * __Skip Unsupported Files__   If true, matching files with unsupported image formats are
 *                                not considered. Otherwise an empty volume slice will be inserted
 *                               for each file.
 *   * __Voxel Spacing__          Used to match the sampling distance of the acquired data and
 *                                affects the physical size of the volume.
 *   * __Data Information__       Metadata of the generated volume data set.
 *
 */
class IVW_MODULE_BASE_API ImageStackVolumeSource : public Processor {
public:
    ImageStackVolumeSource(InviwoApplication* app);
    void addFileNameFilters();
    virtual ~ImageStackVolumeSource() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    std::shared_ptr<Volume> load();
    bool isValidImageFile(std::string);

    virtual void deserialize(Deserializer& d) override;

private:
    VolumeOutport outport_;
    FilePatternProperty filePattern_;
    ButtonProperty reload_;
    BoolProperty skipUnsupportedFiles_;

    BasisProperty basis_;
    VolumeInformationProperty information_;

    std::shared_ptr<Volume> volume_;
    bool deserialized_ = false;

    DataReaderFactory* readerFactory_;
};

}  // namespace inviwo
