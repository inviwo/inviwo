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
#include <set>
#include <functional>
#include <algorithm>

namespace inviwo {
// Forward declaration
class ObservableInterface;

/** \class Observer
 *
 * Class to support observer pattern. An example of usage is given in the Observable class.
 * @see Observable
 */
class IVW_CORE_API Observer {

    friend class ObservableInterface;
public:
    /**
     * Allocates Observable set.
     */
    Observer();
    /**
     * Copy constructor. Copies observerables from other  
     */
    Observer(const Observer& other);

    /**
    * Move constructor. Steals resources from other
    */
    Observer(Observer&& other); 

    /**
    * Move assignment. Steals resources from other
    */
    Observer& operator=(Observer&& other);

    /**
     * Copies observerables from other and returns this.
     */
    Observer& operator=(const Observer& other);

    /**
     * Removes the observer from all observable objects. Makes sure that it cannot be called when destroyed.
     *
     */
    virtual ~Observer();

    /**
     * Stop observing object by removing it from observation list.
     *
     * @param observable (ObservableInterface *) The observable to stop observing.
     */
    void removeObservation(ObservableInterface* observable);

    /**
     * Stops observing all objects by removing them from observation list.
     *
     */
    void removeObservations();

protected:
    /**
     * Add an object to observe.
     *
     * @param observable The observable to add.
     */
    void addObservation(ObservableInterface* observable);


    // Storing observables connected to this observer enables removal upon destruction.
    typedef std::set<ObservableInterface*> ObservableSet;
    ObservableSet* observables_;
};

/** \class ObservableInterface
 *
 * Class to support observer pattern.
 * This is an interface only, inherit from Observable<DerivedObserver> to define your own "notify()" method.
 * An example of usage is given in the Observable class.
 * @see Observable
 * @see Observer
 */
class IVW_CORE_API ObservableInterface {
    friend class Observer;
public:
    /**
     * Allocates Observer set.
     */
    ObservableInterface();
    /**
     * Copy constructor. Copies observers from other  
     */
    ObservableInterface(const ObservableInterface& other);

    /** 
     * Move constructor. Steals resources from other
     */
    ObservableInterface(ObservableInterface&& other); 

    /**
    * Move assignment. Steals resources from other
    */
    ObservableInterface& operator=(ObservableInterface&& other);
    /**
     * Copies observers from other and returns this.
     */
    ObservableInterface& operator=(const ObservableInterface& other);
    /*
     * Removes all observers.
     */
    virtual ~ObservableInterface();

protected:
    /**
     * Add an observer.
     *
     * @param observer The observer to add.
     */
    void addObserver(Observer* observer);
    /**
     * Remove an observer.
     *
     * @param observer The observer to remove.
     */
    void removeObserver(Observer* observer);
    /**
     * Remove all observers.
     */
    void removeObservers();

    typedef std::set<Observer*> ObserverSet;
    ObserverSet* observers_;

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
 *            ObserverSet::iterator endIt = observers_->end();
 *            for(ObserverSet::iterator it = observers_->begin(); it != endIt; ++it) {
 *               // static_cast can be used since only template class objects can be added
 *               static_cast<ButtonObserver*>(*it)->buttonPressed();
 *            }
 *        }
 *    };
 * @endcode
 * @see Observer
 * @see VoidObserver
 */
template<typename T>
class Observable: public ObservableInterface {
public:
    Observable(): ObservableInterface() {};
    /**
     * Removes all observers
     */
    virtual ~Observable() {};
    /**
     * Add an observer.
     *
     * @param observer The observer to add.
     */
    virtual void addObserver(T* observer) {
        ObservableInterface::addObserver(observer);
    }
    /**
     * Remove an observer.
     *
     * @param observer The observer to remove.
     */
    virtual void removeObserver(T* observer) {
        ObservableInterface::removeObserver(observer);
    }

    /**
    * Calls the lambda function func on all registered observers.
    *
    * @param func the lambda function that will be called.
    */
    virtual void forEach(std::function<void(T*)> func) {
        std::for_each(observers_->begin(), observers_->end(),
            [&](Observer* o) { func(dynamic_cast<T*>(o)); });
    }
};

} // namespace

#endif // IVW_OBSERVER_H
