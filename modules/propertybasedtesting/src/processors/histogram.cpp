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
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOr
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/propertybasedtesting/processors/histogram.h>

#include <inviwo/core/datastructures/image/imageram.h>

#include <iostream>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Histogram::processorInfo_{
    "org.inviwo.Histogram",      // Class identifier
    "Histogram",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo Histogram::getProcessorInfo() const { return processorInfo_; }

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
	out << "{";
	for(size_t i = 0; i < v.size(); i++) {
		if(i > 0) out << ", ";
		out << v[i];
	}
	return out << "}";
}

Histogram::Histogram(InviwoApplication* app)
    : Processor()
	, app_(app)
	, inport_("imageInport")
    , outport_("imageOutport")
	, startButton_("startButton", "Start")
	, collectButton_("collectButton", "Collect Properties") {

	std::cerr << "Histogram::Histogram(app = " << app << ")" << std::endl;

	startButton_.onChange([this]() { 
			std::cout << "startButton_.onChange()" << std::endl;
			startTesting(0);
			std::cerr << "startButton_.onChange() End" << std::endl;
		} );

	collectButton_.onChange([this]() {
			// remove previously collected properties
			for(auto& property : props_)
				removeProperty(property);
			props_.clear();

			std::set<Property*> properties;
			for(Processor const* const proc : app_->getProcessorNetwork()->getProcessors()) {
				if(proc == this)
					continue;
				const auto& procProperties = proc->getProperties();
				properties.insert(procProperties.begin(), procProperties.end());
			}

			for(Property* prop : properties) {
				std::cerr << prop << " : " << prop->getDisplayName() << " : " << prop->getPath() << std::endl;
				
				if(auto* _p = dynamic_cast<IntMinMaxProperty*>(prop); _p != nullptr) {
					auto* p = _p->clone();
					
					addProperty(p);
					p->setReadOnly(true);
					props_.emplace_back(p);

					app_->getProcessorNetwork()->addLink(p, _p);
				}
			}
		} );

	addPort(inport_);
    addPort(outport_);
	addProperty(startButton_);
	addProperty(collectButton_);

	currentPropertyIndex = std::nullopt;
	lastPixelCount = std::nullopt;
}

void Histogram::startTesting(const size_t index) {
	lastPixelCount = std::nullopt;

	if(index >= props_.size()) {
		currentPropertyIndex = std::nullopt;
		return;
	}
	
	currentPropertyIndex = {index};

	auto& currentProperty = *(props_[index]);
	currentProperty.setStart( currentProperty.getRangeMin() );
	currentProperty.setCurrentStateAsDefault();
}


void Histogram::process() {
	// std::cerr << "Histogram::process()" << std::endl;

	auto img = inport_.getData();
	const auto dim = img->getDimensions();
	
	if(currentPropertyIndex) { // count number of background pixels 
		size_t pixelCount = 0;
		auto imgRam = img->getRepresentation<ImageRAM>();
		for(size_t x = 0; x < dim.x; x++) {
			for(size_t y = 0; y < dim.y; y++) {
				const auto col = imgRam->readPixel(size2_t(x,y), LayerType::Depth);
				if(col.x == 1)
					pixelCount++;
			}
		}
		std::cerr << "pixelCount = " << pixelCount << std::endl;

		if(lastPixelCount) { // check change
			std::cerr << "lastPixelCount = " << *lastPixelCount << std::endl;
		}
		lastPixelCount = pixelCount;
	}

	if(currentPropertyIndex) {
		size_t index = *currentPropertyIndex;
		auto& currentProperty = *props_[index];
		
		if(currentProperty.getStart() + currentProperty.getMinSeparation() < currentProperty.getEnd()) {
			currentProperty.setStart(currentProperty.getStart() + currentProperty.getIncrement());
		} else {
			currentPropertyIndex = std::nullopt;

			currentProperty.resetToDefaultState();
			startTesting(++index);
		}
	}
	
	if(!currentPropertyIndex) {
		outport_.setData(img);
		this->invalidate(InvalidationLevel::InvalidOutput);
	}

}

}  // namespace inviwo
