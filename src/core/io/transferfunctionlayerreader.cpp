/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/core/io/transferfunctionlayerreader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

namespace inviwo {

TransferFunctionLayerReader::TransferFunctionLayerReader(
    std::unique_ptr<DataReaderType<Layer>> layerReader)
    : layerReader_{std::move(layerReader)} {

    for (auto& ext : layerReader_->getExtensions()) {
        addExtension({ext.extension_, fmt::format("TransferFunction from {}", ext.description_)});
    }
}

TransferFunctionLayerReader::TransferFunctionLayerReader(const TransferFunctionLayerReader& rhs)
    : DataReaderType<TransferFunction>{rhs}, layerReader_{rhs.layerReader_->clone()} {}

TransferFunctionLayerReader& TransferFunctionLayerReader::operator=(
    const TransferFunctionLayerReader& that) {
    if (this != &that) {
        layerReader_.reset(that.layerReader_->clone());
    }
    return *this;
}

TransferFunctionLayerReader* TransferFunctionLayerReader::clone() const {
    return new TransferFunctionLayerReader{*this};
};

std::shared_ptr<TransferFunction> TransferFunctionLayerReader::readData(
    const std::filesystem::path& filePath) {

    const auto layer = layerReader_->readData(filePath);
    auto tf = std::make_shared<TransferFunction>();

    layer->getRepresentation<LayerRAM>()->dispatch<void>([&](auto lrprecision) {
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

        const auto simplified = TransferFunction::simplify(points, 0.01);
        tf->add(simplified);
    });

    return tf;
};

TransferFunctionLayerReaderWrapper::TransferFunctionLayerReaderWrapper(DataReaderFactory* factory)
    : factory_{factory} {
    factory_->addObserver(this);
}

void TransferFunctionLayerReaderWrapper::onRegister(DataReader* reader) {
    if (auto layerReader = dynamic_cast<DataReaderType<Layer>*>(reader)) {
        if (auto [it, inserted] = layerToTFMap_.try_emplace(
                layerReader, std::make_unique<TransferFunctionLayerReader>(
                                 std::unique_ptr<DataReaderType<Layer>>(layerReader->clone())));
            inserted) {
            factory_->registerObject(it->second.get());
        }
    }
}
void TransferFunctionLayerReaderWrapper::onUnRegister(DataReader* reader) {
    if (auto layerReader = dynamic_cast<DataReaderType<Layer>*>(reader)) {
        if (auto it = layerToTFMap_.find(layerReader); it != layerToTFMap_.end()) {
            factory_->unRegisterObject(it->second.get());
            layerToTFMap_.erase(it);
        }
    }
}

}  // namespace inviwo
