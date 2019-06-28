/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef INVIWO_PROJECTS_SPLINEINTERPOLATION_H
#define INVIWO_PROJECTS_SPLINEINTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/animation/interpolation/interpolation.h>
#include <algorithm>
#include "nurb.h"

namespace inviwo::animation {
        template <typename Key>
        class SplineInterpolation : public InterpolationTyped<Key> {
        public:
            SplineInterpolation() = default;
            virtual ~SplineInterpolation() = default;

            virtual SplineInterpolation* clone() const override;

            virtual std::string getName() const override;

            static std::string classIdentifier();
            virtual std::string getClassIdentifier() const override;

            virtual bool equal (const Interpolation& other) const override;

            virtual void serialize(Serializer& s) const override;
            virtual void deserialize(Deserializer& d) override;

            // keys should be sorted by time
            virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                                    easing::EasingType) const -> typename Key::value_type override;
        private:
        Nurb _spline;
        bool _init = false;
        };

        template <typename Key>
        SplineInterpolation<Key>* SplineInterpolation<Key>::clone() const {
            return new SplineInterpolation<Key>(*this);
        }

        namespace detail {
            template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
            std::string getSplineInterpolationClassIdentifier() {
                return "org.inviwo.splineinterpolation.splineinterpolation." + Defaultvalues<T>::getName();
            }
            template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
            std::string getSplineInterpolationClassIdentifier() {
                using ET = typename std::underlying_type<T>::type;
                return "rg.inviwo.splineinterpolation.splineinterpolation.enum." + Defaultvalues<ET>::getName();
            }
        }  // namespace detail


        template <typename Key>
        std::string SplineInterpolation<Key>::classIdentifier() {
            return detail::getSplineInterpolationClassIdentifier<typename Key::value_type>();
        }

        template <typename Key>
        std::string SplineInterpolation<Key>::getClassIdentifier() const {
            return classIdentifier();
        }

        template <typename Key>
        std::string SplineInterpolation<Key>::getName() const {
            return "Spline";
        }

        template <typename Key>
        bool SplineInterpolation<Key>::equal(const Interpolation& other) const {
            return classIdentifier() == other.getClassIdentifier();
        }
        template <typename Key>
        auto SplineInterpolation<Key>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                                    Seconds from, Seconds to, easing::EasingType) const ->
        typename Key::value_type {
            using VT = typename Key::value_type;


            auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
                return time < key->getTime();
            });
            return it->getValue();

        }

        template <typename Key>
        void SplineInterpolation<Key>::serialize(Serializer& s) const {
            s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
        }

        template <typename Key>
        void SplineInterpolation<Key>::deserialize(Deserializer& d) {
            std::string className;
            d.deserialize("type", className, SerializationTarget::Attribute);
            if (className != getClassIdentifier()) {
                throw SerializationException(
                        "Deserialized interpolation: " + getClassIdentifier() +
                        " from a serialized interpolation with a different class identifier: " + className,
                        IvwContext);
            }
        }
} // namespace inviwo // namespace animation

#endif //INVIWO_PROJECTS_SPLINEINTERPOLATION_H
