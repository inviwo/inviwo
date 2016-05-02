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

#include <inviwo/core/util/observer.h>

namespace inviwo {

Observer::Observer(const Observer& rhs) {
    for (auto observable : rhs.observables_) addObservation(observable);
}

Observer::Observer(Observer&& rhs) {
    for (auto observable : rhs.observables_) addObservation(observable);
    rhs.removeObservations();
}

Observer& Observer::operator=(const Observer& that) {
    if (this != &that) {
        removeObservations();
        for (auto observable : that.observables_) addObservation(observable);
    }
    return *this;
}

Observer& Observer::operator=(Observer&& that) {
    if (this != &that) {
        removeObservations();
        for (auto observable : that.observables_) addObservation(observable);
        that.removeObservations();
    }
    return *this;
}

Observer::~Observer() { removeObservations(); }

void Observer::removeObservation(ObservableInterface* observable) {
    if (observables_.erase(observable) > 0) {
        observable->removeObserverInternal(this);
    }
}

void Observer::removeObservations() {
    for (auto o : observables_) o->removeObserverInternal(this);
    observables_.clear();
}

void Observer::addObservation(ObservableInterface* observed) {
    std::pair<ObservableSet::iterator, bool> inserted = observables_.insert(observed);
    if (inserted.second) observed->addObserverInternal(this);
}

void Observer::addObservationInternal(ObservableInterface* observed) {
    observables_.insert(observed);
}

void Observer::removeObservationInternal(ObservableInterface* observable) {
    observables_.erase(observable);
}

void ObservableInterface::addObservationHelper(Observer* observer) {
    observer->addObservationInternal(this);
}

void ObservableInterface::removeObservationHelper(Observer* observer) {
    observer->removeObservationInternal(this);
}

util::NotificationBlocker::NotificationBlocker(ObservableInterface& observable)
    : observable_(observable) {
    observable_.startBlockingNotifications();
}
util::NotificationBlocker::~NotificationBlocker() { observable_.stopBlockingNotifications(); }

}  // namespace
