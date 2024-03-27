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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/transferfunction.h>  // For TransferFunction
#include <inviwo/core/io/datawriter.h>                    // For DataWriter
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/datastructures/image/layer.h>

namespace inviwo {

class IVW_CORE_API TransferFunctionLayerWriter : public DataWriterType<TransferFunction> {
public:
    explicit TransferFunctionLayerWriter(std::unique_ptr<DataWriterType<Layer>> layerWriter);
    TransferFunctionLayerWriter(const TransferFunctionLayerWriter& rhs);
    TransferFunctionLayerWriter(TransferFunctionLayerWriter&&) noexcept = default;
    TransferFunctionLayerWriter& operator=(const TransferFunctionLayerWriter& that);
    TransferFunctionLayerWriter& operator=(TransferFunctionLayerWriter&&) noexcept = default;
    virtual ~TransferFunctionLayerWriter() override = default;
    virtual TransferFunctionLayerWriter* clone() const override;

    virtual void writeData(const TransferFunction* data,
                           const std::filesystem::path& filePath) const override;

    virtual std::unique_ptr<std::vector<unsigned char>> writeDataToBuffer(
        const TransferFunction* data, std::string_view fileExtension) const override;

private:
    std::unique_ptr<DataWriterType<Layer>> layerWriter_;
};

class IVW_CORE_API TransferFunctionLayerWriterWrapper : public FactoryObserver<DataWriter> {
public:
    explicit TransferFunctionLayerWriterWrapper(DataWriterFactory* factory);
    virtual void onRegister(DataWriter* Writer) override;
    virtual void onUnRegister(DataWriter* Writer) override;

private:
    DataWriterFactory* factory_;
    std::unordered_map<DataWriterType<Layer>*, std::shared_ptr<TransferFunctionLayerWriter>>
        layerToTFMap_;
};

}  // namespace inviwo
