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
    , inport1_("inport1")
    , inport2_("inport2") {

	inport1_.setOutportDeterminesSize(true);
    addPort(inport1_);
	inport2_.setOutportDeterminesSize(true);
    addPort(inport2_);

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

	std::stringstream str;
	str << getIdentifier() << ": Image difference: " << diff;
	util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
}

}  // namespace inviwo
