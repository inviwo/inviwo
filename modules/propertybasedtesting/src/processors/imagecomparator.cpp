/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/processors/imagecomparator.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/util/fileextension.h>
#include <filesystem>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageComparator::processorInfo_{
    "org.inviwo.ImageComparator",      // Class identifier
    "Image Comparator",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo ImageComparator::getProcessorInfo() const { return processorInfo_; }

ImageComparator::ImageComparator()
    : Processor()
    , outportDeterminesSize_{"outportDeterminesSize", "Let Outport Determine Size", false}
    , imageSize_{"imageSize",   "Image Size",        size2_t(1024, 1024),
                 size2_t(1, 1), size2_t(4096, 4096), size2_t(1, 1)}
    , inport1_("inport1")
    , inport2_("inport2")
    , maxDeviation_("maxDeviation", "Maximum deviation", 0, 0, std::numeric_limits<float>::max(), 1)
    , comparisonType_("comparisonType", "Comparison Type (dummy)",
                     {{"diff", "Sum of ARGB differences", ComparisonType::Diff},
                      {"perceptual", "Perceptual Difference", ComparisonType::Perceptual},
                      {"global", "Global Difference", ComparisonType::Global},
                      {"local", "Local Difference", ComparisonType::Local}},
                     0, InvalidationLevel::InvalidResources)
    , tempDir{std::filesystem::temp_directory_path() / ("inviwo_imagecomp_" + std::to_string(rand()))}
    , prevSize1_{0}
    , prevSize2_{0} {

	imageSize_.visibilityDependsOn(outportDeterminesSize_,
                                [](const auto& p) -> bool { return !p; });

    outportDeterminesSize_.onChange([this] {
        this->inport1_.setOutportDeterminesSize(outportDeterminesSize_);
        this->inport2_.setOutportDeterminesSize(outportDeterminesSize_);
        sendResizeEvent();
    });

    this->inport1_.setOutportDeterminesSize(outportDeterminesSize_);
    this->inport2_.setOutportDeterminesSize(outportDeterminesSize_);

    imageSize_.onChange([this]() { sendResizeEvent(); });
    addProperties(outportDeterminesSize_, imageSize_);

	addPort(inport1_);
	addPort(inport2_);
	maxDeviation_.setSemantics(PropertySemantics::Text);
	addProperty(maxDeviation_);
	addProperty(comparisonType_);

	if (std::filesystem::create_directory(tempDir)) {
		util::log(IVW_CONTEXT, std::string("Using ") + std::string(tempDir) + std::string(" to store broken images."), LogLevel::Info, LogAudience::User);
	}

	isReady_.setUpdate([&]() {
			if(!allInportsAreReady())
				return false;
			auto img1 = inport1_.getData();
			auto img2 = inport2_.getData();

			const auto dim1 = img1->getDimensions();
			const auto dim2 = img2->getDimensions();

			if(dim1 != dim2) {
				std::stringstream str;
				str << getIdentifier() << ": Images do not have same dimensions: " << dim1 << " != " << dim2;
				util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
				return false;
			}
			return true;
		});
}

void ImageComparator::setNetwork(ProcessorNetwork* network) {
    if (network) network->addObserver(this);

    Processor::setNetwork(network);
}

void ImageComparator::sendResizeEvent() {
    const size2_t newSize1 = outportDeterminesSize_ ? size2_t{0} : *imageSize_;
    const size2_t newSize2 = outportDeterminesSize_ ? size2_t{0} : *imageSize_;

    if (newSize1 != prevSize1_) {
        ResizeEvent event{newSize1, prevSize1_};
        this->inport1_.propagateEvent(&event, nullptr);
        prevSize1_ = newSize1;
    }

    if (newSize2 != prevSize2_) {
        ResizeEvent event{newSize2, prevSize2_};
        this->inport2_.propagateEvent(&event, nullptr);
        prevSize2_ = newSize2;
    }
}

void ImageComparator::process() {
	auto img1 = inport1_.getData();
	auto img2 = inport2_.getData();

	const auto dim1 = img1->getDimensions();
	const auto dim2 = img2->getDimensions();

	assert(dim1 == dim2);
	auto imgRam1 = img1->getRepresentation<ImageRAM>();
	auto imgRam2 = img2->getRepresentation<ImageRAM>();

	auto colorLayerRAM1 = imgRam1->getColorLayerRAM();
	auto colorLayerRAM2 = imgRam2->getColorLayerRAM();

	double diff = 0;
	for(size_t x = 0; x < dim1.x; x++) {
		for(size_t y = 0; y < dim1.y; y++) {
			const auto col1 = colorLayerRAM1->getAsDVec4(size2_t(x,y));
			const auto col2 = colorLayerRAM2->getAsDVec4(size2_t(x,y));
			diff += glm::length(col1 - col2);
		}
	}

	if(diff > maxDeviation_.get()) {
		std::stringstream str;
		str << getIdentifier() << ": Image difference: " << diff;
		util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);

		const auto bad_uid = std::to_string(rand());
		const auto img1Path = tempDir / (std::string("img1_") + std::string(bad_uid) + std::string(".png"));
		const auto img2Path = tempDir / (std::string("img2_") + std::string(bad_uid) + std::string(".png"));
		inviwo::util::saveLayer(*img1->getColorLayer(), img1Path.string(), inviwo::FileExtension::createFileExtensionFromString(std::string("png")));
		inviwo::util::saveLayer(*img2->getColorLayer(), img2Path.string(), inviwo::FileExtension::createFileExtensionFromString(std::string("png")));
	}
}

void ImageComparator::onProcessorNetworkDidAddConnection(const PortConnection& con) {
    const auto successors = util::getSuccessors(con.getInport()->getProcessor());
    if (util::contains(successors, this)) {
        sendResizeEvent();
    }
}

void ImageComparator::onProcessorNetworkDidRemoveConnection(const PortConnection& con) {
    const auto successors = util::getSuccessors(con.getInport()->getProcessor());
    if (util::contains(successors, this)) {
        sendResizeEvent();
    }
}

}  // namespace inviwo
