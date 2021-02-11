#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/base/algorithm/randomutils.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <vector>

namespace inviwo {

namespace pbt {

/*
 * Interface to property assignments, enables to apply the assignment
 * (if it not deactivated).
 * Is used mainly by TestProperty
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API PropertyAssignment {
private:
    const bool* deactivated_;

protected:
    virtual void doApply() const = 0;
    virtual bool doIsApplied() const = 0;

public:
    PropertyAssignment(const bool* deactivated);
    bool isDeactivated() const;

    void apply() const;
    bool isApplied() const;

    virtual ~PropertyAssignment() = default;
};

/*
 * Specialization of PropertyAssignment for anything (P) with
 * P::value_type,
 * set and
 * get
 */
template <typename P, typename T = typename P::value_type,
          decltype(std::declval<P>().set(std::declval<T>()),
                   std::declval<P>().get() == std::declval<T>(), int(0)) = 0>
class PropertyAssignmentTyped : public PropertyAssignment {
private:
    P* const prop;
    const T value;

    void doApply() const override { prop->set(value); }
    bool doIsApplied() const override { return prop->get() == value; }

public:
    PropertyAssignmentTyped(const bool* deactivated, P* const prop, const T& value)
        : PropertyAssignment(deactivated), prop(prop), value(value) {}
    ~PropertyAssignmentTyped() = default;
    P* getProperty() const { return prop; }
    const T& getValue() const { return value; }
};

/*
 * A test is a set of assignments to properties. Should contain no
 * property more than once.
 * We use shared_ptr since multiple tests may assign the same value
 * to a property.
 */
using Test = std::vector<std::shared_ptr<PropertyAssignment>>;

/*
 * Helper for generating assignments.
 * If you want to be able to generate assignments for new properties, you
 * probably want to add a corresponding specialization for this.
 * Note: although the return type is equivalent to Test, there is a semantical
 * difference: a Test should contain no two assignments to the same property,
 * while the returned vector here contains only assignments to the same
 * property.
 */
template <typename T, typename RNG>
struct GenerateAssignments {
    std::vector<std::shared_ptr<PropertyAssignment>> operator()(RNG&, T* const, const bool*) const;
};

constexpr size_t maxStepsPerVal = 3;
constexpr size_t randomValues = 3;

template <typename T, typename RNG>
struct GenerateAssignments<OrdinalProperty<T>, RNG> {
    std::vector<std::shared_ptr<PropertyAssignment>> operator()(RNG& rng,
                                                                OrdinalProperty<T>* const prop,
                                                                const bool* deactivated) const {

        using P = OrdinalProperty<T>;

        std::vector<std::shared_ptr<PropertyAssignment>> res;

        const auto increment = prop->getIncrement();
        const auto minV = prop->getMinValue();
        const auto maxV = prop->getMaxValue();

        for (auto [numSteps, val] = std::tuple<size_t, T>(0, minV);
             numSteps < maxStepsPerVal && val <= prop->getMaxValue() && val >= minV;
             numSteps++, val += increment) {
            res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(deactivated, prop, val));
        }

        for (auto [numSteps, val] = std::tuple<size_t, T>(0, prop->getMaxValue());
             numSteps < maxStepsPerVal && val <= prop->getMaxValue() && val >= minV;
             numSteps++, val -= increment) {
            res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(deactivated, prop, val));
        }

        for (size_t cnt = 0; cnt < randomValues; cnt++) {
            res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
                deactivated, prop, util::randomNumber<T>(rng, minV, maxV)));
        }

        return res;
    }
};

template <typename T, typename RNG>
struct GenerateAssignments<MinMaxProperty<T>, RNG> {
    std::vector<std::shared_ptr<PropertyAssignment>> operator()(RNG& rng,
                                                                MinMaxProperty<T>* const prop,
                                                                const bool* deactivated) const {

        using P = MinMaxProperty<T>;
        using value_type = typename P::value_type;
        std::vector<std::shared_ptr<PropertyAssignment>> res;

        const auto minSeparation = prop->getMinSeparation();

        const auto minR = prop->getRangeMin();
        const auto maxR = prop->getRangeMax();

        for (size_t stepsFromMin = 0; stepsFromMin < maxStepsPerVal; stepsFromMin++) {
            const auto lo = minR + stepsFromMin * minSeparation;
            for (auto [stepsFromMax, hi] = std::tuple<size_t, T>(0, maxR);
                 stepsFromMax + stepsFromMin < maxStepsPerVal && lo + minSeparation <= hi;
                 stepsFromMax++, hi -= minSeparation) {
                res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(deactivated, prop,
                                                                              value_type(lo, hi)));
            }
        }

        // add some random values
        for (size_t cnt = 0; cnt < randomValues; cnt++) {
            auto lo = util::randomNumber<T>(rng, minR, maxR);
            auto hi = util::randomNumber<T>(rng, minR, maxR);
            if (hi < lo) {
                std::swap(lo, hi);
            }
            // ensure minSeparation is respected
            if (lo > hi - minSeparation) {
                if (lo > maxR - minSeparation) {
                    lo = hi - minSeparation;
                } else {
                    hi = lo + minSeparation;
                }
            }
            res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(deactivated, prop,
                                                                          value_type(lo, hi)));
        }

        return res;
    }
};

}  // namespace pbt

}  // namespace inviwo
