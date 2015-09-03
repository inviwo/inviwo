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

#ifndef IVW_VOLUMESOURCE_H
#define IVW_VOLUMESOURCE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

class VolumeBasisProperty : public CompositeProperty {
public:
    VolumeBasisProperty(std::string identifier, std::string displayName,
                        InvalidationLevel invalidationLevel = INVALID_RESOURCES,
                        PropertySemantics semantics = PropertySemantics::Default);
    VolumeBasisProperty(const VolumeBasisProperty& rhs);
    VolumeBasisProperty& operator=(const VolumeBasisProperty& that);

    void updateForNewVolume(const Volume& volume, bool deserialize = false);

    void updateVolume(Volume& volume);

    BoolProperty overRideDefaults_;
    FloatVec3Property a_;
    FloatVec3Property b_;
    FloatVec3Property c_;
    FloatVec3Property offset_;

    FloatVec3Property overrideA_;
    FloatVec3Property overrideB_;
    FloatVec3Property overrideC_;
    FloatVec3Property overrideOffset_;

private:
    void onOverrideChange();
};

class VolumeInformationProperty : public CompositeProperty {
public:
    VolumeInformationProperty(std::string identifier, std::string displayName,
                              InvalidationLevel invalidationLevel = INVALID_RESOURCES,
                              PropertySemantics semantics = PropertySemantics::Default);
    VolumeInformationProperty(const VolumeInformationProperty& rhs);
    VolumeInformationProperty& operator=(const VolumeInformationProperty& that);

    
    void updateForNewVolume(const Volume& volume, bool deserialize = false);
    void updateVolume(Volume& volume);
    
    // Read only used to show information
    StringProperty dimensions_;
    StringProperty format_;

    // read / write
    DoubleMinMaxProperty dataRange_;
    DoubleMinMaxProperty valueRange_;
    StringProperty valueUnit_;

};

class SequenceTimerProperty : public CompositeProperty {
public:
    SequenceTimerProperty(std::string identifier, std::string displayName,
                          InvalidationLevel invalidationLevel = INVALID_RESOURCES,
                          PropertySemantics semantics = PropertySemantics::Default);
    SequenceTimerProperty(const SequenceTimerProperty& rhs);
    SequenceTimerProperty& operator=(const SequenceTimerProperty& that);

    void updateMax(size_t max);

    IntProperty selectedSequenceIndex_;
    BoolProperty playSequence_;
    IntProperty volumesPerSecond_;
    Timer sequenceTimer_;

private:
    void onTimerEvent();
    void onPlaySequenceToggled();
};

/** \docpage{org.inviwo.VolumeSource, Volume Source}
 * ![](org.inviwo.VolumeSource.png?classIdentifier=org.inviwo.VolumeSource)
 *
 * Loads a Volume
 *
 * ### Outports
 *   * __Outport__ The loaded volume
 *
 * ### Properties
 *   * __File name__ File to load.
 */
class IVW_MODULE_BASE_API VolumeSource : public Processor {
public:
    using VolumeVector = std::vector<std::unique_ptr<Volume>>;
    VolumeSource();
    virtual ~VolumeSource();

    InviwoProcessorInfo();

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;


protected:
    virtual void process() override;

private:
    void load(bool deserialize = false);
    void addFileNameFilters();

    std::unique_ptr<VolumeVector> volumes_;

    VolumeOutport outport_;
    FileProperty file_;
    ButtonProperty reload_;

    VolumeBasisProperty basis_;
    VolumeInformationProperty information_;
    SequenceTimerProperty volumeSequence_;
    bool isDeserializing_;
};

}  // namespace

#endif  // IVW_VOLUMESOURCE_H
