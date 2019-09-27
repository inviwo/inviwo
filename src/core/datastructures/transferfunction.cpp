/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/zip.h>

#include <cmath>

namespace inviwo {

TransferFunction::TransferFunction(size_t textureSize)
    : TransferFunction({}, TFPrimitiveSetType::Relative, textureSize) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values, size_t textureSize)
    : TransferFunction(values, TFPrimitiveSetType::Relative, textureSize) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values,
                                   TFPrimitiveSetType type, size_t textureSize)
    : TFPrimitiveSet(values, type)
    , maskMin_(0.0)
    , maskMax_(1.0)
    , invalidData_(true)
    , dataRepr_{std::make_shared<LayerRAMPrecision<vec4>>(size2_t(textureSize, 1))}
    , data_(std::make_unique<Layer>(dataRepr_)) {
    clearMask();
}

TransferFunction::TransferFunction(const TransferFunction& rhs)
    : TFPrimitiveSet(rhs)
    , maskMin_(rhs.maskMin_)
    , maskMax_(rhs.maskMax_)
    , invalidData_(true)
    , dataRepr_(std::shared_ptr<LayerRAMPrecision<vec4>>(rhs.dataRepr_->clone()))
    , data_(std::make_unique<Layer>(dataRepr_)) {}

TransferFunction& TransferFunction::operator=(const TransferFunction& rhs) {
    if (this != &rhs) {
        if (dataRepr_->getDimensions() != rhs.dataRepr_->getDimensions()) {
            dataRepr_ = std::make_shared<LayerRAMPrecision<vec4>>(rhs.dataRepr_->getDimensions());
            data_ = std::make_unique<Layer>(dataRepr_);
        }
        maskMin_ = rhs.maskMin_;
        maskMax_ = rhs.maskMax_;
        invalidData_ = rhs.invalidData_;

        TFPrimitiveSet::operator=(rhs);
    }
    return *this;
}

TransferFunction::~TransferFunction() = default;

const Layer* TransferFunction::getData() const {
    if (invalidData_) calcTransferValues();
    return data_.get();
}

size_t TransferFunction::getTextureSize() const { return dataRepr_->getDimensions().x; }

void TransferFunction::setMaskMin(double maskMin) {
    maskMin_ = maskMin;
    invalidate();
}

double TransferFunction::getMaskMin() const { return maskMin_; }

void TransferFunction::setMaskMax(double maskMax) {
    maskMax_ = maskMax;
    invalidate();
}

double TransferFunction::getMaskMax() const { return maskMax_; }

void TransferFunction::clearMask() {
    maskMin_ =
        (getType() == TFPrimitiveSetType::Relative) ? 0.0 : std::numeric_limits<double>::lowest();
    maskMax_ =
        (getType() == TFPrimitiveSetType::Relative) ? 1.0 : std::numeric_limits<double>::max();
    invalidate();
}

void TransferFunction::invalidate() { invalidData_ = true; }

void TransferFunction::serialize(Serializer& s) const {
    s.serialize("maskMin", maskMin_);
    s.serialize("maskMax", maskMax_);
    TFPrimitiveSet::serialize(s);
}

void TransferFunction::deserialize(Deserializer& d) {
    d.deserialize("maskMin", maskMin_);
    d.deserialize("maskMax", maskMax_);

    TFPrimitiveSet::deserialize(d);
}

vec4 TransferFunction::sample(double v) const { return interpolateColor(v); }

vec4 TransferFunction::sample(float v) const { return interpolateColor(v); }

std::vector<FileExtension> TransferFunction::getSupportedExtensions() const {
    return {{"itf", "Inviwo Transfer Function"}, {"png", "Transfer Function Image"}};
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
        auto unit8Data = std::make_unique<Layer>(uint8DataRepr);

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
            throw DataWriterException("Data writer not found for requested format", IVW_CONTEXT);
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
            throw DataReaderException("Data reader not found for requested format", IVW_CONTEXT);
        }
        const auto layer = reader->readData(filename);

        clear();

        layer->getRepresentation<LayerRAM>()->dispatch<void>([this](auto lrprecision) {
            auto data = lrprecision->getDataTyped();
            const auto size = lrprecision->getDimensions().x;

            const auto points = [&]() {
                std::vector<TFPrimitiveData> tmp;
                for (size_t i = 0; i < size; ++i) {
                    tmp.push_back({static_cast<double>(i) / (size - 1),
                                   util::glm_convert_normalized<vec4>(data[i])});
                }

                if (std::all_of(tmp.cbegin(), tmp.cend(),
                                [](const TFPrimitiveData& p) { return p.color.a == 0.0f; })) {
                    std::for_each(tmp.begin(), tmp.end(),
                                  [](TFPrimitiveData& p) { return p.color.a = 1.0f; });
                }
                return tmp;
            }();

            const auto simplified = simplify(points, 0.01);
            this->add(simplified);
        });
    }
}

std::vector<TFPrimitiveData> TransferFunction::simplify(const std::vector<TFPrimitiveData>& points,
                                                        double delta) {
    if (points.size() < 3) return points;
    std::vector<TFPrimitiveData> simple{points};

    // Calculate the error resulting from using a linear interpolation between the prev and next
    // point instead of including the current one
    const auto error = [&](size_t i) {
        const auto& prev = simple[i - 1];
        const auto& curr = simple[i];
        const auto& next = simple[i + 1];

        const double x = (curr.pos - prev.pos) / (next.pos - prev.pos);
        return glm::compMax(glm::abs(glm::mix(prev.color, next.color, x) - curr.color));
    };

    // Find the point which will result in the smallest error when removed.
    const auto nextToRemove = [&]() {
        const auto index = util::make_sequence<size_t>(1, simple.size() - 1, 1);
        return *std::min_element(index.begin(), index.end(),
                                 [&](size_t a, size_t b) { return error(a) < error(b); });
    };

    // Iteratively remove the point with the smallest error until the error gets larger then delta
    // or only 2 points are left
    auto toRemove = nextToRemove();
    while (error(toRemove) < delta && simple.size() > 2) {
        simple.erase(simple.begin() + toRemove);
        toRemove = nextToRemove();
    }

    return simple;
}

void TransferFunction::calcTransferValues() const {
    ivwAssert(std::is_sorted(sorted_.begin(), sorted_.end(), comparePtr{}), "Should be sorted");

    // We assume the the points a sorted here.
    auto dataArray = dataRepr_->getDataTyped();
    const auto size = dataRepr_->getDimensions().x;

    interpolateAndStoreColors(dataArray, size);

    for (size_t i = 0; i < size_t(maskMin_ * size); i++) dataArray[i].a = 0.0;
    for (size_t i = size_t(maskMax_ * size); i < size; i++) dataArray[i].a = 0.0;

    data_->invalidateAllOther(dataRepr_.get());

    invalidData_ = false;
}

std::string TransferFunction::getTitle() const { return "Transfer Function"; }

std::string TransferFunction::serializationKey() const { return "Points"; }

std::string TransferFunction::serializationItemKey() const { return "Point"; }

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs) {
    if (lhs.maskMin_ != rhs.maskMin_) return false;
    if (lhs.maskMax_ == rhs.maskMax_) return false;

    return static_cast<const TFPrimitiveSet&>(lhs) == static_cast<const TFPrimitiveSet&>(rhs);
}

bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs) {
    return !operator==(lhs, rhs);
}

}  // namespace inviwo
