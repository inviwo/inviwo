/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/io/serialization/serializable.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <warn/pop>

#include <vector>

namespace inviwo {

class IVW_QTEDITOR_API WorkspacePreview : public Serializable {
public:
    using ImageVector = std::vector<std::pair<std::string, QImage>>;

    struct Item : public Serializable {
        Item() = default;
        Item(std::string name, std::string base64img, int w, int h);
        virtual ~Item() = default;

        bool isValid() const;

        virtual void serialize(Serializer &s) const override;
        virtual void deserialize(Deserializer &d) override;

        std::string name;
        std::string base64img;
        ivec2 size = ivec2{0};
    };

    WorkspacePreview() = default;
    WorkspacePreview(const QImage &network, const ImageVector &canvases);
    virtual ~WorkspacePreview() = default;

    virtual void serialize(Serializer &s) const override;
    virtual void deserialize(Deserializer &d) override;

    const Item &getNetworkImage() const;
    const std::vector<Item> getCanvases() const;

private:
    Item network_;
    std::vector<Item> canvases_;
};

}  // namespace inviwo
