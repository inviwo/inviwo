/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2021 Inviwo Foundation
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

#include <modules/nifti/niftireader.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareaderexception.h>

#include <modules/base/algorithm/dataminmax.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

NiftiReader::NiftiReader() : DataReaderType<VolumeSequence>() {
    addExtension(FileExtension("nii", "NIfTI-1 file format"));
    addExtension(FileExtension("hdr", "ANALYZE file format"));
    addExtension(FileExtension("img", "ANALYZE file format"));
    // Compressed files
    addExtension(FileExtension("nii.gz", "NIfTI-1 file format"));
    addExtension(FileExtension("hdr.gz", "ANALYZE file format"));
    addExtension(FileExtension("img.gz", "ANALYZE file format"));
}

NiftiReader* NiftiReader::clone() const { return new NiftiReader(*this); }

const DataFormatBase* NiftiReader::niftiDataTypeToInviwoDataFormat(int niftiDataType) {
    // clang-format off
    switch (niftiDataType){
        case DT_UNKNOWN:    return nullptr;
        case DT_BINARY:     return nullptr;
        case DT_INT8:       return DataInt8::get();
        case DT_UINT8:      return DataUInt8::get();
        case DT_INT16:      return DataInt16::get();
        case DT_UINT16:     return DataUInt16::get();
        case DT_INT32:      return DataInt32::get();
        case DT_UINT32:     return DataUInt32::get();
        case DT_INT64:      return DataInt64::get();
        case DT_UINT64:     return DataUInt64::get();
        case DT_FLOAT32:    return DataFloat32::get();
        case DT_FLOAT64:    return DataFloat64::get();
        case DT_FLOAT128:   return nullptr;
        case DT_COMPLEX64:  return nullptr;
        case DT_COMPLEX128: return nullptr;
        case DT_COMPLEX256: return nullptr;
        case DT_RGB24:      return DataVec3UInt8::get();
        case DT_RGBA32:     return DataVec4UInt8::get();
        default:
            return nullptr;
    }
    // clang-format on
}

std::shared_ptr<NiftiReader::VolumeSequence> NiftiReader::readData(const std::string& filePath) {

    /* read input dataset, but not data */
    std::shared_ptr<nifti_image> niftiImage(nifti_image_read(filePath.c_str(), 0),
                                            nifti_image_free);
    if (!niftiImage) {
        throw DataReaderException("Error: failed to read NIfTI image in file: " + filePath,
                                  IVW_CONTEXT_CUSTOM("NiftiReader"));
    }

    const DataFormatBase* format = nullptr;
    // nim->dim[0] = number of dimensions;
    // ANALYZE supports dim[0] up to 7, but NIFTI-1 reserves
    // dimensions 1, 2, 3 for space(x, y, z), 4 for time(t), and
    // 5, 6, 7 for anything else needed.
    size3_t dim(niftiImage->dim[1], niftiImage->dim[2], niftiImage->dim[3]);
    if (glm::any(glm::equal(dim, size3_t(0)))) {
        throw DataReaderException(
            fmt::format("Unsupported dimension '{}' in nifti file: {}", dim, filePath),
            IVW_CONTEXT_CUSTOM("NiftiReader"));
    }
    glm::mat4 basisAndOffset(2.0f);
    const glm::vec3 spacing(niftiImage->pixdim[1], niftiImage->pixdim[2], niftiImage->pixdim[3]);

    format = niftiDataTypeToInviwoDataFormat(niftiImage->datatype);
    if (format == nullptr) {
        std::string datatype(nifti_datatype_string(niftiImage->datatype));
        throw DataReaderException(
            fmt::format("Unsupported format '{}' in nifti file: {}", datatype, filePath),
            IVW_CONTEXT_CUSTOM("NiftiReader"));
    }

    auto volume = std::make_shared<Volume>(dim, format);

    std::string descrip(niftiImage->descrip);
    if (!descrip.empty()) {
        // Additional information
        volume->setMetaData<StringMetaData>("description", descrip);
    }
    mat4 niftiIndexToModel(1.f);

    // qform_code / sform_code
    // NIFTI_XFORM_UNKNOWN      0 Arbitrary coordinates (Method 1).
    // NIFTI_XFORM_SCANNER_ANAT 1 Scanner-based anatomical coordinates
    // NIFTI_XFORM_ALIGNED_ANAT 2 Coordinates aligned to another file's, or to anatomical "truth".
    // NIFTI_XFORM_TALAIRACH    3 Coordinates aligned to Talairach-Tournoux Atlas; (0,0,0)=AC, etc.
    // NIFTI_XFORM_MNI_152      4 MNI 152 normalized coordinates.
    if (niftiImage->qform_code > 0) {
        // Represents the nominal voxel locations as reported by the scanner, or as rotated to some
        // fiducial orientation and location. The origin of coordinates would generally be whatever
        // the scanner origin is; for example, in MRI, (0, 0, 0) is the center of the gradient coil

        const auto& m = niftiImage->qto_xyz.m;
        niftiIndexToModel =
            glm::mat4x4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1],
                        m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
    } else if (niftiImage->sform_code > 0) {
        // give the location of the voxels in some standard space. The origin of coordinates would
        // depend on the value of sform_code; for example, for the Talairach coordinate system, (0,
        // 0, 0) corresponds to the Anterior Commissure.
        const auto& m = niftiImage->sto_xyz.m;
        niftiIndexToModel =
            glm::mat4x4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1],
                        m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
    } else {
        // METHOD 1 (the "old" way, used only when qform_code = 0)
        niftiIndexToModel[0][0] = -0.5f * niftiImage->pixdim[1];
        niftiIndexToModel[1][1] = -0.5f * niftiImage->pixdim[2];
        niftiIndexToModel[2][2] = -0.5f * niftiImage->pixdim[3];
        vec3 offset = -0.5f * ((vec3(dim) - 0.5f) * spacing);
        niftiIndexToModel[3][0] = offset[0];
        niftiIndexToModel[3][1] = offset[1];
        niftiIndexToModel[3][2] = offset[2];
    }

    // Compute the extent of the entire data set
    // Nifti supplies a matrix to compute the center position of the voxel.
    // Here we compute the start basis vectors spanning the data set
    basisAndOffset[0] = niftiIndexToModel * vec4(dim[0] - 0.5f, 0.f, 0.f, 1.f) -
                        niftiIndexToModel * vec4(-0.5f, 0.f, 0.f, 1.f);
    basisAndOffset[1] = niftiIndexToModel * vec4(0.f, dim[1] - 0.5f, 0.f, 1.f) -
                        niftiIndexToModel * vec4(0.f, -0.5f, 0.f, 1.f);
    basisAndOffset[2] = niftiIndexToModel * vec4(0.f, 0.f, dim[2] - 0.5f, 1.f) -
                        niftiIndexToModel * vec4(0.f, 0.f, -0.5f, 1.f);
    basisAndOffset[3] = niftiIndexToModel * vec4(-0.5f, -0.5f, -0.5f, 1.f);

    // Flip coordinate system and data if the data is provided in neurological convention
    auto modelToWorld = volume->getWorldMatrix();
    std::array<bool, 3> flipAxis = {false, false, false};
    if (niftiIndexToModel[0][0] < 0.f) {
        // flip x-axis
        modelToWorld[0] *= -1.f;
        modelToWorld[3][0] *= -1.f;
        flipAxis[0] = true;
    }
    if (niftiIndexToModel[1][1] < 0.f) {
        // flip y-axis
        modelToWorld[1] *= -1.f;
        modelToWorld[3][1] *= -1.f;
        flipAxis[1] = true;
    }
    if (niftiIndexToModel[2][2] < 0.f) {
        // flip z-axis
        modelToWorld[2] *= -1.f;
        modelToWorld[3][2] *= -1.f;
        flipAxis[2] = true;
    }

    volume->setModelMatrix(basisAndOffset);
    volume->setWorldMatrix(modelToWorld);

    std::array<int, 7> start_index = {0, 0, 0, 0, 0, 0, 0};
    std::array<int, 7> region_size = {
        niftiImage->dim[1], niftiImage->dim[2], niftiImage->dim[3], 1, 1, 1, 1};

    auto volumes = std::make_shared<VolumeSequence>();
    // Fixes single-volume where dim[4] has been set to zero
    auto nTimeSteps = niftiImage->dim[4] > 0 ? niftiImage->dim[4] : 1;
    // Lazy loading of time steps
    for (int t = 0; t < nTimeSteps; ++t) {
        volumes->push_back(std::shared_ptr<Volume>(volume->clone()));
        auto diskRepr = std::make_shared<VolumeDisk>(filePath, dim, format);
        start_index[3] = t;
        diskRepr->setLoader(
            new NiftiVolumeRAMLoader(niftiImage, start_index, region_size, flipAxis));
        volumes->back()->addRepresentation(diskRepr);
    }

    /*
     We currently have no good way of considering cal_min and cal_max.
     See tests/volumes/nifti/volzstat1.nii.gz for an example including cal_min/cal_max.

    The cal_min and cal_max fields (if nonzero) are used for mapping (possibly
    scaled) dataset values to display colors:
    - Minimum display intensity (black) corresponds to dataset value cal_min.
    - Maximum display intensity (white) corresponds to dataset value cal_max.
    - Dataset values below cal_min should display as black also, and values
    above cal_max as white.
    - Colors "black" and "white", of course, may refer to any scalar display
    scheme (e.g., a color lookup table specified via aux_file).
    - cal_min and cal_max only make sense when applied to scalar-valued
    datasets (i.e., dim[0] < 5 or dim[5] = 1)
    */
    DataMapper dm{format};

    // No need to modify range for 8-bit formats since normalization will work well anyway
    if (format->getPrecision() > 8) {
        // These formats may have a tricky data range,
        // so we need to compute it for valid display ranges

        auto volRAM = volumes->front()->getRepresentation<VolumeRAM>();
        auto minmax = util::volumeMinMax(volRAM);
        // minmax always have four components, unused components are set to zero.
        // Hence, only consider components used by the data format
        dvec2 dataRange(minmax.first[0], minmax.second[0]);
        // min/max of all components
        for (size_t component = 1; component < format->getComponents(); ++component) {
            dataRange = dvec2(glm::min(dataRange[0], minmax.first[component]),
                                glm::max(dataRange[1], minmax.second[component]));
        }
        if (format->getId() == DataFormatId::UInt16) {
            // Try to make different UInt16 comparable
            // by not modifying the range
            if (dataRange.y < 4096.) {
                // All values within 12-bit range so we guess that this is a 12-bit data set
                dataRange = dvec2(0., 4095.);
                LogInfo(
                    "Guessing 12-bit data range in 16-bit data since all values are below "
                    "4096. Change data range in VolumeSource to [0 65535] if this is "
                    "incorrect.");
            } else {
                // This was probably a 16-bit data set after all
                dataRange = dvec2(0., format->getMax());
            }
        }
        dm.dataRange = dataRange;
    }

    if (niftiImage->scl_slope != 0) {
        // If the scl_slope field is nonzero, then each voxel value in the dataset
        // should be scaled as
        //    y = scl_slope  x + scl_inter
        // where x = stored voxel value and y = "true" voxel value
        dm.valueRange.x = static_cast<double>(niftiImage->scl_slope) * dm.dataRange.x +
                          static_cast<double>(niftiImage->scl_inter);
        dm.valueRange.y = static_cast<double>(niftiImage->scl_slope) * dm.dataRange.y +
                          static_cast<double>(niftiImage->scl_inter);
    } else {
        dm.valueRange = dm.dataRange;
    }

    for (auto& vol : *volumes) {
        vol->dataMap_ = dm;
    }

    return volumes;
}

NiftiVolumeRAMLoader::NiftiVolumeRAMLoader(std::shared_ptr<nifti_image> nim_,
                                           std::array<int, 7> start_index_,
                                           std::array<int, 7> region_size_,
                                           std::array<bool, 3> flipAxis_)
    : DiskRepresentationLoader()
    , start_index(start_index_)
    , region_size(region_size_)
    , flipAxis(flipAxis_)
    , nim{nim_} {}

NiftiVolumeRAMLoader* NiftiVolumeRAMLoader::clone() const {
    return new NiftiVolumeRAMLoader(*this);
}

void flip(char* data, size_t elemSize, size3_t dim, std::array<bool, 3> flipAxis) {
    // Flip data along axes if necessary
    if (flipAxis[0] || flipAxis[1] || flipAxis[2]) {
        const auto size = glm::compMul(dim);
        auto tmp = std::make_unique<char[]>(elemSize * size);
        auto copy = tmp.get();

        std::memcpy(copy, data, size * elemSize);

        util::IndexMapper3D mapper(dim);
        for (size_t z = 0; z < dim[2]; ++z) {
            const auto idz = flipAxis[2] ? dim[2] - 1 - z : z;
            for (size_t y = 0; y < dim[1]; ++y) {
                const auto idy = flipAxis[1] ? dim[1] - 1 - y : y;
                for (size_t x = 0; x < dim[0]; ++x) {
                    const auto idx = flipAxis[0] ? dim[0] - 1 - x : x;
                    const auto from = mapper(x, y, z);
                    const auto to = mapper(idx, idy, idz);
                    std::memcpy(data + to * elemSize, copy + from * elemSize, elemSize);
                }
            }
        }
    }
}

std::shared_ptr<VolumeRepresentation> NiftiVolumeRAMLoader::createRepresentation(
    const VolumeRepresentation& src) const {

    const auto format = NiftiReader::niftiDataTypeToInviwoDataFormat(nim->datatype);
    const auto voxelSize = format->getSize();

    const std::size_t voxels = region_size[0] * region_size[1] * region_size[2] * region_size[3] *
                               region_size[4] * region_size[5] * region_size[6];

    auto data = std::make_unique<char[]>(voxels * voxelSize);

    void* pdata = static_cast<void*>(data.get());
    auto start = start_index;
    auto region = region_size;
    auto readBytes = nifti_read_subregion_image(nim.get(), start.data(), region.data(), &pdata);

    const auto dim = size3_t{region_size[0], region_size[1], region_size[2]};
    flip(data.get(), voxelSize, dim, flipAxis);

    if (readBytes < 0) {
        throw DataReaderException(
            "Error: Could not read data from file: " + std::string(nim->fname), IVW_CONTEXT);
    }

    auto volumeRAM =
        createVolumeRAM(src.getDimensions(), src.getDataFormat(), data.get(), src.getSwizzleMask(),
                        src.getInterpolation(), src.getWrapping());
    data.release();

    return volumeRAM;
}

void NiftiVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                                const VolumeRepresentation& src) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    if (size3_t{region_size[0], region_size[1], region_size[2]} != volumeDst->getDimensions()) {
        throw Exception("Mismatching volume dimensions, can't update", IVW_CONTEXT);
    }
    auto data = volumeDst->getData();

    auto start = start_index;
    auto region = region_size;
    auto readBytes = nifti_read_subregion_image(nim.get(), start.data(), region.data(), &data);
    if (readBytes < 0) {
        throw DataReaderException(
            "Error: Could not read data from file: " + std::string(nim->fname), IVW_CONTEXT);
    }

    const auto voxelSize = src.getDataFormat()->getSize();
    const auto dim = size3_t{region_size[0], region_size[1], region_size[2]};

    flip(static_cast<char*>(data), voxelSize, dim, flipAxis);
}

}  // namespace inviwo
