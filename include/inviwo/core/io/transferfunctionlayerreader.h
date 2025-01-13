/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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
#include <inviwo/core/io/datareader.h>                    // for DataReaderType
#include <inviwo/core/datastructures/image/layer.h>

#include <memory>

namespace inviwo {

class DataReaderFactory;

class IVW_CORE_API TransferFunctionLayerReader : public DataReaderType<TransferFunction> {
public:
    explicit TransferFunctionLayerReader(std::unique_ptr<DataReaderType<Layer>> layerReader);
    TransferFunctionLayerReader(const TransferFunctionLayerReader& rhs);
    TransferFunctionLayerReader(TransferFunctionLayerReader&&) noexcept = default;
    TransferFunctionLayerReader& operator=(const TransferFunctionLayerReader& that);
    TransferFunctionLayerReader& operator=(TransferFunctionLayerReader&&) noexcept = default;
    virtual ~TransferFunctionLayerReader() override = default;

    virtual TransferFunctionLayerReader* clone() const override;

    virtual std::shared_ptr<TransferFunction> readData(
        const std::filesystem::path& filePath) override;

private:
    std::unique_ptr<DataReaderType<Layer>> layerReader_;
};

class IVW_CORE_API TransferFunctionLayerReaderWrapper : public FactoryObserver<DataReader> {
public:
    explicit TransferFunctionLayerReaderWrapper(DataReaderFactory* factory);
    virtual void onRegister(DataReader* reader) override;
    virtual void onUnRegister(DataReader* reader) override;

private:
    DataReaderFactory* factory_;
    std::unordered_map<DataReaderType<Layer>*, std::shared_ptr<TransferFunctionLayerReader>>
        layerToTFMap_;
};

}  // namespace inviwo
