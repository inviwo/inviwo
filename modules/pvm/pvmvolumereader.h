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

#ifndef IVW_PVMVOLUMEREADER_H
#define IVW_PVMVOLUMEREADER_H

#include <modules/pvm/pvmmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

/** \brief Reader for *.pvm files
 *
 *  Format designed by Stefan Roettger
 */
 class IVW_MODULE_PVM_API PVMVolumeReader : public DataReaderType<Volume> {
public:
    PVMVolumeReader();
    PVMVolumeReader(const PVMVolumeReader& rhs) = default;
    PVMVolumeReader& operator=(const PVMVolumeReader& that) = default;
    virtual PVMVolumeReader* clone() const override;
    virtual ~PVMVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(const std::string filePath) override;
    static std::shared_ptr<Volume> readPVMData(std::string filePath);

 protected:
     void printMetaInfo(const MetaDataOwner&, std::string) const;

};

}  // namespace

#endif  // IVW_PVMVOLUMEREADER_H
