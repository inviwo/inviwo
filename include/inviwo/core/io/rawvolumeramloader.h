/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_RAWVOLUMERAMLOADER_H
#define IVW_RAWVOLUMERAMLOADER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/bytereaderutil.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

/**
 * \class RawVolumeRAMLoader
 *
 * \brief A loader of raw files. Used to create VolumeRAM representations.
 *
 * This class us used by the DatVolumeReader, IvfVolumeReader and RawVolumeReader.
 */

class IVW_CORE_API RawVolumeRAMLoader : public DiskRepresentationLoader {
public:
    RawVolumeRAMLoader(const std::string& rawFile, size_t offset, size3_t dimensions,
                       bool littleEndian, const DataFormatBase* format);
    virtual RawVolumeRAMLoader* clone() const override;
    virtual std::shared_ptr<DataRepresentation> createRepresentation() const override;
    virtual void updateRepresentation(std::shared_ptr<DataRepresentation> dest) const override;

    using type = std::shared_ptr<VolumeRAM>;

    template <class T>
    std::shared_ptr<VolumeRAM> dispatch() const {
        typedef typename T::type F;

        std::size_t size = dimensions_.x * dimensions_.y * dimensions_.z;
        auto data = util::make_unique<F[]>(size);

        if (!data) {
            throw DataReaderException(
                "Error: Could not allocate memory for loading raw file: " + rawFile_, IvwContext);
        }

        util::readBytesIntoBuffer(rawFile_, offset_, size * format_->getSize(), littleEndian_,
                                  format_->getSize(), data.get());

        auto repr = std::make_shared<VolumeRAMPrecision<F>>(data.get(), dimensions_);
        data.release();
        return repr; 
    }

private:
    std::string rawFile_;
    size_t offset_;
    size3_t dimensions_;
    bool littleEndian_;
    const DataFormatBase* format_;
};

}  // namespace

#endif  // IVW_RAWVOLUMERAMLOADER_H
