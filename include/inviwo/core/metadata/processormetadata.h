/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORMETADATA_H
#define IVW_PROCESSORMETADATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/positionmetadata.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class IVW_CORE_API ProcessorMetaDataObserver : public Observer {
public:
    virtual void onProcessorMetaDataPositionChange() {};
    virtual void onProcessorMetaDataVisibilityChange() {};
    virtual void onProcessorMetaDataSelectionChange() {};
};

class IVW_CORE_API ProcessorMetaData : public MetaData, public Observable<ProcessorMetaDataObserver>{
public:
    ProcessorMetaData();
    ProcessorMetaData(const ProcessorMetaData& rhs);
    ProcessorMetaData& operator=(const ProcessorMetaData& that);
    virtual ~ProcessorMetaData();

    virtual std::string getClassIdentifier() const { return CLASS_IDENTIFIER; }
    virtual ProcessorMetaData* clone() const;

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);
    virtual bool equal(const MetaData& rhs) const;

    void setPosition(const ivec2& pos);
    ivec2 getPosition() const;
    void setVisibile(bool visibility);
    bool isVisible() const;
    void setSelected(bool selection);
    bool isSelected() const;

    static const std::string CLASS_IDENTIFIER;

private:
    ivec2 position_;
    bool visibility_;
    bool selection_;
};

}  // namespace

#endif  // IVW_PROCESSORMETADATA_H
