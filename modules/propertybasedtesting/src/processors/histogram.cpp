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

Histogram::Histogram(InviwoApplication* app)
    : Processor()
	, app_(app)
	, inport_("imageInport")
    , outport_("imageOutport")
	, startButton_("startButton", "Start")
	, collectButton_("collectButton", "Collect Properties") {

	std::cerr << "Histogram::Histogram(app = " << app << ")" << std::endl;

	startButton_.onChange([this]() { 
			std::cerr << "startButton_.onChange()" << std::endl;
			startTesting(0);
			std::cerr << "startButton_.onChange() End" << std::endl;
		} );

	collectButton_.onChange([this]() {
            // Only necessary because there is no actual serializing yet
            props_.clear();
            for(Property* _property : getProperties()) {
                if(IntMinMaxProperty* property = dynamic_cast<IntMinMaxProperty*>(_property); property != nullptr) {
                    props_.emplace_back(property);
                }
            }

            // remove previously collected properties
            for(auto& property : props_)
				removeProperty(property);
			props_.clear();

			std::set<Property*> properties;

            std::unordered_set<const Processor*> visited;
            std::queue<const Processor*> q;
            q.emplace(this);
            visited.emplace(this);

            // collect all properties of the processors feeding directly or
            // indirectly into this
            while(!q.empty()) {
                auto processor = q.front();
                q.pop();

                std::cerr << "visiting processor " << processor << " "
                          << processor->getIdentifier() << std::endl;
                
                if(processor != this) {
                    const auto& processorProperties = processor->getProperties();
                    properties.insert(processorProperties.begin(), processorProperties.end());
                }

                for(const Inport* inport : processor->getInports()) {
                    for(const Outport* outport : inport->getConnectedOutports()) {
                        if(const Processor* proc = outport->getProcessor(); visited.insert(proc).second) {
                            q.emplace(proc);
                        }
                    }
                }
            }

            std::unordered_set<std::string> usedIdentifiers;
            // TODO: only insert minimal set S of properties such that all properties
            // are either in S or directly or indirectly set by a property in S
			for(Property* prop : properties) {
				if(auto* _p = dynamic_cast<IntMinMaxProperty*>(prop); _p != nullptr) {
					auto* p = _p->clone();
                    if(!usedIdentifiers.insert(p->getIdentifier()).second) {
                        size_t num = 0;
                        do {
                            p->setIdentifier(p->getIdentifier() + "_" + std::to_string(num));
                        } while(!usedIdentifiers.insert(p->getIdentifier()).second);
                    }
					
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

template<typename A, typename B>
std::ostream& operator<<(std::ostream& out, const std::pair<A,B>& v) {
    return out << "[" << v.first << ", " << v.second << "]";
}
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
    out << "{";
    for(size_t i = 0; i < v.size(); i++) {
        if(i > 0) out << ", ";
        out << v[i];
    }
    return out << "}";
}

void Histogram::checkTestResults() {
    std::cerr << "checking " << testResults.size() << " test results ..." << std::flush;
    std::vector< std::pair<TestResult, TestResult> > errors;
    // maybe TODO: can be done with a segment tree in O(n*log(n))
    for(const auto& [range,count] : testResults) {
        for(const auto& [otherRange,otherCount] : testResults) {
            // otherRange lies fully within range
            if(range.x <= otherRange.x && range.y >= otherRange.y &&
                    count > otherCount) {
                errors.emplace_back( TestResult(range,count), TestResult(otherRange,otherCount) );
            }
        }
    }
    std::cerr << " done with " << errors.size() << " errors" << std::endl;
    if(!errors.empty()) {
        
        std::stringstream str;
        str << "Found " << errors.size() << " errors while testing: ";
        errors.resize(std::min(size_t(5), errors.size())); // print at most 5 examples
        str << errors << std::endl;
        util::log(IVW_CONTEXT, str.str(), LogLevel::Error, LogAudience::User);
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

		testResults.emplace_back(currentProperty.get(), pixelCount);
		
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
