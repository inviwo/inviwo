/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_ORDINALPROPERTYANIMATOR_H
#define IVW_ORDINALPROPERTYANIMATOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.OrdinalPropertyAnimator, Property Animator}
 * ![](org.inviwo.OrdinalPropertyAnimator.png?classIdentifier=org.inviwo.OrdinalPropertyAnimator)
 *
 * ...
 * 
 * 
 * 
 * ### Properties
 *   * __Property__ ...
 *   * __Active__ ...
 *   * __Delay (ms)__ ...
 *   * __Periodic__ ...
 *
 */
class IVW_MODULE_BASE_API OrdinalPropertyAnimator : public Processor {
public:
    OrdinalPropertyAnimator();
    virtual ~OrdinalPropertyAnimator();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

protected:
    virtual void process();

    void updateTimerInterval();
    void timerEvent();
    void changeProperty();
    void changeActive();

private:
    struct BaseProp {
        BaseProp(std::string classname, std::string displayName)
            : classname_(classname), displayName_(displayName) {}
        virtual ~BaseProp() {}

        virtual Property* getProp() = 0;
        virtual Property* getDelta() = 0;
        virtual void update(bool pbc) = 0;

        std::string classname_;
        std::string displayName_;
    };

    template <typename T>
    struct VecProp : public BaseProp {
        VecProp(std::string classname, std::string displayName);
        virtual ~VecProp();

        void setLimits();

        virtual Property* getProp() override;
        virtual Property* getDelta() override;
        virtual void update(bool pbc) override;

        OrdinalProperty<T>* prop_;
        OrdinalProperty<T>* delta_;
        typedef T valueType;
    };

    template <typename T>
    struct PrimProp : public BaseProp {
        PrimProp(std::string classname, std::string displayName);
        virtual ~PrimProp();

        void setLimits();

        virtual Property* getProp() override;
        virtual Property* getDelta() override;
        virtual void update(bool pbc) override;

        OrdinalProperty<T>* prop_;
        OrdinalProperty<T>* delta_;
        typedef T valueType;
    };

    OptionPropertyInt type_;
    IntProperty delay_;
    BoolProperty pbc_;
    BoolProperty active_;
    Timer* timer_;

    std::vector<BaseProp*> properties_;
};

template <typename T>
void OrdinalPropertyAnimator::VecProp<T>::update(bool pbc) {
    T p = prop_->get();
    T d = delta_->get();
    T r = p + d;
    for (glm::length_t i = 0; i < static_cast<glm::length_t>(prop_->getDim().x); ++i) {
        if (r[i] > prop_->getMaxValue()[i]) {
            if (pbc) {
                r[i] = r[i] - (prop_->getMaxValue()[i] - prop_->getMinValue()[i]);
            } else {
                r[i] = prop_->getMaxValue()[i];
            }
        } else if (r[i] < prop_->getMinValue()[i]) {
            if (pbc) {
                r[i] = r[i] + (prop_->getMaxValue()[i] - prop_->getMinValue()[i]);
            } else {
                r[i] = prop_->getMinValue()[i];
            }
        }
    }
    if (r != p) {
        prop_->set(r);
    }
}

template <typename T>
Property* OrdinalPropertyAnimator::VecProp<T>::getDelta() {
    return delta_;
}

template <typename T>
Property* OrdinalPropertyAnimator::VecProp<T>::getProp() {
    return prop_;
}

template <typename T>
OrdinalPropertyAnimator::VecProp<T>::~VecProp() {
    delete prop_;
    delete delta_;
}

template <typename T>
void OrdinalPropertyAnimator::VecProp<T>::setLimits() {
    T max = prop_->getMaxValue();

    T dmin = delta_->getMinValue();
    T dmax = delta_->getMaxValue();

    T newMin = -T(static_cast<typename T::value_type>(0.1)) * max;
    T newMax = T(static_cast<typename T::value_type>(0.1)) * max;

    if (dmin != newMin) {
        delta_->setMinValue(newMin);
    }
    if (dmax != newMax) {
        delta_->setMaxValue(newMax);
    }
}

template <typename T>
OrdinalPropertyAnimator::VecProp<T>::VecProp(std::string classname, std::string displayName)
    : BaseProp(classname, displayName), prop_(nullptr), delta_(nullptr) {
    prop_ = dynamic_cast<OrdinalProperty<T>*>(
        PropertyFactory::getPtr()->getProperty(classname, classname, displayName));

    std::stringstream ss1;
    ss1 << classname << "-"
        << "Delta";
    std::string identifier = ss1.str();

    delta_ = dynamic_cast<OrdinalProperty<T>*>(
        PropertyFactory::getPtr()->getProperty(classname, identifier, "Delta"));

    prop_->onChange(this, &VecProp<T>::setLimits);
}

template <typename T>
void OrdinalPropertyAnimator::PrimProp<T>::update(bool pbc) {
    T p = prop_->get();
    T d = delta_->get();
    T r = p + d;

    if (r > prop_->getMaxValue()) {
        if (pbc) {
            r = r - (prop_->getMaxValue() - prop_->getMinValue());
        } else {
            r = prop_->getMaxValue();
        }
    } else if (r < prop_->getMinValue()) {
        if (pbc) {
            r = r + (prop_->getMaxValue() - prop_->getMinValue());
        } else {
            r = prop_->getMinValue();
        }
    }

    if (r != p) {
        prop_->set(r);
    }
}

template <typename T>
Property* OrdinalPropertyAnimator::PrimProp<T>::getDelta() {
    return delta_;
}

template <typename T>
Property* OrdinalPropertyAnimator::PrimProp<T>::getProp() {
    return prop_;
}

template <typename T>
OrdinalPropertyAnimator::PrimProp<T>::~PrimProp() {
    delete prop_;
    delete delta_;
}

template <typename T>
void OrdinalPropertyAnimator::PrimProp<T>::setLimits() {
    T max = prop_->getMaxValue();

    T dmin = delta_->getMinValue();
    T dmax = delta_->getMaxValue();

    if (dmin != -max / 10) {
        delta_->setMinValue(-max / 10);
    }
    if (dmax != max / 10) {
        delta_->setMaxValue(max / 10);
    }
}

template <typename T>
OrdinalPropertyAnimator::PrimProp<T>::PrimProp(std::string classname, std::string displayName)
    : BaseProp(classname, displayName), prop_(nullptr), delta_(nullptr) {
    prop_ = dynamic_cast<OrdinalProperty<T>*>(
        PropertyFactory::getPtr()->getProperty(classname, classname, displayName));

    std::stringstream ss1;
    ss1 << classname << "-"
        << "Delta";
    std::string identifier = ss1.str();

    delta_ = dynamic_cast<OrdinalProperty<T>*>(
        PropertyFactory::getPtr()->getProperty(classname, identifier, "Delta"));

    prop_->onChange(this, &PrimProp<T>::setLimits);
}

}  // namespace

#endif  // IVW_ORDINALPROPERTYANIMATOR_H
