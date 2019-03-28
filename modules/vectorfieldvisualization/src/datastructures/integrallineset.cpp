/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>

namespace inviwo {

IntegralLineSet::IntegralLineSet(mat4 modelMatrix, mat4 worldMatrix)
    : lines_(), modelMatrix_(modelMatrix), worldMatrix_(worldMatrix) {}

IntegralLineSet::~IntegralLineSet() {}

mat4 IntegralLineSet::getModelMatrix() const { return modelMatrix_; }
mat4 IntegralLineSet::getWorldMatrix() const { return worldMatrix_; }

std::vector<IntegralLine>::const_iterator IntegralLineSet::begin() const { return lines_.begin(); }

std::vector<IntegralLine>::iterator IntegralLineSet::begin() { return lines_.begin(); }

std::vector<IntegralLine>::const_iterator IntegralLineSet::end() const { return lines_.end(); }

std::vector<IntegralLine>::iterator IntegralLineSet::end() { return lines_.end(); }

size_t IntegralLineSet::size() const { return lines_.size(); }

IntegralLine& IntegralLineSet::operator[](size_t idx) { return lines_[idx]; }

const IntegralLine& IntegralLineSet::operator[](size_t idx) const { return lines_[idx]; }

IntegralLine& IntegralLineSet::at(size_t idx) { return lines_.at(idx); }

const IntegralLine& IntegralLineSet::at(size_t idx) const { return lines_.at(idx); }

void IntegralLineSet::push_back(const IntegralLine& line, SetIndex updateIndex) {
    if (updateIndex == SetIndex::No) {
        lines_.push_back(line);
    } else {
        push_back(line, lines_.size());
    }
}

void IntegralLineSet::push_back(const IntegralLine& line, size_t idx) {
    IntegralLine copy(line);
    copy.setIndex(idx);
    lines_.push_back(std::move(copy));
}

void IntegralLineSet::push_back(IntegralLine&& line, SetIndex updateIndex) {
    if (updateIndex == SetIndex::Yes) {
        line.setIndex(lines_.size());
    }
    lines_.push_back(line);
}

void IntegralLineSet::push_back(IntegralLine&& line, size_t idx) {
    line.setIndex(idx);
    lines_.push_back(line);
}

}  // namespace inviwo
