/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_PORTINSPECTORMANAGER_H
#define IVW_PORTINSPECTORMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <map>

namespace inviwo {

class Outport;
class InviwoApplication;
class Image;
class ProcessorWidget;
class PortInspector;
class ProcessorNetwork;

/**
 * \class PortInspectorManager
 * \brief Keep track of which port has port inspectors on them
 */
class IVW_CORE_API PortInspectorManager : public Serializable, public ProcessorNetworkObserver {
public:
    PortInspectorManager(InviwoApplication* app);
    virtual ~PortInspectorManager();

    PortInspectorManager(const PortInspectorManager&) = delete;
    PortInspectorManager(PortInspectorManager&&) = default;
    PortInspectorManager& operator=(const PortInspectorManager& that) = delete;
    PortInspectorManager& operator=(PortInspectorManager&& that) = default;

    bool isPortInspectorSupported(const Outport* outport);

    bool hasPortInspector(Outport* outport) const;
    ProcessorWidget* addPortInspector(Outport* outport, ivec2 pos);
    void removePortInspector(Outport* outport);

    std::shared_ptr<const Image> renderPortInspectorImage(Outport* outport);

    void clear();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    using PortInspectorMap = std::map<std::string, std::unique_ptr<PortInspector>>;

    static std::string getPortId(Outport* outport);
    PortInspector* borrow(Outport* outport);
    std::unique_ptr<PortInspector> getPortInspector(Outport* outport);
    void returnPortInspector(std::unique_ptr<PortInspector>);

    static void insertNetwork(PortInspector* portInspector, ProcessorNetwork* network,
                              Outport* outport, bool bidirectionalAutoLinks);
    static void removeNetwork(PortInspector* portInspector, ProcessorNetwork* network);

    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor) override;

    void removePortInspector(PortInspectorMap::iterator it);

    PortInspectorMap portInspectors_;
    std::vector<std::unique_ptr<PortInspector>> unUsedInspectors_;

    InviwoApplication* app_;
};

}  // namespace inviwo

#endif  // IVW_PORTINSPECTORMANAGER_H
