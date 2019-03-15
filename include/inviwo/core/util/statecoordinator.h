/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_STATECOORDINATOR_H
#define IVW_STATECOORDINATOR_H

#include <inviwo/core/common/inviwocoredefine.h>

#include <functional>

namespace inviwo {

/**
 * \class StateCoordinator
 * A utility class for keeping track of changes to some state, and delivering notifications when it
 * changes. The StateCoordinator uses two functors one for updating the state (updater), and one for
 * delivering a notification (notifier) when the result of the updater changes. Moreover there is an
 * update function which the user should call whenever the result of the updater might change. For
 * example in Processor the isSink StateCoordinator updater depends on the number of outports, hence
 * we call update after each call to addPort and removePort.
 */
template <typename T>
class StateCoordinator {
public:
    /**
     * Construct a StateCoordinator with a initial value, a notification functor, and a update
     * functor.
     */
    StateCoordinator(const T& value, std::function<void(const T&)> notifyer,
                     std::function<T()> updater)
        : updater_{std::move(updater)}, notifyer_{std::move(notifyer)}, value_{value} {}

    /**
     * Trigger an update. Will call the updater and if the value changes call the notifier.
     * This function should be called whenever the outcome of the updater might change.
     */
    void update() {
        auto value = updater_();
        if (value != value_) {
            value_ = value;
            notifyer_(value_);
        }
    }

    /**
     * Set the update functor
     */
    void setUpdate(std::function<T()> updater) {
        updater_ = std::move(updater);
        update();
    }
    /**
     * Set the notifier functor
     */
    void setNotify(std::function<void(const T&)> notifier) { notifyer_ = std::move(notifier); }

    /**
     *	Implicit conversion to value.
     */
    operator const T&() const { return value_; }

    /**
     *	Get the value.
     */
    const T& get() const { return value_; }

private:
    std::function<T()> updater_;
    std::function<void(const T&)> notifyer_;
    T value_;
};

}  // namespace inviwo

#endif  // IVW_STATECOORDINATOR_H
