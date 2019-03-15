/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TRACKWIDGETQTFACTORY_H
#define IVW_TRACKWIDGETQTFACTORY_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/factory.h>

#include <modules/animation/datastructures/track.h>

#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/animationqt/factories/trackwidgetqtfactoryobject.h>

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATIONQT_API TrackWidgetQtFactory
    : public StandardFactory<TrackWidgetQt, TrackWidgetQtFactoryObject, const std::string&,
                             Track&> {
public:
    TrackWidgetQtFactory() = default;
    virtual ~TrackWidgetQtFactory() = default;

    using StandardFactory<TrackWidgetQt, TrackWidgetQtFactoryObject, const std::string&,
                          Track&>::create;

    void registerTrackToWidgetMap(const std::string& trackId, const std::string& widgetId);

    std::string getWidgetId(const std::string& trackId) const;

private:
    std::unordered_map<std::string, std::string> trackToWidget_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_TRACKWIDGETQTFACTORY_H
