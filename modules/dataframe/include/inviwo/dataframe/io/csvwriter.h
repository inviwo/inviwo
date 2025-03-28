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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/io/datawriter.h>  // for DataWriterType

#include <iosfwd>       // for ostream
#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class DataFrame;

/**
 * \brief DataFrame CSV Writer
 */
class IVW_MODULE_DATAFRAME_API CSVWriter : public DataWriterType<DataFrame> {
public:
public:
    CSVWriter();
    CSVWriter(const CSVWriter&) = default;
    CSVWriter& operator=(const CSVWriter&) = default;
    virtual CSVWriter* clone() const override;
    virtual ~CSVWriter() = default;

    virtual void writeData(const DataFrame* data,
                           const std::filesystem::path& filePath) const override;
    virtual std::unique_ptr<std::vector<unsigned char>> writeDataToBuffer(
        const DataFrame* data, std::string_view fileExtension) const override;

    std::string delimiter = ",";
    bool quoteStrings = true;
    bool exportIndexCol = false;
    bool separateVectorTypesIntoColumns = true;

    void writeData(const DataFrame* data, std::ostream& os) const;
};

}  // namespace inviwo
