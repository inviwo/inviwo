/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORNETWORKCONVERTER_H
#define IVW_PROCESSORNETWORKCONVERTER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/versionconverter.h>

namespace inviwo {

/**
 * \class ProcessorNetworkConverter
 * \brief A utitlity to handle conversion of ProcessorNetwork versions.
 */
class IVW_CORE_API ProcessorNetworkConverter : public VersionConverter {
    public:
        typedef void (ProcessorNetworkConverter::*updateType)(TxElement*);
        ProcessorNetworkConverter(int from);
        virtual bool convert(TxElement* root);
        int from_;
    private:
        void updateProcessorType(TxElement* node);
        void updateMetaDataTree(TxElement* node);
        void updatePropertType(TxElement* node);
        void updateMetaDataType(TxElement* node);
        void updateMetaDataKeys(TxElement* node);
        void updateShadingMode(TxElement* node);
        void updateCameraToComposite(TxElement* node);
        void updateDimensionTag(TxElement* node);
        void updatePropertyLinks(TxElement* node);
        void updatePortsInProcessors(TxElement* node);
        void updateNoSpaceInProcessorClassIdentifers(TxElement* node);
        void updateDisplayName(TxElement* node);

        void traverseNodes(TxElement* node, updateType update);
    };

} // namespace

#endif // IVW_PROCESSORNETWORKCONVERTER_H

