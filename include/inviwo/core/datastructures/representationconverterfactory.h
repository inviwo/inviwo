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

#ifndef IVW_REPRESENTATIONCONVERTERFACTORY_H
#define IVW_REPRESENTATIONCONVERTERFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/representationconverter.h>

#include <warn/push>
#include <warn/ignore/all>
#include <memory>
#include <mutex>
#include <warn/pop>

namespace inviwo {

class IVW_CORE_API RepresentationConverterFactory {
public:
    using ConverterID = RepresentationConverter::ConverterID;
    using RepMap = std::unordered_map<ConverterID, RepresentationConverter*>;
    using PackageMap =
        std::unordered_multimap<ConverterID, std::unique_ptr<RepresentationConverterPackage>>;
    RepresentationConverterFactory() = default;
    virtual ~RepresentationConverterFactory() = default;

    // This will not assume ownership.
    bool registerObject(RepresentationConverter* representationConverter);
    bool unRegisterObject(RepresentationConverter* representationConverter);

    // Get best converter
    const RepresentationConverterPackage* getRepresentationConverter(ConverterID);
    const RepresentationConverterPackage* getRepresentationConverter(std::type_index from,
                                                                     std::type_index to);

private:
    const RepresentationConverterPackage* createConverterPackage(ConverterID id);

    // converters are owned by the Module
    RepMap converters_;

    // All the converter packages created locally;
    std::mutex mutex_;
    PackageMap packages_;
};

}  // namespace

#endif  // IVW_REPRESENTATIONCONVERTERFACTORY_H
