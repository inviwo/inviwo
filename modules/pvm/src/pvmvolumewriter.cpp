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

#include <modules/pvm/pvmvolumewriter.h>

#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataWrite...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/metadata/metadata.h>                              // for MetaDataPrimitive...
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmvec.h>                                    // for size3_t, vec3
#include <inviwo/core/util/safecstr.h>                                  // for SafeCStr
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT

#include <cstring>        // for strncpy, memcpy
#include <memory>         // for allocator, unique...
#include <string>         // for basic_string, ope...
#include <unordered_set>  // for unordered_set

#include <glm/mat3x3.hpp>   // for mat<>::col_type
#include <glm/vec3.hpp>     // for vec<>::(anonymous)
#include <tidds/ddsbase.h>  // for swapbytes, writeP...

namespace inviwo {

PVMVolumeWriter::PVMVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("pvm", "PVM file format"));
}

PVMVolumeWriter::PVMVolumeWriter(const PVMVolumeWriter& rhs) : DataWriterType<Volume>(rhs) {}

PVMVolumeWriter& PVMVolumeWriter::operator=(const PVMVolumeWriter& that) {
    if (this != &that) DataWriterType<Volume>::operator=(that);

    return *this;
}

PVMVolumeWriter* PVMVolumeWriter::clone() const { return new PVMVolumeWriter(*this); }

void PVMVolumeWriter::writeData(const Volume* data, const std::filesystem::path& filePath) const {
    checkOverwrite(filePath);

    const DataFormatBase* format = data->getDataFormat();
    int components = 0;

#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (format->getId()) {
        case inviwo::DataFormatId::UInt8:
            components = 1;
            break;
        case inviwo::DataFormatId::UInt16:
            components = 2;
            break;
        default:
            break;
    }
#include <warn/pop>

    if (components == 0) {
        throw DataWriterException(IVW_CONTEXT, "Error: Output format {} not support by PVM writer",
                                  format->getString());
    }

    const VolumeRAM* vr = data->getRepresentation<VolumeRAM>();
    const unsigned char* dataPtr = (const unsigned char*)vr->getData();

    size3_t dim = vr->getDimensions();
    vec3 spacing(1.f);
    mat3 basis = data->getBasis();
    spacing.x = basis[0][0] / dim.x;
    spacing.y = basis[1][1] / dim.y;
    spacing.z = basis[2][2] / dim.z;

    unsigned char* data2Ptr = nullptr;
    if (components == 2) {
        size_t size = dim.x * dim.y * dim.z;
        data2Ptr = new unsigned char[size * components];
        size_t bytes = format->getSizeInBytes();
        memcpy(data2Ptr, dataPtr, size * bytes);
        swapbytes(data2Ptr, static_cast<unsigned int>(size * bytes));
        dataPtr = (const unsigned char*)data2Ptr;
    }

    unsigned char* description = nullptr;
    if (auto descMetaData = data->getMetaData<StringMetaData>("description")) {
        description = new unsigned char[descMetaData->get().size() + 1];
        strncpy((char*)description, descMetaData->get().c_str(), descMetaData->get().size());
        description[descMetaData->get().size()] = '\0';
    }

    unsigned char* courtesy = nullptr;
    if (auto courMetaData = data->getMetaData<StringMetaData>("courtesy")) {
        courtesy = new unsigned char[courMetaData->get().size() + 1];
        strncpy((char*)courtesy, courMetaData->get().c_str(), courMetaData->get().size());
        courtesy[courMetaData->get().size()] = '\0';
    }

    unsigned char* parameter = nullptr;
    if (auto paraMetaData = data->getMetaData<StringMetaData>("parameter")) {
        parameter = new unsigned char[paraMetaData->get().size() + 1];
        strncpy((char*)parameter, paraMetaData->get().c_str(), paraMetaData->get().size());
        parameter[paraMetaData->get().size()] = '\0';
    }

    unsigned char* comment = nullptr;
    if (auto commMetaData = data->getMetaData<StringMetaData>("comment")) {
        comment = new unsigned char[commMetaData->get().size() + 1];
        strncpy((char*)comment, commMetaData->get().c_str(), commMetaData->get().size());
        comment[commMetaData->get().size()] = '\0';
    }

    writePVMvolume(filePath.string().c_str(), dataPtr, static_cast<unsigned int>(dim.x),
                   static_cast<unsigned int>(dim.y), static_cast<unsigned int>(dim.z), components,
                   spacing.x, spacing.y, spacing.z, description, courtesy, parameter, comment);

    delete[] data2Ptr;
    delete[] description;
    delete[] courtesy;
    delete[] parameter;
    delete[] comment;
}

}  // namespace inviwo
