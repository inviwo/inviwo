/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_NIFTIREADER_H
#define IVW_NIFTIREADER_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/indexmapper.h>
#include <modules/nifti/niftimoduledefine.h>

#include <array>

#include <nifti1.h>
#include <nifti1_io.h>

namespace inviwo {

/**
 * \class NiftiReader
 * \brief Volume data reader for Nifti-1 files.
 *
 */
class IVW_MODULE_NIFTI_API NiftiReader
    : public DataReaderType<std::vector<std::shared_ptr<Volume>>> {
public:
    using VolumeSequence = std::vector<std::shared_ptr<Volume>>;
    NiftiReader();
    NiftiReader(const NiftiReader& rhs) = default;
    NiftiReader& operator=(const NiftiReader& that) = default;
    virtual NiftiReader* clone() const override;

    virtual ~NiftiReader() = default;

    virtual std::shared_ptr<VolumeSequence> readData(const std::string& filePath) override;

    /**
     * \brief Convert from Nifti defined data types to inviwo DataFormat.
     *
     * @param niftiDataType nifti_image::datatype.
     * @return Equivalent data type, null if not found.
     */
    static const DataFormatBase* niftiDataTypeToInviwoDataFormat(int niftiDataType);
};

/**
 * \brief A loader of Nifti files. Used to create VolumeRAM representations.
 * This class us used by the NiftiReader.
 */
class IVW_MODULE_NIFTI_API NiftiVolumeRAMLoader
    : public DiskRepresentationLoader<VolumeRepresentation> {
public:
    NiftiVolumeRAMLoader(std::shared_ptr<nifti_image> nim_, std::array<int, 7> start_index_,
                         std::array<int, 7> region_size_, std::array<bool, 3> flipAxis);
    virtual NiftiVolumeRAMLoader* clone() const override;
    virtual ~NiftiVolumeRAMLoader() = default;

    virtual std::shared_ptr<VolumeRepresentation> createRepresentation() const override;
    virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest) const override;

    using type = std::shared_ptr<VolumeRAM>;

    template <typename Result, typename T>
    std::shared_ptr<VolumeRAM> operator()() const {
        using F = typename T::type;

        const std::size_t size = region_size[0] * region_size[1] * region_size[2] * region_size[3] *
                                 region_size[4] * region_size[5] * region_size[6];
        auto data = std::make_unique<F[]>(size);

        if (!data) {
            throw DataReaderException(
                "Error: Could not allocate memory for loading raw file: " + std::string(nim->fname),
                IVW_CONTEXT);
        }
        auto dataPointer = reinterpret_cast<void*>(data.get());
        auto readBytes =
            nifti_read_subregion_image(nim.get(), const_cast<int*>(start_index.data()),
                                       const_cast<int*>(region_size.data()), &dataPointer);
        if (readBytes < 0) {
            throw DataReaderException(
                "Error: Could not read data from file: " + std::string(nim->fname), IVW_CONTEXT);
        }
        // Flip data along axes if necessary
        if (flipAxis[0] || flipAxis[1] || flipAxis[2]) {
            auto tmp = std::make_unique<F[]>(size);
            std::memcpy(tmp.get(), dataPointer, size * sizeof(F));
            auto dim = size3_t{region_size[0], region_size[1], region_size[2]};
            util::IndexMapper3D mapper(dim);
            for (auto z = 0; z < region_size[2]; ++z) {
                auto idz = flipAxis[2] ? region_size[2] - 1 - z : z;
                for (auto y = 0; y < region_size[1]; ++y) {
                    auto idy = flipAxis[1] ? region_size[1] - 1 - y : y;
                    for (auto x = 0; x < region_size[0]; ++x) {
                        auto idx = flipAxis[0] ? region_size[0] - 1 - x : x;
                        auto from = mapper(x, y, z);
                        auto to = mapper(idx, idy, idz);
                        data[to] = tmp[from];
                    }
                }
            }
        }

        auto repr = std::make_shared<VolumeRAMPrecision<F>>(
            data.get(), size3_t{region_size[0], region_size[1], region_size[2]});
        data.release();
        return repr;
    }

private:
    std::array<int, 7> start_index;
    std::array<int, 7> region_size;
    std::array<bool, 3> flipAxis;  // Flip x,y,z axis?
    std::shared_ptr<nifti_image> nim;
};

}  // namespace inviwo

#endif  // IVW_NIFTIREADER_H
