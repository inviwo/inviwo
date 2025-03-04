/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>

#include <memory>
#include <QObject>
#include <Qt>

namespace inviwo {

/**
 * A unique pointer that automatically connects to the QObject's destroyed signal
 * to automatically release the pointer if the object is destroyed.
 */
template <typename T>
class QPtr : std::unique_ptr<T> {
public:
    explicit QPtr() = default;
    explicit QPtr(std::nullptr_t) : std::unique_ptr<T>(std::nullptr_t{}) {}
    explicit QPtr(T* ptr) : std::unique_ptr<T>{ptr} { connect(); }

    QPtr(const QPtr& rhs) = delete;
    QPtr& operator=(const QPtr& that) = delete;
    QPtr(QPtr&& rhs) noexcept : std::unique_ptr<T>{std::move(rhs)} { connect(); }
    QPtr& operator=(QPtr&& that) noexcept {
        if (this != &that) {
            disconnect();
            std::unique_ptr<T>::operator=(std::move(that));
            connect();
        }
        return *this;
    }
    ~QPtr() { disconnect(); }

    void reset(T* ptr = nullptr) {
        disconnect();
        std::unique_ptr<T>::reset(ptr);
        connect();
    }

    using std::unique_ptr<T>::get;
    using std::unique_ptr<T>::release;
    using std::unique_ptr<T>::operator bool;
    using std::unique_ptr<T>::operator*;
    using std::unique_ptr<T>::operator->;

private:
    void disconnect() {
        if (connection) QObject::disconnect(connection);
    }
    void connect() {
        if (T* ptr = get()) {
            connection = QObject::connect(ptr, &T::destroyed, [this](QObject*) { release(); });
        }
    }
    QMetaObject::Connection connection;
};

namespace util {

template <class T, class... Args>
inline auto make_qptr(Args&&... args) {
    return QPtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace util

}  // namespace inviwo
