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
				//std::cerr << prop << " : " << prop->getDisplayName() << " : " << prop->getPath() << std::endl;
				if(auto* _p = dynamic_cast<IntMinMaxProperty*>(prop); _p != nullptr) {
					auto* p = _p->clone();
					
					addProperty(p);
					//p->setReadOnly(true);
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
	testResults.clear();
}

void Histogram::startTesting(const size_t index) {
	testResults.clear();
	assert(rangesToTest.empty());

	if(index >= props_.size()) {
		currentPropertyIndex = std::nullopt;
		return;
	}
	// TODO: test this property only, if changing it invalidates inport_
	if(app_->getProcessorNetwork()->getPropertiesLinkedTo(props_[index]).empty()) {
		return startTesting(index+1);
	}
	
	currentPropertyIndex = {index};

	auto& currentProperty = *(props_[index]);
	currentProperty.setCurrentStateAsDefault(); // store current state in order to reset it after testing

	// generate ranges to test
	for(int lo = currentProperty.getRangeMin();
		    lo + currentProperty.getMinSeparation() <= currentProperty.getRangeMax();
			lo++) {
		rangesToTest.emplace(lo, currentProperty.getRangeMax());
	}
	for(int hi = currentProperty.getRangeMin() + currentProperty.getMinSeparation();
            hi <= currentProperty.getRangeMax();
			hi++) {
		rangesToTest.emplace(currentProperty.getRangeMin(), hi);
	}

    assert(rangesToTest.size() > 1);

    currentProperty.set(rangesToTest.front());
    rangesToTest.pop();

    // force update, TODO: find better solution
    app_->getProcessorNetwork()->forEachProcessor([](auto proc) {
            proc->invalidate(InvalidationLevel::InvalidOutput);
        });
}

void Histogram::checkTestResults() {
    // maybe TODO: can be done with a segment tree in O(n*log(n))
    for(auto&& [range,count] : testResults) {
        for(auto&& [otherRange,otherCount] : testResults) {
            // otherRange lies fully within range
            if(range.x <= otherRange.x && range.y >= otherRange.y) {
                assert(count >= otherCount);
            }
        }
    }
}

void Histogram::process() {
	auto img = inport_.getData();
	const auto dim = img->getDimensions();

	if(currentPropertyIndex) { // if currently testing
		size_t index = *currentPropertyIndex;
		auto& currentProperty = *props_[index];

		size_t pixelCount = 0;
		auto imgRam = img->getRepresentation<ImageRAM>();
		for(size_t x = 0; x < dim.x; x++) {
			for(size_t y = 0; y < dim.y; y++) {
				const auto col = imgRam->readPixel(size2_t(x,y), LayerType::Depth);
				if(col.x == 1)
					pixelCount++;
			}
		}
		std::cerr << currentProperty.getIdentifier() << " "
                  << currentProperty.get() << " / "
                  << currentProperty.getRange() << " "
                  << rangesToTest.size() << " "
                  << "pixelCount = " << pixelCount << std::endl;

		testResults.emplace_back(dim, pixelCount);
		
		if(!rangesToTest.empty()) {
            currentProperty.set(rangesToTest.front());
            rangesToTest.pop();
            // force update, TODO: find better solution
            app_->getProcessorNetwork()->forEachProcessor([](auto proc) {
                    proc->invalidate(InvalidationLevel::InvalidOutput);
                });
		} else {
			currentPropertyIndex = std::nullopt;
			currentProperty.resetToDefaultState();

            checkTestResults();
			startTesting(index+1);
		}
	}
	
	if(!currentPropertyIndex) {
		outport_.setData(img);
		this->invalidate(InvalidationLevel::InvalidOutput);
	}

}

}  // namespace inviwo
