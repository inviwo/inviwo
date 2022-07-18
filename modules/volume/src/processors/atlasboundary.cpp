/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/processors/atlasboundary.h>
#include <inviwo/volume/algorithm/volumemap.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AtlasBoundary::processorInfo_{
    "org.inviwo.atlasboundary",  // Class identifier
    "Atlas Boundary",            // Display name
    "Volume Operation",          // Category
    CodeState::Experimental,     // Code state
    "GL, Volume, Atlas",         // Tags
};
const ProcessorInfo AtlasBoundary::getProcessorInfo() const { return processorInfo_; }

AtlasBoundary::AtlasBoundary()
    : Processor(), volumeInport_("volume"), brushing_("brushing"), outport_("outport") {
    addPort(volumeInport_);
    addPort(brushing_);
    addPort(outport_);
}

void AtlasBoundary::process() {
    if (brushing_.isChanged() || volumeInport_.isChanged() || !volume_) {
        // Get B&L indices
        auto selected = brushing_.getSelectedIndices();
        auto filtered = brushing_.getFilteredIndices();
        auto highlighted = brushing_.getHighlightedIndices();
        highlighted -= filtered;
        selected -= highlighted;
        selected -= filtered;
        std::vector<int> selectedDestination(selected.toVector().size(), 1);
        std::vector<int> filteredDestination(filtered.toVector().size(), 2);
        std::vector<int> highlightedDestination(highlighted.toVector().size(), 3);

        // Append to B&L to vectors
        sourceIndices_.clear();
        destinationIndices_.clear();
        sourceIndices_.insert(sourceIndices_.end(), selected.begin(), selected.end());
        sourceIndices_.insert(sourceIndices_.end(), filtered.begin(), filtered.end());
        sourceIndices_.insert(sourceIndices_.end(), highlighted.begin(), highlighted.end());
        destinationIndices_.insert(destinationIndices_.end(), selectedDestination.begin(),
                                  selectedDestination.end());
        destinationIndices_.insert(destinationIndices_.end(), filteredDestination.begin(),
                                  filteredDestination.end());
        destinationIndices_.insert(destinationIndices_.end(), highlightedDestination.begin(),
                                  highlightedDestination.end());

        if (sourceIndices_.size() == 0) {
            sourceIndices_.push_back(0);
            destinationIndices_.push_back(0);
        }

        // Remap using vectors and cloned volume
        auto volume = volumeInport_.getData();
        volume_ = std::shared_ptr<Volume>(volume->clone());
        util::remap(*volume_, sourceIndices_, destinationIndices_, 0, true);
        // Set volume properties
        volume_->setInterpolation(InterpolationType::Nearest);
        volume_->setSwizzleMask(swizzlemasks::luminance);
        volume_->dataMap_.dataRange = dvec2{0.0, 3.0};
        volume_->dataMap_.valueRange = volume_->dataMap_.dataRange;
    }
    outport_.setData(volume_);
}

}  // namespace inviwo
