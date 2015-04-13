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
#include <inviwo/core/util/assertion.h>
#include <sstream>

namespace inviwo {

////////////////////////////// Observer ////////////////////////////////////////////

Observer::Observer() {
    observables_ = new ObservableSet();
}

Observer::Observer(const Observer& other) {
    observables_ = new ObservableSet();
    *this = other;
}


Observer& Observer::operator=(const Observer& other) {
    if (this != &other) {
        removeObservations();
        for (const auto& elem : *other.observables_) {
            addObservation(elem);
        }
    }
    return *this;
}

Observer::~Observer() {
    removeObservations();
    delete observables_;
}

void Observer::removeObservation(ObservableInterface* observable) {
    ObservableSet::iterator it = observables_->find(observable);

    // Remove from list and observed object if observing it
    if (it != observables_->end()) {
        observables_->erase(it);
        // Remove from observable
        observable->removeObserver(this);
    }
}

void Observer::removeObservations() {
    while (!observables_->empty())
        removeObservation(*observables_->begin());
}

void Observer::addObservation(ObservableInterface* observed) {
    ivwAssert(observed!=nullptr, "Tried to add null Observable");
    std::pair<ObservableSet::iterator, bool> inserted = observables_->insert(observed);

    if (inserted.second)
        observed->addObserver(this);
}

////////////////////////////// ObservableInterface ////////////////////////////////////////////

ObservableInterface::ObservableInterface() {
    observers_ = new ObserverSet();
}

ObservableInterface::ObservableInterface(const ObservableInterface& other) {
    observers_ = new ObserverSet();
    *this = other;
}

ObservableInterface& ObservableInterface::operator=(const ObservableInterface& other) {
    if (this != &other) {
        removeObservers();
        for (const auto& elem : *other.observers_) {
            addObserver(elem);
        }
    }
    return *this;
}

ObservableInterface::~ObservableInterface() {
    removeObservers();
    delete observers_;
}
void ObservableInterface::addObserver(Observer* observer) {
    ivwAssert(observer!=nullptr, "Tried to add null Observer");
    std::pair<ObserverSet::iterator, bool> inserted = observers_->insert(observer);

    if (inserted.second)
        observer->addObservation(this);
}

void ObservableInterface::removeObserver(Observer* observer) {
    ObserverSet::iterator it = observers_->find(observer);

    // Remove from list and observer if observed by it
    if (it != observers_->end()) {
        observers_->erase(it);
        // Remove from observer
        observer->removeObservation(this);
    }
}

void ObservableInterface::removeObservers() {
    while (!observers_->empty())
        removeObserver(*observers_->begin());
}


} // namespace

