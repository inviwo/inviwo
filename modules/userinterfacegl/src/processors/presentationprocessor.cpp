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

#include <modules/userinterfacegl/processors/presentationprocessor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/interaction/events/event.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PresentationProcessor::processorInfo_{
    "org.inviwo.PresentationProcessor",  // Class identifier
    "Presentation Processor",            // Display name
    "UI",                                // Category
    CodeState::Stable,                   // Code state
    "CPU, UI, Presentation",             // Tags
};
const ProcessorInfo PresentationProcessor::getProcessorInfo() const { return processorInfo_; }

PresentationProcessor::PresentationProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , presentationMode_("presentationMode", "Presentation Mode", false)
    , imageFilePattern_("imageFilePattern", "Slide Images",
                        filesystem::getPath(PathType::Images, "/*"), "")
    , findFilesButton_("findFiles", "Update File List")
    , slideIndex_("slideIndex", "Slide Index", 1, 1, 1, 1)
    , imageFileName_("imageFileName", "Image File Name")
    , interactions_("interactions", "Interactions")
    , toggleMode_("toggleMode", "Toggle Mode",
                  [this](Event *e) {
                      presentationMode_.set(!presentationMode_.get());
                      e->markAsUsed();
                  },
                  IvwKey::P, KeyState::Press)
    , quitPresentation_("quitPresentation", "Quit Presentation",
                        [this](Event *e) {
                            presentationMode_.set(false);
                            e->markAsUsed();
                        },
                        IvwKey::Escape, KeyState::Press)
    , nextSlide_("nextSlide", "Next Slide", [this](Event *e) { nextSlide(e); }, IvwKey::Space,
                 KeyState::Press)
    , prevSlide_("prevSlide", "Previous Slide", [this](Event *e) { previousSlide(e); },
                 IvwKey::Backspace, KeyState::Press)
    , nextSlideAlt_("nextSlideAlt", "Next Slide (alternative)", [this](Event *e) { nextSlide(e); },
                    IvwKey::Right, KeyState::Press)
    , prevSlideAlt_("prevSlideAlt", "Previous Slide (alternative)",
                    [this](Event *e) { previousSlide(e); }, IvwKey::Left, KeyState::Press)
    , mouseNextSlide_("mouseNextSlide", "Next Slide (Mouse)", [this](Event *e) { nextSlide(e); },
                      MouseButton::Left, MouseState::Press)
    , mousePrevSlide_("mousePrevSlide", "Previous Slide (Mouse)",
                      [this](Event *e) { previousSlide(e); }, MouseButton::Right,
                      MouseState::Press) {

    isReady_.setUpdate([this]() {
        if (!presentationMode_) {
            return allInportsAreReady();
        } else {
            return !fileList_.empty();
        }
    });
    presentationMode_.onChange([this]() { isReady_.update(); });

    outport_.setHandleResizeEvents(false);

    addPort(inport_);
    addPort(outport_);

    addProperty(presentationMode_);

    addProperty(slideIndex_);
    addProperty(imageFileName_);
    addProperty(imageFilePattern_);
    addProperty(findFilesButton_);

    interactions_.addProperty(toggleMode_);
    interactions_.addProperty(nextSlide_);
    interactions_.addProperty(nextSlideAlt_);
    interactions_.addProperty(prevSlide_);
    interactions_.addProperty(prevSlideAlt_);
    interactions_.addProperty(mouseNextSlide_);
    interactions_.addProperty(mousePrevSlide_);
    addProperty(interactions_);

    interactions_.setCollapsed(true);

    validExtensions_ =
        InviwoApplication::getPtr()->getDataReaderFactory()->getExtensionsForType<Layer>();
    imageFilePattern_.addNameFilters(validExtensions_);

    imageFilePattern_.onChange([&]() { onFindFiles(); });
    findFilesButton_.onChange([&]() { onFindFiles(); });

    imageFileName_.setReadOnly(true);
}

void PresentationProcessor::process() {
    if (imageFilePattern_.isModified()) {
        // check all matching files whether they have a supported file extension,
        // i.e. a data reader exists
        fileList_ = imageFilePattern_.getFileList();
        const auto numElems = fileList_.size();
        util::erase_remove_if(fileList_,
                              [this](std::string &file) { return !isValidImageFile(file); });
        if (numElems != fileList_.size()) {
            // number of valid files has changed, need to update properties
            updateProperties();
            return;
        }
    }

    if (slideIndex_.isModified()) {
        updateFileName();
    }

    if (presentationMode_.get()) {
        if (!currentSlide_ || slideIndex_.isModified()) {
            updateSlideImage();
        }
        outport_.setData(currentSlide_);
    } else {
        outport_.setData(inport_.getData());
    }
}

void PresentationProcessor::updateSlideImage() {
    // sanity check for valid index
    const auto index = slideIndex_.get() - 1;
    if ((index < 0) || (index >= static_cast<int>(fileList_.size()))) {
        LogError("Invalid image index. Exceeded number of files.");
        currentSlide_ = nullptr;
        return;
    }

    const std::string currentFileName = fileList_[index];
    const std::string ext = filesystem::getFileExtension(currentFileName);

    auto factory = getNetwork()->getApplication()->getDataReaderFactory();
    auto reader = factory->getReaderForTypeAndExtension<Layer>(ext);

    // there should always be a reader since we asked the reader for valid extensions
    ivwAssert(reader != nullptr, "Could not find reader for \"" << currentFileName << "\"");

    try {
        auto layer = reader->readData(currentFileName);

        currentSlide_ = std::make_shared<Image>(layer);
    } catch (DataReaderException const &e) {
        LogError(e.getMessage());
    }
}

void PresentationProcessor::onFindFiles() {
    // this processor will only be ready if at least one matching file exists
    fileList_ = imageFilePattern_.getFileList();
    if (fileList_.empty() && !imageFilePattern_.getFilePattern().empty()) {
        if (imageFilePattern_.hasOutOfRangeMatches()) {
            LogError("All matching files are outside the specified range (\""
                     << imageFilePattern_.getFilePattern() << "\", "
                     << imageFilePattern_.getMinRange() << " - " << imageFilePattern_.getMaxRange()
                     << ").");
        } else {
            LogError("No images found matching \"" << imageFilePattern_.getFilePattern() << "\" in "
                                                   << imageFilePattern_.getFilePatternPath()
                                                   << ".");
        }
    }
    updateProperties();
}

void PresentationProcessor::updateProperties() {
    isReady_.update();
    currentSlide_ = nullptr;

    slideIndex_.setReadOnly(fileList_.size() <= 1);

    if (fileList_.size() < static_cast<std::size_t>(slideIndex_.get())) {
        slideIndex_.set(1);
    }
    slideIndex_.setMaxValue(std::max(static_cast<int>(fileList_.size()), 1));
    updateFileName();
}

void PresentationProcessor::updateFileName() {
    int index = slideIndex_.get() - 1;
    if ((index < 0) || (static_cast<std::size_t>(index) >= fileList_.size())) {
        imageFileName_.set("<no images found>");
    } else {
        imageFileName_.set(fileList_[index]);
    }
}

bool PresentationProcessor::isValidImageFile(std::string fileName) {
    std::string fileExtension = toLower(filesystem::getFileExtension(fileName));
    return util::contains_if(validExtensions_,
                             [&](const FileExtension &f) { return f.extension_ == fileExtension; });
}

void PresentationProcessor::nextSlide(Event *e) {
    if (presentationMode_.get()) {
        if (slideIndex_.get() < slideIndex_.getMaxValue()) {
            slideIndex_.set(slideIndex_.get() + 1);
        }
        e->markAsUsed();
    }
}

void PresentationProcessor::previousSlide(Event *e) {
    if (presentationMode_.get()) {
        if (slideIndex_.get() > slideIndex_.getMinValue()) {
            slideIndex_.set(slideIndex_.get() - 1);
        }
        e->markAsUsed();
    }
}

}  // namespace inviwo
