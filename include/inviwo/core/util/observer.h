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

#ifndef IVW_OBSERVER_H
#define IVW_OBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <unordered_set>
#include <functional>
#include <algorithm>

namespace inviwo {

class ObservableInterface;

/** \class Observer
 * Class to support observer pattern. An example of usage is given in the Observable class.
 * @see Observable
 */
class IVW_CORE_API Observer {
    friend class ObservableInterface;

public:
    Observer() = default;
    Observer(const Observer& other);
    Observer(Observer&& other);
    Observer& operator=(Observer&& other);
    Observer& operator=(const Observer& other);

    /**
     * Removes the observer from all observable objects. Makes sure that it cannot
     * be called when destroyed.
     */
    virtual ~Observer();

    /**
     * Stop observing object by removing it from observation list.
     * @param observable (ObservableInterface *) The observable to stop observing.
     */
    void removeObservation(ObservableInterface* observable);

    // Stops observing all objects by removing them from observation list.
    void removeObservations();

protected:
    /**
     * Add an object to observe.
     * @param observable The observable to add.
     */
    void addObservation(ObservableInterface* observable);

    // Storing observables connected to this observer enables removal upon destruction.
    typedef std::unordered_set<ObservableInterface*> ObservableSet;
    ObservableSet observables_;

private:
    // The internal ones will not call observable->add/remove
    // Should only be called by ObservableInterface.
    void addObservationInternal(ObservableInterface* observable);
    void removeObservationInternal(ObservableInterface* observable);
};

/** \class ObservableInterface
 * Class to support observer pattern.
 * This is an interface only, inherit from Observable<DerivedObserver> to define your
 * own "notify()" method.
 * An example of usage is given in the Observable class.
 * @see Observable
 * @see Observer
 */
class IVW_CORE_API ObservableInterface {
    friend class Observer;
public:
    virtual ~ObservableInterface() = default;

protected:
    virtual void addObserver(Observer* observer) = 0;
    virtual void removeObserver(Observer* observer) = 0;
    virtual void removeObservers() = 0;

    // Helper functions in base class since derived template
    // classes aren't friends with Observer.
    void addObservationHelper(Observer* observer);
    void removeObservationHelper(Observer* observer);

private:
    // The internal ones will not call observer->add/remove
    // Should only be called by Observer.
    virtual void addObserverInternal(Observer* observer) = 0;
    virtual void removeObserverInternal(Observer* observer) = 0;
};

/** \class Observable
 *
 * Class to support observer pattern.
 *
 * \section Observable.example Example
 * Example of how to apply it to a simple button.
 * @code
 *    class IVW_XXX_API ButtonObserver: public Observer {
 *    public:
 *        ButtonObserver(): Observer() {};
 *        // Will be notified when the observed button is pressed.
 *        void buttonPressed(){};
 *    };
 *
 *    class IVW_XXX_API Button: public Observable<ButtonObserver> {
 *    public:
 *        Button(): Observable<ButtonObserver>() {};
 *        void pressButton() {
 *            // Do stuff
 *            // Notify observers
 *            for (auto o : observers_) o->buttonPressed();
 *        }
 *    };
 * @endcode
 * @see Observer
 * @see VoidObserver
 */
template <typename T>
class Observable : public ObservableInterface {
public:
    Observable() = default;
    Observable(const Observable<T>& other);
    Observable(Observable<T>&& other);
    Observable<T>& operator=(Observable<T>&& other);
    Observable<T>& operator=(const Observable<T>& other);
    virtual ~Observable();

    void addObserver(T* observer);
    void removeObserver(T* observer);

protected:
    typedef std::unordered_set<T*> ObserverSet;
    ObserverSet observers_;

private:
    virtual void addObserver(Observer* observer) override;
    virtual void removeObserver(Observer* observer) override;
    virtual void removeObservers() override;
    virtual void addObserverInternal(Observer* observer) override;
    virtual void removeObserverInternal(Observer* observer) override;
};

template <typename T>
Observable<T>::Observable(const Observable<T>& rhs) {
    for (auto elem : rhs.observers_) addObserver(elem);
}

template <typename T>
Observable<T>::Observable(Observable<T>&& rhs) {
    for (auto elem : rhs.observers_) addObserver(elem);
    rhs.removeObservers();
}

template <typename T>
Observable<T>& inviwo::Observable<T>::operator=(const Observable<T>& that) {
    if (this != &that) {
        removeObservers();
        for (auto elem : that.observers_) addObserver(elem);
    }
    return *this;
}

template <typename T>
Observable<T>& inviwo::Observable<T>::operator=(Observable<T>&& that) {
    if (this != &that) {
        removeObservers();
        for (auto elem : that.observers_) addObserver(elem);
        that.removeObservers();
    }
    return *this;
}

template <typename T>
Observable<T>::~Observable() {
    removeObservers();
}

template <typename T>
void Observable<T>::removeObservers() {
    for (auto o : observers_) removeObservationHelper(o);
    observers_.clear();
}

template <typename T>
void Observable<T>::addObserver(T* observer) {
    auto inserted = observers_.insert(observer);
    if (inserted.second) addObservationHelper(observer);
}

template <typename T>
void Observable<T>::removeObserver(T* observer) {
    if (observers_.erase(observer) > 0) {
        removeObservationHelper(observer);
    }
}

template <typename T>
void Observable<T>::addObserver(Observer* observer) {
    addObserver(static_cast<T*>(observer));
}

template <typename T>
void Observable<T>::removeObserver(Observer* observer) {
    removeObserver(static_cast<T*>(observer));
}

template <typename T>
void inviwo::Observable<T>::addObserverInternal(Observer* observer) {
    observers_.insert(static_cast<T*>(observer));
}

template <typename T>
void inviwo::Observable<T>::removeObserverInternal(Observer* observer) {
    observers_.erase(static_cast<T*>(observer));
}

}  // namespace

#endif  // IVW_OBSERVER_H
