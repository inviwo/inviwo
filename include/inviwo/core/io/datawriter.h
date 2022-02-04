/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/exception.h>

#include <vector>
#include <iostream>

namespace inviwo {

enum class Overwrite { No, Yes };

/**
 * \ingroup dataio
 */
class IVW_CORE_API DataWriter {
public:
    DataWriter();
    DataWriter(const DataWriter& rhs);
    DataWriter& operator=(const DataWriter& that);
    virtual DataWriter* clone() const = 0;
    virtual ~DataWriter() = default;

    const std::vector<FileExtension>& getExtensions() const;
    void addExtension(FileExtension ext);

    Overwrite getOverwrite() const;
    void setOverwrite(Overwrite val);

    /**
     * Verify that you don't overwrite @p path unless @p overwrite is `Yes`.
     * @throws DataWriterException if the condition is broken.
     */
    static void checkOverwrite(std::string_view path, Overwrite overwrite);

    /**
     * Verify that you don't overwrite @p path unless @p overwrite is `Yes`.
     * @throws DataWriterException if the condition is broken.
     */
    void checkOverwrite(std::string_view path) const;

protected:
    /**
     * Open @p path in @p mode for writing. If the overwrite condition is broken or the file can't
     * be opened an exception is thrown.
     * @throws DataWriterException if the condition is broken, and FileException if the file can't
     * be opened.
     */
    std::ofstream open(std::string_view path,
                       std::ios_base::openmode mode = std::ios_base::out) const;

    Overwrite overwrite_;
    std::vector<FileExtension> extensions_;
};

/**
 * \ingroup dataio
 */
template <typename T>
class DataWriterType : public DataWriter {
public:
    DataWriterType() = default;
    DataWriterType(const DataWriterType& rhs) = default;
    DataWriterType& operator=(const DataWriterType& that) = default;
    virtual DataWriterType* clone() const = 0;
    virtual ~DataWriterType() = default;

    /**
     * @brief Write @p data to @p filePath
     * @throws DataWriterException if anything goes wrong
     */
    virtual void writeData(const T* data, std::string_view filePath) const = 0;

    virtual std::unique_ptr<std::vector<unsigned char>> writeDataToBuffer(
        const T* /*data*/, std::string_view /*fileExtension*/) const {
        return nullptr;
    }
};

}  // namespace inviwo
