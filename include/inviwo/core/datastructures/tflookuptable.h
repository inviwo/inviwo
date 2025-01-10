/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/transferfunction.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/util/glmvec.h>

namespace inviwo {

class IVW_CORE_API TFLookupTable : public TFPrimitiveSetObserver {
public:
    explicit TFLookupTable(TransferFunction& tf, size_t size = 1024);
    TFLookupTable(const TFLookupTable&) = delete;
    TFLookupTable(TFLookupTable&&) noexcept = default;
    TFLookupTable& operator=(const TFLookupTable&) = delete;
    TFLookupTable& operator=(TFLookupTable&&) noexcept = default;
    virtual ~TFLookupTable() = default;

    void setSize(size_t size);
    size_t getSize() const;

    const TransferFunction& getTransferFunction() const;
    void setTransferFunction(TransferFunction& tf);

    template <typename T>
    const T* getRepresentation() {
        if (invalid_) {
            calc();
        }
        return data_->getRepresentation<T>();
    }

protected:
    virtual void onTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p) override;
    virtual void onTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type) override;
    virtual void onTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask) override;

private:
    void calc();

    TransferFunction* tf_;  // Should not be null
    size_t size_;
    bool invalid_;
    std::shared_ptr<LayerRAMPrecision<vec4>> repr_;
    std::unique_ptr<Layer> data_;
};

}  // namespace inviwo
