/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/tetramesh/tetrameshmoduledefine.h>

#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/util/document.h>

namespace inviwo {

class TetraMeshProvider;

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_TETRAMESH_API TetraMeshData {
public:
    TetraMeshData(const std::shared_ptr<TetraMeshProvider>& meshprovider);
    virtual TetraMeshData* clone() const;
    virtual ~TetraMeshData() = default;

    int getNumberOfCells() const;
    int getNumberOfPoints() const;

    operator TetraMeshProvider*() const { return &(*meshProvider_); }

    std::shared_ptr<TetraMeshProvider> getProvider() const { return meshProvider_; }

private:
    std::shared_ptr<TetraMeshProvider> meshProvider_;
};

template <>
struct DataTraits<TetraMeshData> {
    static std::string classIdentifier() { return "org.inviwo.tetra.TetraMeshData"; }
    static std::string dataName() { return "TetraMeshData"; }
    static uvec3 colorCode() { return uvec3{/*65, 153, 211*/ 50, 161, 234}; }

    static Document info(const TetraMeshData& data) {
        using P = Document::PathComponent;
        using H = utildoc::TableBuilder::Header;
        Document doc;
        doc.append("b", "TetraMesh Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());

        tb(H("Tetras"), data.getNumberOfCells());
        tb(H("Points"), data.getNumberOfPoints());

        // auto osr = data.getSharedPointer();
        // int levels = osr->getLevelCount();
        // tb(H("Level Count"), levels);
        // for (int i = 0; i < levels; ++i) {
        //     glm::i64vec2 dim = osr->getLevelDimensions(i);
        //     tb(H("Level " + toString(i)), "(" + toString(dim.x) + "x" + toString(dim.y) + ")");
        // }

        return doc;
    }
};

}  // namespace inviwo
