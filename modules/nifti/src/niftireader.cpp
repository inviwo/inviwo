/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera/camera.h>                   // for mat4
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/diskrepresentation.h>              // for DiskRepresentatio...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataReade...
#include <inviwo/core/datastructures/volume/volumedisk.h>               // for VolumeDisk
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumeramprecision.h>       // for createVolumeRAM
#include <inviwo/core/datastructures/volume/volumerepresentation.h>     // for VolumeRepresentation
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/metadata/metadata.h>                              // for StringMetaData
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/formats.h>                                   // for NumericType, Data...
#include <inviwo/core/util/glmvec.h>                                    // for size3_t, dvec2, vec4
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/safecstr.h>                                  // for SafeCStr
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT, IVW_...
#include <modules/base/algorithm/dataminmax.h>                          // for volumeMinMax

#include <warn/push>
#include <warn/ignore/all>
#include <glm/common.hpp>              // for max, min
#include <glm/fwd.hpp>                 // for mat4, mat4x4, vec3
#include <glm/gtx/component_wise.hpp>  // for compMul
#include <glm/mat4x4.hpp>              // for mat<>::col_type
#include <glm/vec2.hpp>                // for vec<>::(anonymous)
#include <glm/vec3.hpp>                // for operator*, operat...
#include <glm/vec4.hpp>                // for operator*, operator+
#include <glm/vector_relational.hpp>   // for any, equal
#include <nifti1.h>                    // for DT_BINARY, DT_COM...
#include <nifti1_io.h>                 // for nifti_image, nift...

#include <warn/pop>

#include <array>          // for array
#include <cstring>        // for size_t, memcpy
#include <string>         // for string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, move

#include <fmt/std.h>

namespace inviwo {

/**
 * \brief A loader of Nifti files. Used to create VolumeRAM representations.
 * This class us used by the NiftiReader.
 */
class NiftiVolumeRAMLoader : public DiskRepresentationLoader<VolumeRepresentation> {
public:
    NiftiVolumeRAMLoader(std::shared_ptr<nifti_image> nim_, std::array<int, 7> start_index_,
                         std::array<int, 7> region_size_, std::array<bool, 3> flipAxis);
    virtual NiftiVolumeRAMLoader* clone() const override;
    virtual ~NiftiVolumeRAMLoader() = default;

    virtual std::shared_ptr<VolumeRepresentation> createRepresentation(
        const VolumeRepresentation& src) const override;
    virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                      const VolumeRepresentation& src) const override;

private:
    std::array<int, 7> start_index;
    std::array<int, 7> region_size;
    std::array<bool, 3> flipAxis;  // Flip x,y,z axis?
    std::shared_ptr<nifti_image> nim;
};

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
/**
 * \brief Convert from Nifti defined data types to inviwo DataFormat.
 *
 * @param niftiDataType nifti_image::datatype.
 * @return Equivalent data type, null if not found.
 */
const DataFormatBase* niftiDataTypeToInviwoDataFormat(const nifti_image* niftiImage) {

    NumericType type;
    size_t components = 1;
    size_t precision = 1;
    // clang-format off
    switch (niftiImage->datatype){
        case DT_UNKNOWN:    return nullptr;
        case DT_BINARY:     return nullptr;
        case DT_INT8:
            type = NumericType::SignedInteger;
            precision = 8;
            break;
        case DT_UINT8:
            type = NumericType::UnsignedInteger;
            precision = 8;
            break;
        case DT_INT16:
            type = NumericType::SignedInteger;
            precision = 16;
            break;
        case DT_UINT16:
            type = NumericType::UnsignedInteger;
            precision = 16;
            break;
        case DT_INT32:
            type = NumericType::SignedInteger;
            precision = 32;
            break;
        case DT_UINT32:
            type = NumericType::UnsignedInteger;
            precision = 32;
            break;
        case DT_INT64:
            type = NumericType::SignedInteger;
            precision = 64;
            break;
        case DT_UINT64:
            type = NumericType::UnsignedInteger;
            precision = 64;
            break;
        case DT_FLOAT32:
            type = NumericType::Float;
            precision = 32;
            break;
        case DT_FLOAT64:
            type = NumericType::Float;
            precision = 64;
            break;
        case DT_FLOAT128:   return nullptr;
        case DT_COMPLEX64:  return nullptr;
        case DT_COMPLEX128: return nullptr;
        case DT_COMPLEX256: return nullptr;
        case DT_RGB24:      return DataVec3UInt8::get(); // Do not know if niftiImage->dim[5] is 1 for this type
        case DT_RGBA32:     return DataVec4UInt8::get(); // Do not know if niftiImage->dim[5] is 1 for this type
        default:
            return nullptr;
    }
    // clang-format on
    // In NIFTI-1 files, dimensions 1,2,3 are for space, dimension 4 is for time,
    // and dimension 5 is for storing multiple values at each spatiotemporal voxel
    if (niftiImage->dim[5] > 0) {
        components = niftiImage->dim[5];
    }
    if (components > 4) {
        return nullptr;
    }

    return DataFormatBase::get(type, components, precision);
}
std::shared_ptr<NiftiReader::VolumeSequence> NiftiReader::readData(
    const std::filesystem::path& filePath) {
    checkExists(filePath);

    /* read input dataset, but not data */
    std::shared_ptr<nifti_image> niftiImage(nifti_image_read(filePath.string().c_str(), 0),
                                            nifti_image_free);
    if (!niftiImage) {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("NiftiReader"),
                                  "Error: failed to read NIfTI image in file: {}", filePath);
    }

    const DataFormatBase* format = nullptr;
    // nim->dim[0] = number of dimensions;
    // ANALYZE supports dim[0] up to 7, but NIFTI-1 reserves
    // dimensions 1, 2, 3 for space(x, y, z), 4 for time(t), and
    // 5, 6, 7 for anything else needed.
    size3_t dim(niftiImage->dim[1], niftiImage->dim[2], niftiImage->dim[3]);
    if (glm::any(glm::equal(dim, size3_t(0)))) {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("NiftiReader"),
                                  "Unsupported dimension '{}' in nifti file: {}", dim, filePath);
    }
    glm::mat4 basisAndOffset(2.0f);
    const glm::vec3 spacing(niftiImage->pixdim[1], niftiImage->pixdim[2], niftiImage->pixdim[3]);

    format = niftiDataTypeToInviwoDataFormat(niftiImage.get());
    if (format == nullptr) {
        std::string datatype(nifti_datatype_string(niftiImage->datatype));
        throw DataReaderException(IVW_CONTEXT_CUSTOM("NiftiReader"),
                                  "Unsupported format '{}' in nifti file: {}", datatype, filePath);
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

    // Flip coordinate system and data if the data is provided in radiological convention
    // https://nipy.org/nibabel/neuro_radio_conventions.html
    std::array<bool, 3> flipAxis = {false, false, false};
    if (niftiIndexToModel[0][0] < 0.f) {
        // flip x-axis
        basisAndOffset[0] *= -1.f;
        basisAndOffset[3][0] *= -1.f;
        flipAxis[0] = true;
    }
    if (niftiIndexToModel[1][1] < 0.f) {
        // flip y-axis
        basisAndOffset[1] *= -1.f;
        basisAndOffset[3][1] *= -1.f;
        flipAxis[1] = true;
    }
    if (niftiIndexToModel[2][2] < 0.f) {
        // flip z-axis
        basisAndOffset[2] *= -1.f;
        basisAndOffset[3][2] *= -1.f;
        flipAxis[2] = true;
    }

    volume->setModelMatrix(basisAndOffset);

    std::array<int, 7> start_index = {0, 0, 0, 0, 0, 0, 0};
    std::array<int, 7> region_size = {niftiImage->dim[1],
                                      niftiImage->dim[2],
                                      niftiImage->dim[3],
                                      1,
                                      static_cast<int>(format->getComponents()),
                                      1,
                                      1};

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

    DataMapper dm{format};
    // Figure out how to map the stored values into
    // 1. Normalized domain [0 1] (using dataRange) as used by the Transfer Function (TF)
    // 2. Value range (physical meaning of values)
    if (niftiImage->cal_min != 0 || niftiImage->cal_max != 0) {
        // The values that should map to [0 1] in the TF is provided, i.e. cal_min and cal_max

        /*
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
        // If the scl_slope field is nonzero, then each voxel value in the dataset
        // should be scaled as
        //    y = scl_slope  x + scl_inter
        // where x = stored voxel value and y = "true" voxel value.
        // Otherwise, we assume scl_slope = 1 and scl_inter = 0.
        //
        // Inviwo normalizes data into [0 1] using dataRange and then transforms
        // the values using valueRange
        // So, we need to calculate which values in the data domain
        // that correspond to cal_min/cal_max using:
        // v_min = (cal_min - scl_inter) / scl_slope
        // v_max = (cal_max - scl_inter) / scl_slope
        //
        // Now v_min and v_max can be used to normalize the stored values.
        // Values below v_min and above v_max will not be displayed since the fall outside the
        // normalized [0 1] range.
        double offset = niftiImage->scl_slope != 0 ? niftiImage->scl_inter : 0;
        double slope = niftiImage->scl_slope != 0 ? niftiImage->scl_slope : 1;
        dm.dataRange.x = static_cast<double>(niftiImage->cal_min - offset) / slope;
        dm.dataRange.y = static_cast<double>(niftiImage->cal_max - offset) / slope;

        dm.valueRange.x = niftiImage->cal_min;
        dm.valueRange.y = niftiImage->cal_max;
    } else {
        // No information about display range is provided so we assume that the whole range should
        // be displayed.
        // In other words:
        // The minimum value will correspond to 0 and the maximum will correspond to 1 in the
        // Transfer Function
        for (auto& vol : *volumes) {

            auto volRAM = vol->getRepresentation<VolumeRAM>();
            auto minmax = util::volumeMinMax(volRAM);
            // minmax always have four components, unused components are set to zero.
            // Hence, only consider components used by the data format
            dvec2 dataRange(minmax.first[0], minmax.second[0]);
            // min/max of all components
            for (size_t component = 1; component < format->getComponents(); ++component) {
                dataRange = dvec2(glm::min(dataRange[0], minmax.first[component]),
                                  glm::max(dataRange[1], minmax.second[component]));
            }
            dm.dataRange = dataRange;

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
            vol->dataMap_ = dm;
        }
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

    const auto format = niftiDataTypeToInviwoDataFormat(nim.get());
    const auto voxelSize = format->getSizeInBytes();

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
        throw DataReaderException(IVW_CONTEXT, "Error: Could not read data from file: {}",
                                  nim->fname);
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
        throw DataReaderException("Mismatching volume dimensions, can't update", IVW_CONTEXT);
    }
    auto data = volumeDst->getData();

    auto start = start_index;
    auto region = region_size;
    auto readBytes = nifti_read_subregion_image(nim.get(), start.data(), region.data(), &data);
    if (readBytes < 0) {
        throw DataReaderException(IVW_CONTEXT, "Error: Could not read data from file: {}",
                                  nim->fname);
    }

    const auto voxelSize = src.getDataFormat()->getSizeInBytes();
    const auto dim = size3_t{region_size[0], region_size[1], region_size[2]};

    flip(static_cast<char*>(data), voxelSize, dim, flipAxis);
}

}  // namespace inviwo
