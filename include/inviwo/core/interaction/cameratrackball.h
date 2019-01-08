/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_CAMERATRACKBALL_H
#define IVW_CAMERATRACKBALL_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

class IVW_CORE_API CameraTrackball : public Trackball {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     * Convenience clas for trackball using camera properties. Calls propertyModified() on camera
     * when any of the positions or vectors change.
     * Rotates and moves the camera around a sphere.
     * This object does not take ownership of pointers handed to it.
     * @see Trackball
     * @param cameraProp Pointer to the camera that will be controlled.
     */
    CameraTrackball(CameraProperty* cameraProp);
    virtual ~CameraTrackball();
};

}  // namespace inviwo

#endif  // IVW_CAMERATRACKBALL_H