/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/geometry/mesh.h>  // for DataWriterType
#include <inviwo/core/io/datawriter.h>                 // for DataWriterType

#include <memory>       // for unique_ptr
#include <ostream>      // for ostream
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

/**
 * \class BinarySTLWriter
 * \brief Export Meshes in the binary STL format
 */
class IVW_MODULE_BASE_API BinarySTLWriter : public DataWriterType<Mesh> {
public:
    BinarySTLWriter();
    BinarySTLWriter(const BinarySTLWriter&) = default;
    BinarySTLWriter& operator=(const BinarySTLWriter&) = default;
    virtual BinarySTLWriter* clone() const override;
    virtual ~BinarySTLWriter() = default;

    virtual void writeData(const Mesh* data, const std::filesystem::path& filePath) const override;
    virtual std::unique_ptr<std::vector<unsigned char>> writeDataToBuffer(
        const Mesh* data, std::string_view fileExtension) const override;

private:
    void writeData(const Mesh* data, std::ostream& os) const;
};

}  // namespace inviwo
