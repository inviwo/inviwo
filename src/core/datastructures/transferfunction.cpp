/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/util/filesystem.h>

#include <cmath>

namespace inviwo {

TransferFunction::TransferFunction(int textureSize)
    : TransferFunctionObservable()
    , maskMin_(0.0f)
    , maskMax_(1.0f)
    , invalidData_(true)
    , dataRepr_{std::make_shared<LayerRAMPrecision<vec4>>(size2_t(textureSize, 1))}
    , data_(util::make_unique<Layer>(dataRepr_)) {

    // initialize with standard ramp
    addPoint(vec2(0.0f,0.0f), vec4(0.0f,0.0f,0.0f,0.0f));
    addPoint(vec2(1.0f,1.0f), vec4(1.0f,1.0f,1.0f,1.0f));
}

TransferFunction::TransferFunction(const TransferFunction& rhs) 
    : maskMin_(rhs.maskMin_)
    , maskMax_(rhs.maskMax_)
    , invalidData_(rhs.invalidData_)
    , dataRepr_(std::shared_ptr<LayerRAMPrecision<vec4>>(rhs.dataRepr_->clone()))
    , data_(util::make_unique<Layer>(dataRepr_)) {

    for (int i = 0; i < rhs.getNumPoints(); i++) {
        addPoint(rhs.getPoint(i)->getPos(), rhs.getPoint(i)->getRGBA());
    }
}

TransferFunction& TransferFunction::operator=(const TransferFunction& rhs) {
    if (this != &rhs) {
        if (dataRepr_->getDimensions() != rhs.dataRepr_->getDimensions()) {
            dataRepr_ = std::make_shared<LayerRAMPrecision<vec4>>(rhs.dataRepr_->getDimensions());
            data_ = util::make_unique<Layer>(dataRepr_);
        }
        maskMin_ = rhs.maskMin_;
        maskMax_ = rhs.maskMax_;
        for (size_t i = 0; i < std::min(points_.size(), rhs.points_.size()); i++) {
            *points_[i] = *rhs.points_[i];
        }
        for (size_t i = std::min(points_.size(), rhs.points_.size()); i < rhs.points_.size(); i++) {
            addPoint(rhs.getPoint(i)->getPos(), rhs.getPoint(i)->getRGBA());
        }
        while (points_.size() > rhs.points_.size()) {
            auto dp = std::move(points_.back());
            points_.pop_back();
            notifyControlPointRemoved(dp.get());
        }
    }
    invalidate();
    return *this;
}

TransferFunction::~TransferFunction() = default;

const TransferFunctionDataPoint* TransferFunction::getPoint(int i) const {
    return points_[i].get();
}

TransferFunctionDataPoint* TransferFunction::getPoint(int i) {
    return points_[i].get();
}

void TransferFunction::addPoint(const vec2& pos) {
    // determine color
    vec4 color = vec4(0.5f, 0.5f, 0.5f, pos.y);
    if (points_.size() > 0) {
        int leftNeighborID = 0;
        int rightNeighborID = 0;

        for (int i = 0; i < static_cast<int>(points_.size()); i++) {
            if (points_[i]->getPos().x <= pos.x) {
                leftNeighborID = i;
            } else if (rightNeighborID == 0 && points_[i]->getPos().x > pos.x) {
                rightNeighborID = i;
            }
        }

        vec4 colorL = points_[leftNeighborID]->getRGBA();
        vec4 colorR = points_[rightNeighborID]->getRGBA();
        float a = 0.5f;
        float denom = points_[rightNeighborID]->getPos().x - points_[leftNeighborID]->getPos().x;

        if (denom > 0.0f) a = (points_[rightNeighborID]->getPos().x - pos.x) / denom;

        color = vec4(a * colorL.r + (1.0f - a) * colorR.r, a * colorL.g + (1.0f - a) * colorR.g,
                     a * colorL.b + (1.0f - a) * colorR.b, pos.y);
    }

    addPoint(pos, color);
}

void TransferFunction::addPoint(const vec2& pos, const vec4& color) {
    addPoint(util::make_unique<TransferFunctionDataPoint>(pos, color));
}

void TransferFunction::addPoint(std::unique_ptr<TransferFunctionDataPoint> dataPoint) {
    dataPoint->addObserver(this);
    auto it = std::lower_bound(points_.begin(), points_.end(), dataPoint, comparePtr{});
    it = points_.insert(it, std::move(dataPoint));
    invalidate();
    notifyControlPointAdded(it->get());
}

void TransferFunction::removePoint(TransferFunctionDataPoint* dataPoint) {
    auto it = std::find_if(points_.begin(), points_.end(),
                           [&](const auto& p) { return dataPoint == p.get(); });
    if (it != points_.end()) {
        // make sure we call the destuctor after we have removed the point from points_
        auto dp = std::move(*it);
        points_.erase(it);
        notifyControlPointRemoved(dp.get());
        invalidate();
    }
}

void TransferFunction::clearPoints() {
    invalidate();
    while (points_.size() > 0) {
        auto dp = std::move(points_.back());
        points_.pop_back();
        notifyControlPointRemoved(dp.get());
    }
}

void TransferFunction::onTransferFunctionPointChange(const TransferFunctionDataPoint* p){
    std::stable_sort(points_.begin(), points_.end(), comparePtr{});
    invalidate();
    notifyControlPointChanged(p);
}

void TransferFunction::calcTransferValues() const {
    ivwAssert(std::is_sorted(points_.begin(), points_.end(), comparePtr{}), "Should be sorted");

    // We assume the the points a sorted here.
    auto dataArray = dataRepr_->getDataTyped();
    const auto size = dataRepr_->getDimensions().x;

    if (points_.size() == 0) {  // in case of 0 points
        for (size_t i = 0; i < size; i++) {
            dataArray[i] =
                vec4((float)i / (size - 1), (float)i / (size - 1), (float)i / (size - 1), 1.0);
        }
    } else if (points_.size() == 1) {  // in case of 1 point
        for (size_t i = 0; i < size; ++i) {
            dataArray[i] = points_[0]->getRGBA();
        }
    } else {  // in case of more than 1 points
        size_t leftX = static_cast<size_t>(ceil(points_.front()->getPos().x * (size - 1)));
        size_t rightX = static_cast<size_t>(ceil(points_.back()->getPos().x * (size - 1)));

        for (size_t i = 0; i <= leftX; i++) dataArray[i] = points_.front()->getRGBA();
        for (size_t i = rightX; i < size; i++) dataArray[i] = points_.back()->getRGBA();

        auto pLeft = points_.begin();
        auto pRight = points_.begin() + 1;

        while (pRight != points_.end()) {
            size_t n = static_cast<size_t>(ceil((*pLeft)->getPos().x * (size - 1)));

            while (n < static_cast<size_t>(ceil((*pRight)->getPos().x * (size - 1)))) {
                vec4 lrgba = (*pLeft)->getRGBA();
                vec4 rrgba = (*pRight)->getRGBA();
                float lx = (*pLeft)->getPos().x * (size - 1);
                float rx = (*pRight)->getPos().x * (size - 1);
                float alpha = (n - lx) / (rx - lx);
                dataArray[n] = alpha * rrgba + (1.0f - alpha) * lrgba;
                n++;
            }

            pLeft++;
            pRight++;
        }
    }

    for (size_t i = 0; i < size_t(maskMin_ * size); i++) dataArray[i].a = 0.0;
    for (size_t i = size_t(maskMax_ * size); i < size; i++) dataArray[i].a = 0.0;

    data_->invalidateAllOther(dataRepr_.get());

    invalidData_ = false;
}

void TransferFunction::serialize(Serializer& s) const {
    s.serialize("maskMin", maskMin_);
    s.serialize("maskMax", maskMax_);
    s.serialize("dataPoints", points_, "point");
}

void TransferFunction::deserialize(Deserializer& d) {
    d.deserialize("maskMin", maskMin_);
    d.deserialize("maskMax", maskMax_);

    util::IndexedDeserializer<std::unique_ptr<TransferFunctionDataPoint>>("dataPoints", "point")
        .onNew([&](std::unique_ptr<TransferFunctionDataPoint>& point) {
            auto ptr = point.get();
            std::stable_sort(points_.begin(), points_.end(), comparePtr{});
            notifyControlPointAdded(ptr);
        })
        .onRemove([&](std::unique_ptr<TransferFunctionDataPoint>& point) {
            notifyControlPointRemoved(point.get());
        })(d, points_);

    invalidate();
}

vec4 TransferFunction::sample(double v) const {
    return sample(static_cast<float>(v));
}

vec4 TransferFunction::sample(float v) const {
    if (v <= 0) {
        return points_.front()->getRGBA();
    } else if (v >= 1) {
        return points_.back()->getRGBA();
    }

    auto it = std::upper_bound(points_.begin(), points_.end(), v,
                               [](float val, const auto& p) { return val < p->getPos().x; });

    if (it == points_.begin()) {
        return points_.front()->getRGBA();
    } else if (it == points_.end()) {
        return points_.back()->getRGBA();
    }

    auto next = it--;
    float x = (v - (*it)->getPos().x) / ((*next)->getPos().x - (*it)->getPos().x);
    return Interpolation<vec4, float>::linear((*it)->getRGBA(), (*next)->getRGBA(), x);
}

const Layer* TransferFunction::getData() const {
    if (invalidData_) calcTransferValues();
    return data_.get();
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

int TransferFunction::getTextureSize() {
    return dataRepr_->getDimensions().x;
}

int TransferFunction::getNumPoints() const {
    return static_cast<int>(points_.size());
}

void TransferFunctionObservable::notifyControlPointAdded(TransferFunctionDataPoint* p) {
    forEachObserver([&](TransferFunctionObserver* o){o->onControlPointAdded(p);});
}

void TransferFunctionObservable::notifyControlPointRemoved(TransferFunctionDataPoint* p) {
    forEachObserver([&](TransferFunctionObserver* o){o->onControlPointRemoved(p);});
}

void TransferFunctionObservable::notifyControlPointChanged(const TransferFunctionDataPoint* p) {
    forEachObserver([&](TransferFunctionObserver* o){o->onControlPointChanged(p);});
}

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs) {
    return lhs.maskMin_ == rhs.maskMin_
        && lhs.maskMax_ == rhs.maskMax_
        && lhs.points_ == rhs.points_;
}

bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs) {
    return !operator==(lhs, rhs);
}

void TransferFunction::save(const std::string& filename, const FileExtension& ext) const {
    std::string extension = toLower(filesystem::getFileExtension(filename));
    
    if (ext.extension_ == "itf" || (ext.empty() && extension == "itf")) {
        Serializer serializer(filename);
        serialize(serializer);
        serializer.writeFile();
    } else {
        if (invalidData_) calcTransferValues();
        
        // Convert layer to UINT8
        auto uint8DataRepr =
            std::make_shared<LayerRAMPrecision<glm::u8vec4>>(dataRepr_->getDimensions());
        auto unit8Data = util::make_unique<Layer>(uint8DataRepr);

        const auto size = glm::compMul(dataRepr_->getDimensions());
        const auto sptr = dataRepr_->getDataTyped();
        const auto dptr = uint8DataRepr->getDataTyped();

        for (size_t i = 0; i < size; ++i) {
            dptr[i] =
                static_cast<glm::u8vec4>(glm::clamp(sptr[i] * 255.0f, vec4(0.0f), vec4(255.0f)));
        }

        auto factory = InviwoApplication::getPtr()->getDataWriterFactory();
        auto writer = factory->getWriterForTypeAndExtension<Layer>(ext);
        if (!writer) {
            writer = factory->getWriterForTypeAndExtension<Layer>(extension);
        }
        if (!writer) {
            throw DataWriterException("Data writer not found for requested format", IvwContext);
        }
        writer->setOverwrite(true);
        writer->writeData(unit8Data.get(), filename);
    }
}

void TransferFunction::load(const std::string& filename, const FileExtension& ext) {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "itf" || (ext.empty() && extension == "itf")) {
        Deserializer deserializer(filename);
        deserialize(deserializer);
    } else {
        auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
        auto reader = factory->getReaderForTypeAndExtension<Layer>(ext);
        if (!reader) {
            reader = factory->getReaderForTypeAndExtension<Layer>(extension);
        }
        if (!reader) {
            throw DataReaderException("Data reader not found for requested format", IvwContext);
        }
        auto layer = reader->readData(filename);

        clearPoints();

        layer->getRepresentation<LayerRAM>()->dispatch<void>([this](auto lrprecision) {
            using LayerType = util::PrecsionType<decltype(lrprecision)>;
            using ValueType = util::PrecsionValueType<decltype(lrprecision)>;
        
            auto data = lrprecision->getDataTyped();
            const auto size = lrprecision->getDimensions().x;

            std::vector<vec4> points;
            for (size_t i = 0; i < size; ++i) {
                auto rgba = util::glm_convert_normalized<vec4>(data[i]);
                points.push_back(rgba);
            }
            if (points.empty()) return;

            std::vector<std::pair<std::ptrdiff_t, vec4>> uniquePoints;
            uniquePoints.emplace_back(0, points.front());
            for (auto p = points.begin(), c = std::next(points.begin()),
                      n = std::next(points.begin(), 2);
                 n != points.end(); ++p, ++c, ++n) {
                if (!glm::all(glm::lessThan(glm::abs(*c - 0.5f * (*p + *n)), vec4(1.0f/255.0f)))) {
                    uniquePoints.emplace_back(std::distance(points.begin(), c), *c);
                }
            }
            uniquePoints.emplace_back(std::ptrdiff_t(size - 1), points.back());

            for (const auto &p : uniquePoints ) {
                this->addPoint(vec2(float(p.first)/(size-1), p.second.a), p.second);
            }
        });
    }
}

}  // namespace
