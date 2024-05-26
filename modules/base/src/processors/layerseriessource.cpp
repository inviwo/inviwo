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

#include <modules/base/processors/layerseriessource.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/factoryutil.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerSeriesSource::processorInfo_{
    "org.inviwo.LayerSeriesSource",  // Class identifier
    "Layer Series Source",           // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo LayerSeriesSource::getProcessorInfo() const { return processorInfo_; }

LayerSeriesSource::LayerSeriesSource(InviwoApplication* app)
    : Processor{}
    , outport_{"outport", "<description of the generated outport data>"_help}
    , filePattern_("imageFilePattern", "File Pattern", filesystem::getPath(PathType::Images, "/*"),
                   "")
    , findFilesButton_("findFiles", "Update File List")
    , currentIndex_("currentIndex", "Index", 1, 1, 1, 1)
    , fileName_("fileName", "ile Name") {

    addPorts(outport_);
    addProperties(filePattern_, findFilesButton_, currentIndex_, fileName_);

    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() { return !fileList_.empty(); });

    validExtensions_ = util::getDataReaderFactory(app)->getExtensionsForType<Layer>();
    filePattern_.addNameFilters(validExtensions_);

    filePattern_.onChange([&]() {
        onFindFiles();
        isReady_.update();
    });
    findFilesButton_.onChange([&]() {
        onFindFiles();
        isReady_.update();
    });

    fileName_.setReadOnly(true);
}

void LayerSeriesSource::process() {
    if (fileList_.empty()) return;

    if (filePattern_.isModified()) {
        // check all matching files whether they have a supported file extension,
        // i.e. a data reader exists
        fileList_ = filePattern_.getFileList();
        const auto numElems = fileList_.size();
        std::erase_if(fileList_, [this](const auto& file) { return !isValidImageFile(file); });
        if (numElems != fileList_.size()) {
            // number of valid files has changed, need to update properties
            updateProperties();
            return;
        }
    }

    if (currentIndex_.isModified()) {
        updateFileName();
    }

    // sanity check for valid index
    const auto index = currentIndex_.get() - 1;
    if ((index < 0) || (index >= static_cast<int>(fileList_.size()))) {
        LogError("Invalid image index. Exceeded number of files.");
        return;
    }

    const auto fileName = fileList_[index];
    const auto sext = filePattern_.getSelectedExtension();

    auto factory = util::getDataReaderFactory(getInviwoApplication());
    auto reader = factory->getReaderForTypeAndExtension<Layer>(sext, fileName);

    // there should always be a reader since we asked the reader for valid extensions
    IVW_ASSERT(reader != nullptr, "Could not find reader for \"" << fileName << "\"");

    try {
        auto layer = reader->readData(fileName);
        outport_.setData(layer);
    } catch (const DataReaderException& e) {
        LogError(e.getMessage());
    }
}

void LayerSeriesSource::onFindFiles() {
    // this processor will only be ready if at least one matching file exists
    fileList_ = filePattern_.getFileList();
    if (fileList_.empty() && !filePattern_.getFilePattern().empty()) {
        if (filePattern_.hasOutOfRangeMatches()) {
            LogError("All matching files are outside the specified range (\""
                     << filePattern_.getFilePattern() << "\", " << filePattern_.getMinRange()
                     << " - " << filePattern_.getMaxRange() << ").");
        } else {
            LogError("No images found matching \"" << filePattern_.getFilePattern() << "\" in "
                                                   << filePattern_.getFilePatternPath() << ".");
        }
    }
    updateProperties();
}

void LayerSeriesSource::updateProperties() {
    currentIndex_.setReadOnly(fileList_.size() <= 1);

    if (fileList_.size() < static_cast<std::size_t>(currentIndex_.get())) {
        currentIndex_.set(1);
    }
    currentIndex_.setMaxValue(std::max(static_cast<int>(fileList_.size()), 1));
    updateFileName();
    isReady_.update();
}

void LayerSeriesSource::updateFileName() {
    int index = currentIndex_.get() - 1;
    if ((index < 0) || (static_cast<std::size_t>(index) >= fileList_.size())) {
        fileName_.set("<no images found>");
    } else {
        fileName_.set(fileList_[index].string());
    }
}

bool LayerSeriesSource::isValidImageFile(const std::filesystem::path& fileName) {
    return util::contains_if(validExtensions_,
                             [&](const FileExtension& f) { return f.matches(fileName); });
}

}  // namespace inviwo
