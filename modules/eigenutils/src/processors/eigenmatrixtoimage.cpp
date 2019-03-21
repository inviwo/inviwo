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

#include <modules/eigenutils/processors/eigenmatrixtoimage.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo EigenMatrixToImage::processorInfo_{
    "org.inviwo.EigenMatrixToImage",  // Class identifier
    "Matrix To Image",                // Display name
    "Eigen",                          // Category
    CodeState::Experimental,          // Code state
    "Eigen",                          // Tags
};
const ProcessorInfo EigenMatrixToImage::getProcessorInfo() const { return processorInfo_; }

EigenMatrixToImage::EigenMatrixToImage()
    : Processor()
    , matrix_("matrix")
    , image_("image")
    , flipY_("flipy", "Flip Y-axis", true)
    , usePortSize_("usePortSize", "Use port size (when Matrix is larger)" , true)
{

    addPort(matrix_);
    addPort(image_);

    addProperty(flipY_);
    addProperty(usePortSize_);

    usePortSize_.onChange([this](){
        
    });
}

void EigenMatrixToImage::process() {
    auto m = matrix_.getData();

    auto portSize = image_.getDimensions();

    if(usePortSize_.get() && ( portSize.x < m->cols() || portSize.y < m->rows() ) ){
        auto numRows = std::min<Eigen::Index>(portSize.y , m->rows());
        auto numCols = std::min<Eigen::Index>(portSize.x , m->cols());
        auto newM = util::downsample(*m, numRows,numCols);
        image_.setData(util::eigenMatToImage( newM, flipY_.get()));
    }else{
        image_.setData(util::eigenMatToImage(*m, flipY_.get()));
    }
}

}  // namespace inviwo
