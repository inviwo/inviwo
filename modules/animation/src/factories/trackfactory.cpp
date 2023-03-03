/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/animation/factories/trackfactory.h>

#include <inviwo/core/properties/property.h>                 // for Property
#include <inviwo/core/util/exception.h>                      // for Exception
#include <inviwo/core/util/logcentral.h>                     // for LogCentral, LogWarn
#include <modules/animation/datastructures/propertytrack.h>  // for BasePropertyTrack
#include <modules/animation/datastructures/track.h>          // for Track
#include <modules/animation/factories/trackfactoryobject.h>  // for TrackFactoryObject

#include <ostream>  // for operator<<
#include <utility>  // for pair

namespace inviwo {
namespace animation {

TrackFactory::TrackFactory(ProcessorNetwork* network) : network_{network} {}

bool TrackFactory::hasKey(std::string_view key) const { return Parent::hasKey(key); }

std::unique_ptr<Track> TrackFactory::create(std::string_view key) const {
    return Parent::create(key, network_);
}

std::unique_ptr<Track> TrackFactory::create(Property* property) const {
    auto it = propertyToTrackMap_.find(property->getClassIdentifier());
    if (it != propertyToTrackMap_.end()) {
        if (auto track = create(it->second)) {
            if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track.get())) {
                try {
                    basePropertyTrack->setProperty(property);
                } catch (const Exception& e) {
                    LogWarn(e.getMessage() << " Invalid property class identified?") return nullptr;
                }

                return track;
            }
        }
    }
    return nullptr;
}

void TrackFactory::registerPropertyTrackConnection(std::string_view propertyClassID,
                                                   std::string_view trackClassID) {
    propertyToTrackMap_.emplace(propertyClassID, trackClassID);
}

}  // namespace animation
}  // namespace inviwo
