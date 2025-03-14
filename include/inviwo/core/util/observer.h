/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/raiiutils.h>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <vector>

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
    std::unordered_set<ObservableInterface*> observables_;

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

    virtual void startBlockingNotifications() = 0;
    virtual void stopBlockingNotifications() = 0;

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
    virtual bool addObserverInternal(Observer* observer) = 0;
    virtual bool removeObserverInternal(Observer* observer) = 0;
};

/** \class Observable
 *
 * Class to support observer pattern.
 *
 * Example of how to apply it to a simple button.
 * @code
 *     class ButtonObservable;
 *
 *     class IVW_XXX_API ButtonObserver: public Observer {
 *     public:
 *         friend ButtonObservable;
 *         ButtonObserver() = default
 *         // Override to be notified when the observed button is pressed.
 *         virtual void onButtonPressed(){};
 *     };
 *
 *     class IVW_XXX_API ButtonObservable: public Observable<ButtonObserver> {
 *     protected:
 *         ButtonObservable() = default;
 *         void notifyObserversAboutButtonPressed() {
 *             forEachObserver([](ButtonObserver* o) { o->onButtonPressed(); });
 *         }
 *     };
 * @endcode
 *
 * Usage:
 * @code
 *     class Button : public ButtonObservable {
 *         ...
 *         void handleButtonPress() {
 *             ...
 *             notifyObserversAboutButtonPressed();
 *             ...
 *         }
 *         ...
 *     };
 * @endcode
 *
 * @code
 *     class MyClass : public ButtonObserver {
 *     public:
 *         MyClass(Button* button) {
 *             button->addObserver(this);
 *         }
 *
 *         ...
 *     private:
 *         virtual void onButtonPressed() override {
 *             // Do stuff on button press
 *         };
 *         ...
 *     };
 * @endcode
 *
 * @see Observer
 * @see VoidObserver
 */
template <typename T>
class Observable : public ObservableInterface {
public:
    Observable() = default;

    /**
     * This operation does nothing. We will not touch the observers of other.
     */
    Observable(const Observable<T>& other);

    /**
     * This operation will remove all observers from other and add them to this.
     */
    Observable(Observable<T>&& other) noexcept;

    /**
     * This operation does nothing. We will not touch the observers of other.
     */
    Observable<T>& operator=(const Observable<T>& other);

    /**
     * This operation will remove all observers of this, and make all observers of other observe
     * this instead.
     */
    Observable<T>& operator=(Observable<T>&& other) noexcept;
    virtual ~Observable();

    void addObserver(T* observer);
    void removeObserver(T* observer);

    virtual void startBlockingNotifications() override final;
    virtual void stopBlockingNotifications() override final;

protected:
    template <typename C>
    void forEachObserver(C callback);

private:
    std::vector<T*> observers_;

    // invocationCount counts how may time we have called forEachObserver
    // Add we will only add and remove observers when that it is zero to avoid
    // Invalidation the iterators. This is needed since a observer might remove it
    // self in the on... callback.
    size_t invocationCount_ = 0;
    std::vector<T*> toAdd_;

    size_t notificationsBlocked_ = 0;

    virtual void addObserver(Observer* observer) override;
    virtual void removeObserver(Observer* observer) override;
    virtual void removeObservers() final;
    virtual bool addObserverInternal(Observer* observer) final;
    virtual bool removeObserverInternal(Observer* observer) final;
};

template <typename T>
Observable<T>::Observable(const Observable<T>&) : observers_{}, toAdd_{} {}

template <typename T>
Observable<T>::Observable(Observable<T>&& rhs) noexcept : observers_{}, toAdd_{} {
    for (auto* o : rhs.observers_) addObserver(o);
    rhs.removeObservers();
}

template <typename T>
Observable<T>& Observable<T>::operator=(const Observable<T>&) {
    return *this;
}

template <typename T>
Observable<T>& Observable<T>::operator=(Observable<T>&& that) noexcept {
    if (this != &that) {
        removeObservers();
        for (auto* o : that.observers_) addObserver(o);
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
    for (auto o : observers_) {
        if (o) removeObservationHelper(o);
    }
    observers_.clear();
}

template <typename T>
void Observable<T>::addObserver(T* observer) {
    if (addObserverInternal(observer)) {
        addObservationHelper(observer);
    }
}

template <typename T>
void Observable<T>::removeObserver(T* observer) {
    if (removeObserverInternal(observer)) {
        removeObservationHelper(observer);
    }
}

template <typename T>
void Observable<T>::startBlockingNotifications() {
    ++notificationsBlocked_;
}
template <typename T>
void Observable<T>::stopBlockingNotifications() {
    --notificationsBlocked_;
}

template <typename T>
template <typename C>
void Observable<T>::forEachObserver(C callback) {
    if (notificationsBlocked_ > 0) return;
    bool toRemove = false;
    ++invocationCount_;

    const util::OnScopeExit decreaseCount{[&]() {
        --invocationCount_;
        // Add and remove any observers that were added/removed while we invoked the callbacks.
        if (invocationCount_ == 0) {
            if (toRemove) {
                observers_.erase(std::remove(observers_.begin(), observers_.end(), nullptr),
                                 observers_.end());
            }
            observers_.insert(observers_.end(), toAdd_.begin(), toAdd_.end());
            toAdd_.clear();
        }
    }};

    for (auto* o : observers_) {
        if (o) {
            callback(o);
        } else {
            toRemove = true;
        }
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
bool Observable<T>::addObserverInternal(Observer* aObserver) {
    auto observer = static_cast<T*>(aObserver);
    const auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it == observers_.end()) {
        if (invocationCount_ == 0) {
            observers_.push_back(observer);
        } else {
            toAdd_.push_back(observer);
        }
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool Observable<T>::removeObserverInternal(Observer* aObserver) {
    auto observer = static_cast<T*>(aObserver);
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        if (invocationCount_ == 0) {
            observers_.erase(it);
        } else {
            *it = nullptr;
        }
        return true;
    } else {
        return false;
    }
}

namespace util {
class IVW_CORE_API NotificationBlocker {
public:
    explicit NotificationBlocker(ObservableInterface& observable);
    NotificationBlocker() = delete;
    NotificationBlocker(const NotificationBlocker&) = delete;
    NotificationBlocker(NotificationBlocker&&) = delete;
    NotificationBlocker& operator=(NotificationBlocker) = delete;
    ~NotificationBlocker();

private:
    ObservableInterface& observable_;
};

}  // namespace util

}  // namespace inviwo
