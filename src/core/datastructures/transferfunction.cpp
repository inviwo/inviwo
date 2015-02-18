/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/vectoroperations.h>
#include <math.h>

namespace inviwo {

TransferFunction::TransferFunction(int textureSize)
    : TransferFunctionObservable()
    , maskMin_(0.0f)
    , maskMax_(1.0f)
    , interpolationType_(InterpolationLinear)
    , textureSize_(textureSize)
    , invalidData_(true)
    , data_(new Layer(uvec2(textureSize, 1), DataVec4FLOAT32::get())) {

    // initialize with standard ramp
    addPoint(vec2(0.0f,0.0f), vec4(0.0f,0.0f,0.0f,0.0f));
    addPoint(vec2(1.0f,1.0f), vec4(1.0f,1.0f,1.0f,1.0f));
}

TransferFunction::TransferFunction(const TransferFunction& rhs) 
    : maskMin_(rhs.maskMin_)
    , maskMax_(rhs.maskMax_)
    , interpolationType_(rhs.interpolationType_)
    , textureSize_(rhs.textureSize_)
    , invalidData_(rhs.invalidData_)
    , data_(new Layer(*rhs.data_)) {

    for (int i = 0; i < rhs.getNumPoints(); i++) {
        addPoint(rhs.getPoint(i)->getPos(), rhs.getPoint(i)->getRGBA());
    }
}

TransferFunction& TransferFunction::operator=(const TransferFunction& rhs) {
    if (this != &rhs) {
        delete data_;
        data_ = new Layer(*rhs.data_);
        clearPoints();
        textureSize_ = rhs.textureSize_;
        maskMin_ = rhs.maskMin_;
        maskMax_ = rhs.maskMax_;
        interpolationType_ = rhs.interpolationType_;

        for (int i = 0; i < rhs.getNumPoints(); i++) {
            addPoint(rhs.getPoint(i)->getPos(), rhs.getPoint(i)->getRGBA());
        }
    }
    invalidate();
    return *this;
}

TransferFunction::~TransferFunction() {
    clearPoints();
    delete data_;
}

TransferFunctionDataPoint* TransferFunction::getPoint(int i) const {
    return points_[i];
}

void TransferFunction::addPoint(const vec2& pos) {
    // determine color
    vec4 color = vec4(0.5f, 0.5f, 0.5f, pos.y);
    if (points_.size() > 0) {
        int leftNeighborID = 0;
        int rightNeighborID = 0;

        for (int i = 0; i < static_cast<int>(points_.size()); i++)
            if (points_[i]->getPos().x <= pos.x)
                leftNeighborID = i;
            else if (rightNeighborID == 0 && points_[i]->getPos().x > pos.x)
                rightNeighborID = i;

        vec4 colorL = points_[leftNeighborID]->getRGBA();
        vec4 colorR = points_[rightNeighborID]->getRGBA();
        float a = 0.5f;
        float denom = points_[rightNeighborID]->getPos().x - points_[leftNeighborID]->getPos().x;

        if (denom > 0.0) a = (points_[rightNeighborID]->getPos().x - pos.x) / denom;

        color = vec4(a * colorL.r + (1.0 - a) * colorR.r, a * colorL.g + (1.0 - a) * colorR.g,
                     a * colorL.b + (1.0 - a) * colorR.b, pos.y);
    }

    addPoint(pos, color);
}

void TransferFunction::addPoint(const vec2& pos, const vec4& color) {
    addPoint(new TransferFunctionDataPoint(pos, color));
}

void TransferFunction::addPoint(TransferFunctionDataPoint* dataPoint) {
    dataPoint->addObserver(this);
    TFPoints::iterator pos = std::lower_bound(points_.begin(), points_.end(), dataPoint,
                                              comparePtr<TransferFunctionDataPoint>);
    points_.insert(pos, dataPoint);

    invalidate();
    notifyControlPointAdded(dataPoint);
}

void TransferFunction::removePoint(TransferFunctionDataPoint* dataPoint) {
    TFPoints::iterator it = std::find(points_.begin(), points_.end(), dataPoint);
    if (it != points_.end()) {
        points_.erase(it);
        invalidate();
        notifyControlPointRemoved(dataPoint);
        delete dataPoint;
    }
}

void TransferFunction::clearPoints() {
    invalidate();
    while(points_.size() > 0) {
        TransferFunctionDataPoint* dataPoint = points_.back();
        points_.pop_back();
        notifyControlPointRemoved(dataPoint);
        delete dataPoint;
    }
}

void TransferFunction::onTransferFunctionPointChange(const TransferFunctionDataPoint* p){
    std::stable_sort(points_.begin(), points_.end(), comparePtr<TransferFunctionDataPoint>);
    invalidate();
    notifyControlPointChanged(p);
}

void TransferFunction::calcTransferValues() {
    vec4* dataArray = static_cast<vec4*>(data_->getEditableRepresentation<LayerRAM>()->getData());

    std::stable_sort(points_.begin(), points_.end(), comparePtr<TransferFunctionDataPoint>);

    if (points_.size() == 0) { // in case of 0 points     
        for (int i = 0; i < textureSize_; i++) {
            dataArray[i] = vec4((float)i / (textureSize_ - 1.0), (float)i / (textureSize_ - 1.0),
                                (float)i / (textureSize_ - 1.0), 1.0);
        }
    } else if (points_.size() == 1) {  // in case of 1 point
        for (int i = 0; i < textureSize_; ++i) {
            dataArray[i] = points_[0]->getRGBA();
        }
    } else { // in case of more than 1 points
        int leftX = static_cast<int>(ceil(points_.front()->getPos().x * (textureSize_ - 1)));
        int rightX = static_cast<int>(ceil(points_.back()->getPos().x * (textureSize_ - 1)));

        for (int i = 0; i <= leftX; i++) dataArray[i] = points_.front()->getRGBA();
        for (int i = rightX; i < textureSize_; i++) dataArray[i] = points_.back()->getRGBA();

        // if (interpolationType_==TransferFunction::InterpolationLinear) {
        // linear interpolation
        std::vector<TransferFunctionDataPoint*>::iterator pLeft = points_.begin();
        std::vector<TransferFunctionDataPoint*>::iterator pRight = points_.begin() + 1;

        while (pRight != points_.end()) {
            int n = static_cast<int>(ceil((*pLeft)->getPos().x * (textureSize_ - 1)));

            while (n < ceil((*pRight)->getPos().x * (textureSize_ - 1))) {
                vec4 lrgba = (*pLeft)->getRGBA();
                vec4 rrgba = (*pRight)->getRGBA();
                float lx = (*pLeft)->getPos().x * (textureSize_ - 1);
                float rx = (*pRight)->getPos().x * (textureSize_ - 1);
                float alpha = (n - lx) / (rx - lx);
                dataArray[n] = alpha * rrgba + (1.0f - alpha) * lrgba;
                n++;
            }

            pLeft++;
            pRight++;
        }

        //} else {
        // cubic interpolation
        // TODO: implement cubic interpolation
        //}
    }

    for (int i = 0; i < int(maskMin_ * textureSize_); i++) dataArray[i].a = 0.0;
    for (int i = int(maskMax_ * textureSize_); i < textureSize_; i++) dataArray[i].a = 0.0;

    invalidData_ = false;
}

void TransferFunction::serialize(IvwSerializer& s) const {
    s.serialize("maskMin", maskMin_);
    s.serialize("maskMax", maskMax_);
    s.serialize("dataPoints", points_, "point");
    s.serialize("interpolationType_", static_cast<int>(interpolationType_));
}

void TransferFunction::deserialize(IvwDeserializer& d) {
    d.deserialize("maskMin", maskMin_);
    d.deserialize("maskMax", maskMax_);
    d.deserialize("dataPoints", points_, "point");
    for (TFPoints::iterator it = points_.begin(); it != points_.end(); ++it) {
        (*it)->addObserver(this);
    }
    int type = static_cast<int>(interpolationType_);
    d.deserialize("interpolationType_", type);
    interpolationType_ = static_cast<TransferFunction::InterpolationType>(type);
    
    invalidate();
}

const Layer* TransferFunction::getData() const {
    if (invalidData_) {
        const_cast<TransferFunction*>(this)->calcTransferValues();
    }
    return data_;
}

void TransferFunction::invalidate() {
    invalidData_ = true;
}

float TransferFunction::getMaskMin() const {
    return maskMin_;
}

void TransferFunction::setMaskMin(float maskMin) {
    maskMin_ = maskMin;
    invalidate();
}

float TransferFunction::getMaskMax() const {
    return maskMax_;
}

void TransferFunction::setMaskMax(float maskMax) {
    maskMax_ = maskMax;
    invalidate();
}

void TransferFunction::setInterpolationType(InterpolationType interpolationType) {
    interpolationType_ = interpolationType;
    invalidate();
}

inviwo::TransferFunction::InterpolationType TransferFunction::getInterpolationType() const {
    return interpolationType_;
}

int TransferFunction::getTextureSize() {
    return textureSize_;
}

int TransferFunction::getNumPoints() const {
    return static_cast<int>(points_.size());
}

void TransferFunctionObservable::notifyControlPointAdded(TransferFunctionDataPoint* p) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it)
        static_cast<TransferFunctionObserver*>(*it)->onControlPointAdded(p);
}

void TransferFunctionObservable::notifyControlPointRemoved(TransferFunctionDataPoint* p) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it)
        static_cast<TransferFunctionObserver*>(*it)->onControlPointRemoved(p);
}

void TransferFunctionObservable::notifyControlPointChanged(const TransferFunctionDataPoint* p) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it)
        static_cast<TransferFunctionObserver*>(*it)->onControlPointChanged(p);
}

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs) {
    return lhs.maskMin_ == rhs.maskMin_
        && lhs.maskMax_ == rhs.maskMax_
        && lhs.interpolationType_ == rhs.interpolationType_
        && lhs.points_ == rhs.points_;
}

bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs) {
    return !operator==(lhs, rhs);
}

}