/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/positionmetadata.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class IVW_CORE_API ProcessorMetaDataObserver : public Observer {
public:
    virtual void onProcessorMetaDataPositionChange(){};
    virtual void onProcessorMetaDataVisibilityChange(){};
    virtual void onProcessorMetaDataSelectionChange(){};
};

class IVW_CORE_API ProcessorMetaData : public MetaData,
                                       public Observable<ProcessorMetaDataObserver> {
public:
    ProcessorMetaData();
    ProcessorMetaData(const ProcessorMetaData& rhs) = default;
    ProcessorMetaData& operator=(const ProcessorMetaData& that) = default;
    virtual ~ProcessorMetaData() = default;

    virtual std::string_view getClassIdentifier() const override { return classIdentifier; }
    virtual ProcessorMetaData* clone() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    virtual bool equal(const MetaData& rhs) const override;

    void setPosition(const ivec2& pos);
    ivec2 getPosition() const;
    void setVisible(bool visibility);
    bool isVisible() const;
    void setSelected(bool selection);
    bool isSelected() const;

    static constexpr std::string_view classIdentifier{"org.inviwo.ProcessorMetaData"};

private:
    ivec2 position_;
    bool visibility_;
    bool selection_;
};

}  // namespace inviwo
