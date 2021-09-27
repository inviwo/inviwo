/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/contourtree/processors/contourtree.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ContourTree::processorInfo_{
    "org.inviwo.ContourTree",      // Class identifier
    "Contour Tree",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo ContourTree::getProcessorInfo() const { return processorInfo_; }

ContourTree::ContourTree()
    : Processor()
    , treeOutport_("countour_tree")
    , fieldOutport_("scalar_field")
    , segmentationOutport_("segmentation")
    , inputFile_("input_file", "Input RAW file", "")
    , gridSize_("grid_size", "Grid size")
    , simplification_("simplification", "Simplification Threshold", 0.0f, 0.0f, 1.0f) {

    addPort(treeOutport_);
    addPort(fieldOutport_);
    addPort(segmentationOutport_);
    addProperty(inputFile_);
    addProperty(gridSize_);
    addProperty(simplification_);
    
    inputFile_.addNameFilter("raw");
}

void ContourTree::process() {
    size3_t dims = gridSize_.get();
    size_t dimX = dims.x;
    size_t dimY = dims.y;
    size_t dimZ = dims.z;
    
    //std::cout << "Dimensions: " << dimX << ", " << dimY << ", " << dimZ << std::endl;
    
    contourtree::Grid3D<float> grid(dimX, dimY, dimZ);
    
    std::string filePath = inputFile_.get();
    if (!filesystem::fileExists(filePath)) {
        throw Exception("Error could not find input file: " + filePath, IVW_CONTEXT);
    }
    grid.loadGrid(filePath);
    // outport_.setData(myImage);
}

}  // namespace inviwo
