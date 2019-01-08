/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_CIMGLAYERREADER_H
#define IVW_CIMGLAYERREADER_H

#include <modules/cimg/cimgmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/image/layerdisk.h>

namespace inviwo {

/**
 * \ingroup dataio
 * \brief Reader for Images files
 */
class IVW_MODULE_CIMG_API CImgLayerReader : public DataReaderType<Layer> {
public:
    CImgLayerReader();
    CImgLayerReader(const CImgLayerReader& rhs) = default;
    CImgLayerReader& operator=(const CImgLayerReader& that) = default;
    virtual CImgLayerReader* clone() const override;
    virtual ~CImgLayerReader() = default;

    virtual std::shared_ptr<Layer> readData(const std::string& filePath) override;
};

class IVW_MODULE_CIMG_API CImgLayerRAMLoader
    : public DiskRepresentationLoader<LayerRepresentation> {
public:
    CImgLayerRAMLoader(LayerDisk* layerDisk);
    virtual CImgLayerRAMLoader* clone() const override;
    virtual ~CImgLayerRAMLoader() = default;
    virtual std::shared_ptr<LayerRepresentation> createRepresentation() const override;
    virtual void updateRepresentation(std::shared_ptr<LayerRepresentation> dest) const override;

    using type = std::shared_ptr<LayerRAM>;

    template <typename Result, typename T>
    std::shared_ptr<LayerRAM> operator()(void* data) const {
        using F = typename T::type;
        auto layerRAM = std::make_shared<LayerRAMPrecision<F>>(
            static_cast<F*>(data), layerDisk_->getDimensions(), layerDisk_->getLayerType(),
            layerDisk_->getSwizzleMask());
        return layerRAM;
    }

private:
    static void updateSwizzleMask(LayerDisk* layerDisk);

    LayerDisk* layerDisk_;
};

}  // namespace inviwo

#endif  // IVW_CIMGLAYERREADER_H
