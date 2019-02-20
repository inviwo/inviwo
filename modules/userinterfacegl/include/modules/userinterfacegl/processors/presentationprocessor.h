/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_PRESENTATIONPROCESSOR_H
#define IVW_PRESENTATIONPROCESSOR_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

class Event;

/** \docpage{org.inviwo.PresentationProcessor, Presentation Switch}
 * ![](org.inviwo.PresentationProcessor.png?classIdentifier=org.inviwo.PresentationProcessor)
 * Allows switching between slide images and a regular image inport (e.g. rendered image)
 *
 * ### Inports
 *   * __inport__  regular image inport connected to the rendering network
 *
 * ### Outports
 *   * __outport__  either the regular inport image or a slide depending on Slide Mode
 *
 * ### Properties
 *   * __Presentation Mode__   if true the current slide is shown, otherwise the image of the inport
 *   * __File Pattern__ Pattern used for multi-file matching including path
 *   * __Slide Index__  Index of currently selected slide
 *   * __Image File Name__  Name of the selected file (read-only)
 *   * __Update File List__ Reload the list of matching slide images
 */

/**
 * \class PresentationProcessor
 * \brief processor for switching between slide images and another image inport
 */
class IVW_MODULE_USERINTERFACEGL_API PresentationProcessor : public Processor {
public:
    PresentationProcessor();
    virtual ~PresentationProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void updateSlideImage();
    void onFindFiles();
    bool isValidImageFile(std::string);
    void updateProperties();
    void updateFileName();

    void nextSlide(Event *e);
    void previousSlide(Event *e);

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty presentationMode_;

    FilePatternProperty imageFilePattern_;
    ButtonProperty findFilesButton_;
    IntProperty slideIndex_;
    StringProperty imageFileName_;

    CompositeProperty interactions_;
    EventProperty toggleMode_;
    EventProperty quitPresentation_;
    EventProperty nextSlide_;
    EventProperty prevSlide_;
    EventProperty nextSlideAlt_;
    EventProperty prevSlideAlt_;
    EventProperty mouseNextSlide_;
    EventProperty mousePrevSlide_;

    std::vector<FileExtension> validExtensions_;
    std::vector<std::string> fileList_;

    std::shared_ptr<Image> currentSlide_;
};

}  // namespace inviwo

#endif  // IVW_PRESENTATIONPROCESSOR_H
