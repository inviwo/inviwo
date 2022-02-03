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
#include <inviwo/core/util/fileextension.h>

#include <vector>
#include <memory>
#include <any>
#include <iostream>

namespace inviwo {

class MetaDataOwner;

/**
 * \defgroup dataio Data Reader & Writers
 */

/**
 * \ingroup dataio
 * \brief A abstract base class for all file readers.
 */
class IVW_CORE_API DataReader {
public:
    DataReader() = default;
    DataReader(const DataReader& rhs) = default;
    DataReader(DataReader&& rhs) noexcept = default;
    DataReader& operator=(const DataReader& that) = default;
    DataReader& operator=(DataReader&& that) noexcept = default;

    virtual DataReader* clone() const = 0;
    virtual ~DataReader() = default;

    const std::vector<FileExtension>& getExtensions() const;
    void addExtension(FileExtension ext);

    /**
     * @brief Set reader specific options
     * See the documentation of the specific reader about which options that are available
     * @param key the option to set
     * @param value the new value for the option
     * @return true of the option was recognized and set, otherwise false
     */
    virtual bool setOption([[maybe_unused]] std::string_view key, [[maybe_unused]] std::any value) {
        return false;
    }

    /**
     * @brief Query the value of an reader specific option
     * @param key the option to query
     * @return an std::any with the requested option of an empty std::any of the option was not
     * found
     */
    virtual std::any getOption([[maybe_unused]] std::string_view key) { return std::any{}; }

protected:
    /**
     * Verify that @p path exists, and throw DataReaderException if not.
     * @throws DataReaderException if the file is not found
     */
    void checkExists(std::string_view path) const;

    /**
     * Open @p path in @p mode for reading. If the file is not found or the file can't
     * be opened an exception is thrown.
     * @throws DataReaderException if the file is not found, and FileException if the file can't
     * be opened.
     */
    std::ifstream open(std::string_view path,
                       std::ios_base::openmode mode = std::ios_base::in) const;

    std::vector<FileExtension> extensions_;
};

/**
 * \ingroup dataio
 *\brief Template base class for file readers designating what type of data
 * object the reader returns.
 */
template <typename T>
class DataReaderType : public DataReader {
public:
    DataReaderType() = default;
    DataReaderType(const DataReaderType& rhs) = default;
    DataReaderType(DataReaderType&& rhs) noexcept = default;
    DataReaderType& operator=(const DataReaderType& that) = default;
    DataReaderType& operator=(DataReaderType&& that) noexcept = default;
    virtual DataReaderType* clone() const override = 0;
    virtual ~DataReaderType() override = default;

    virtual std::shared_ptr<T> readData(std::string_view filePath) = 0;

    /**
     * Optional overload that passed a MetaDataOwner to facilitate saving/loading state in the data
     * reader the use is optional and the pointer can be null.
     * @see RawVolumeReader
     */
    virtual std::shared_ptr<T> readData(std::string_view filePath, MetaDataOwner*) {
        return readData(filePath);
    };
};

}  // namespace inviwo
