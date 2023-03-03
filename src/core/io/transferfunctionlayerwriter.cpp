/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/core/io/transferfunctionlayerwriter.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datawriterfactory.h>

namespace inviwo {

TransferFunctionLayerWriter::TransferFunctionLayerWriter(
    std::unique_ptr<DataWriterType<Layer>> layerWriter)
    : layerWriter_{std::move(layerWriter)} {

    for (auto& ext : layerWriter_->getExtensions()) {
        addExtension({ext.extension_, fmt::format("TransferFunction to {}", ext.description_)});
    }
}

TransferFunctionLayerWriter::TransferFunctionLayerWriter(const TransferFunctionLayerWriter& rhs)
    : DataWriterType<TransferFunction>{rhs}, layerWriter_{rhs.layerWriter_->clone()} {}

TransferFunctionLayerWriter& TransferFunctionLayerWriter::operator=(
    const TransferFunctionLayerWriter& that) {
    if (this != &that) {
        layerWriter_.reset(that.layerWriter_->clone());
    }
    return *this;
}

TransferFunctionLayerWriter* TransferFunctionLayerWriter::clone() const {
    return new TransferFunctionLayerWriter{*this};
}

namespace {

std::unique_ptr<Layer> toUint8Layer(const TransferFunction* tf) {

    // src is vec4
    auto src = tf->getRamRepresentation();

    // Convert layer to UINT8
    auto dst = std::make_shared<LayerRAMPrecision<glm::u8vec4>>(src->getDimensions());
    auto layer = std::make_unique<Layer>(dst);

    const auto size = glm::compMul(src->getDimensions());
    const auto sptr = src->getDataTyped();
    const auto dptr = dst->getDataTyped();

    for (size_t i = 0; i < size; ++i) {
        dptr[i] = static_cast<glm::u8vec4>(glm::clamp(sptr[i] * 255.0f, vec4(0.0f), vec4(255.0f)));
    }

    return layer;
}

}  // namespace

void TransferFunctionLayerWriter::writeData(const TransferFunction* tf,
                                            std::string_view filePath) const {

    auto uint8Layer = toUint8Layer(tf);
    layerWriter_->setOverwrite(Overwrite::Yes);
    layerWriter_->writeData(uint8Layer.get(), filePath);
}

std::unique_ptr<std::vector<unsigned char>> TransferFunctionLayerWriter::writeDataToBuffer(
    const TransferFunction* tf, std::string_view fileExtension) const {

    auto uint8Layer = toUint8Layer(tf);
    return layerWriter_->writeDataToBuffer(uint8Layer.get(), fileExtension);
}

TransferFunctionLayerWriterWrapper::TransferFunctionLayerWriterWrapper(DataWriterFactory* factory)
    : factory_{factory} {
    factory_->addObserver(this);
}

void TransferFunctionLayerWriterWrapper::onRegister(DataWriter* writer) {
    if (auto layerWriter = dynamic_cast<DataWriterType<Layer>*>(writer)) {
        if (auto [it, inserted] = layerToTFMap_.try_emplace(
                layerWriter, std::make_unique<TransferFunctionLayerWriter>(
                                 std::unique_ptr<DataWriterType<Layer>>(layerWriter->clone())));
            inserted) {
            factory_->registerObject(it->second.get());
        }
    }
}
void TransferFunctionLayerWriterWrapper::onUnRegister(DataWriter* writer) {
    if (auto layerWriter = dynamic_cast<DataWriterType<Layer>*>(writer)) {
        if (auto it = layerToTFMap_.find(layerWriter); it != layerToTFMap_.end()) {
            factory_->unRegisterObject(it->second.get());
            layerToTFMap_.erase(it);
        }
    }
}

}  // namespace inviwo
