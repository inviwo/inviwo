/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/ffmpeg/ffmpegmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/ffmpeg/wrap/codecid.h>
#include <inviwo/ffmpeg/wrap/outputformat.h>
#include <inviwo/ffmpeg/recorder.h>

namespace inviwo {

class IVW_MODULE_FFMPEG_API MovieExport : public Processor {
public:
    MovieExport();
    virtual ~MovieExport();
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    bool guessFormat() const;
    ffmpeg::OutputFormat getOutputFormat() const;

    bool guessCodec() const;
    ffmpeg::CodecID getCodec(const ffmpeg::OutputFormat& outputFormat) const;

    ImageInport inport_;
    FileProperty file_;
    OptionPropertyString format_;
    StringProperty activeFormat_;
    OptionProperty<ffmpeg::CodecID> codec_;
    StringProperty activeCodec_;
    OptionProperty<ffmpeg::Recorder::Mode> mode_;
    IntProperty frameRate_;
    IntProperty bitRate_;
    ButtonProperty start_;
    ButtonProperty stop_;

    std::unique_ptr<ffmpeg::Recorder> recorder;
};

}  // namespace inviwo
