/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/assimp/assimpmoduledefine.h>  // for IVW_MODULE_ASSIMP_API

#include <inviwo/core/datastructures/geometry/mesh.h>  // for DataReaderType
#include <inviwo/core/io/datareader.h>                 // for DataReaderType

#include <any>          // for any
#include <memory>       // for shared_ptr
#include <string_view>  // for string_view

namespace inviwo {

enum class AssimpLogLevel : int { None, Error, Warn, Info, Debug };  // increased verbosity

/**
 * \ingroup dataio
 * \brief Inviwo Module Assimp
 *
 *  A GeometryReader (DataReaderType<Geometry>) using the Assimp Library.
 */
class IVW_MODULE_ASSIMP_API AssimpReader : public DataReaderType<Mesh> {
public:
    AssimpReader();
    AssimpReader(const AssimpReader& rhs) = default;
    AssimpReader& operator=(const AssimpReader& that) = default;
    virtual AssimpReader* clone() const override;
    virtual ~AssimpReader() = default;

    void setLogLevel(AssimpLogLevel level, bool verbose = false);
    AssimpLogLevel getLogLevel() const;

    void setFixInvalidDataFlag(bool enable);
    bool getFixInvalidDataFlag() const;

    virtual std::shared_ptr<Mesh> readData(const std::filesystem::path& filePath) override;

    virtual bool setOption(std::string_view key, std::any value) override;
    virtual std::any getOption(std::string_view key) override;

private:
    AssimpLogLevel
        logLevel_;  //!< determines the verbosity of the logging during data import (default = Warn)
    bool verboseLog_;
    bool fixInvalidData_;  //!< if true, the imported data will be checked for invalid data, e.g.
                           //!< invalid normals or UV coords, which might be fixed or removed by
                           //!< Assimp
};

}  // namespace inviwo
