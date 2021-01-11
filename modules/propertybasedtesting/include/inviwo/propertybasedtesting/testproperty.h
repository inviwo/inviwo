/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

#include <filesystem>
#include <fstream>

namespace inviwo {

class TestResult;

class TestPropertyObservable;

class TestPropertyObserver : public Observer {
public:
    friend TestPropertyObservable;
    TestPropertyObserver() = default;
    virtual void onTestPropertyChange() {}
    virtual ~TestPropertyObserver() = default;
};

class TestPropertyObservable : public Observable<TestPropertyObserver> {
protected:
    TestPropertyObservable() = default;
    void notifyObserversAboutChange() {
        forEachObserver([](TestPropertyObserver* o) { o->onTestPropertyChange(); });
    }
    virtual ~TestPropertyObservable() = default;
};

template <typename T>
class NetworkPath : public Serializable {
private:
    std::unique_ptr<std::string> path_;
    std::unique_ptr<T*> ptrPtr_;
    bool isProcessor_;

    ProcessorNetwork* pn_;

    const std::string& path() const {
        if (*ptrPtr_ == nullptr) {
            assert(!path_->empty());
            return *path_;
        } else {
            if (auto p = dynamic_cast<Processor*>(*ptrPtr_); p != nullptr) {
                assert(isProcessor_);
                return *path_ = p->getIdentifier();
            } else {
                assert(!isProcessor_);
                auto p2 = dynamic_cast<Property*>(*ptrPtr_);
                assert(p2 != nullptr);
                return *path_ = p2->getPath();
            }
        }
    }

public:
    bool operator<(const NetworkPath& other) const { return path() < other.path(); }
    // only for deserialization
    NetworkPath() = default;

    NetworkPath(const std::string& path, ProcessorNetwork* pn,
                bool isProcessor = std::is_same_v<T, Processor>)
        : path_(std::make_unique<std::string>(path))
        , ptrPtr_(std::make_unique<T*>(nullptr))
        , isProcessor_(isProcessor)
        , pn_(pn) {}

    NetworkPath(T* ptr, ProcessorNetwork* pn = nullptr)
        : path_(std::make_unique<std::string>(""))
        , ptrPtr_(std::make_unique<T*>(ptr))
        , isProcessor_(dynamic_cast<Processor*>(ptr) != nullptr)
        , pn_(pn) {}
    void setNetwork(ProcessorNetwork* pn) {
        if (pn == nullptr) {
            *path_ = path();
            *ptrPtr_ = nullptr;
        }
        pn_ = pn;
    }

    T* get() const {
        if (*ptrPtr_ == nullptr) {
            assert(pn_ != nullptr);
            if (isProcessor_) {
                return *ptrPtr_ = reinterpret_cast<T*>(pn_->getProcessorByIdentifier(*path_));
            } else {
                Property* tmp = pn_->getProperty(*path_);
                assert(tmp != nullptr);
                *ptrPtr_ = dynamic_cast<T*>(tmp);
                assert(*ptrPtr_ != nullptr);
            }
        }
        return *ptrPtr_;
    }
    operator T*() const { return get(); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    void serialize(Serializer& s) const override {
        s.serialize("Path", path());
        s.serialize("IsProcessor", isProcessor_);
    }
    void deserialize(Deserializer& d) override {
        std::string path;
        d.deserialize("Path", path);
        path_ = std::make_unique<std::string>(path);
        d.deserialize("IsProcessor", isProcessor_);
        pn_ = nullptr;
        ptrPtr_ = std::make_unique<T*>(nullptr);
    }
};

/** \docpage{org.inviwo.PropertyAnalyzer, PropertyAnalyzer}
 * ![](org.inviwo.TestProperty.png?classIdentifier=org.inviwo.TestProperty)
 *
 * A TestProperty holds a reference to a testable property and maintains the
 * respective BoolCompositeProperty and drop-down menu.
 * It is used to provide a general interface to all properties, regardless of
 * their value_type.
 * It enables
 *	- generation of assignments,
 *	- a textual description of the expected effects on the counted number of pixels,
 *	- the expected effect on the counted number of pixels given two testcases, and
 *  - storing and resetting the value of the property befor the tests have started.
 * Instantiation happes mainly by testableProperty (see below).
 */
class TestProperty : public Serializable, public TestPropertyObservable {
private:
    std::string displayName_, identifier_;
    NetworkPath<BoolCompositeProperty> boolComp_ = nullptr;
    std::vector<std::function<void(ProcessorNetwork*)>> onNetwork;

protected:
    void onNetworkReceive(std::function<void(ProcessorNetwork*)>);
    TestProperty() = default;
    TestProperty(const std::string& displayName, const std::string& identifier);

    const auto& options() const {
        const static std::vector<OptionPropertyOption<int>> options{
            {"EQUAL", "EQUAL", 0},     {"NOT_EQUAL", "NOT_EQUAL", 1},
            {"LESS", "LESS", 2},       {"LESS_EQUAL", "LESS_EQUAL", 3},
            {"GREATER", "GREATER", 4}, {"GREATER_EQUAL", "GREATER_EQUAL", 5},
            {"ANY", "ANY", 6},         {"NOT_COMPARABLE", "NOT_COMPARABLE", 7}};
        return options;
    }

public:
    void setNetwork(ProcessorNetwork*);
    virtual size_t totalCheckedComponents() const = 0;
    virtual bool* deactivated(size_t) = 0;
    virtual const bool* deactivated(size_t) const = 0;

    virtual void serialize(Serializer&) const override;
    virtual void deserialize(Deserializer&) override;

    virtual BoolCompositeProperty* getBoolComp() const;

    const std::string& getDisplayName() const;
    const std::string& getIdentifier() const;

    virtual std::string textualDescription(unsigned int indent = 0) const = 0;

    virtual std::string getValueString(std::shared_ptr<TestResult>) const = 0;
    void traverse(std::function<void(const TestProperty*, const TestProperty*)>) const;
    virtual void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                          const TestProperty*) const = 0;

    virtual std::optional<util::PropertyEffect> getPropertyEffect(
        std::shared_ptr<TestResult>, std::shared_ptr<TestResult>) const = 0;

    virtual std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>) const = 0;

    virtual std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>,
                               std::shared_ptr<TestResult>) const = 0;

    virtual void setToDefault() const = 0;
    virtual void storeDefault() = 0;
    virtual std::vector<
        std::pair<util::AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp() const = 0;
    virtual ~TestProperty() = default;
};

/*
 * returns the combined expected effect (if any) of two values, given the
 * effect of each of the components.
 */
template <typename T, size_t numComp = DataFormat<T>::components()>
std::optional<util::PropertyEffect> propertyEffect(
    const T& val_old, const T& val_new,
    const std::array<util::PropertyEffect, numComp>& selectedEffects);

/*
 * Creates and returns a TestProperty for an arbitrary property, if possible.
 * That is, if the given property derives from CompositeProperty or one of
 * the properties in PropertyTypes (see below).
 */
std::optional<std::unique_ptr<TestProperty>> testableProperty(Property* prop);

/*
 * Adds necessary callbacks for the proper behavior of the BoolComps of/in a
 * BoolCompositeProperty, i.e. the BoolProperty of the BoolCompositeProperty
 * is checked, if and only if at least one of the contained
 * BoolCompositeProperties is checked.
 */
void makeOnChange(BoolCompositeProperty* const prop);

/*
 * Derived from TestProperty, for all PropertyOwners supporting
 * getDisplayName and getIdentifier, i.e. CompositeProperty and Processor
 * May only be instantiated by TestPropertyComposite::make
 */
class TestPropertyComposite : public TestProperty, public TestPropertyObserver {
    NetworkPath<PropertyOwner> propertyOwner_;
    std::vector<std::unique_ptr<TestProperty>> subProperties;

    TestPropertyComposite() = default;
    TestPropertyComposite(PropertyOwner* original, const std::string& displayName,
                          const std::string& identifier);

    void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                  const TestProperty*) const override;

public:
    PropertyOwner* getPropertyOwner() const;

    template <typename F>
    void for_each_checked(F) const;

    size_t totalCheckedComponents() const override;
    bool* deactivated(size_t) override;
    const bool* deactivated(size_t) const override;

    const static std::string& getClassIdentifier() {
        const static std::string name = "org.inviwo.TestPropertyComposite";
        return name;
    }
    friend class TestPropertyFactory;
    friend class TestPropertyCompositeFactory;

    void onTestPropertyChange() override;

    std::string textualDescription(unsigned int indent = 0) const override;

    std::string getValueString(std::shared_ptr<TestResult> testResult) const override;

    std::optional<util::PropertyEffect> getPropertyEffect(
        std::shared_ptr<TestResult> newTestResult,
        std::shared_ptr<TestResult> oldTestResult) const override;

    std::ostream& ostr(std::ostream& out, std::shared_ptr<TestResult> testResult) const override;

    std::ostream& ostr(std::ostream& out, std::shared_ptr<TestResult> newTestResult,
                       std::shared_ptr<TestResult> oldTestResult) const override;

    // Constructor for all Propertyowners with a display name and an identifier
    template <typename C, decltype(static_cast<PropertyOwner*>(std::declval<C*>()),
                                   std::declval<C>().getDisplayName(),
                                   std::declval<C>().getIdentifier(), int()) = 0>
    static std::unique_ptr<TestPropertyComposite> make(C* orig);

    virtual ~TestPropertyComposite() = default;
    void setToDefault() const override;

    void serialize(Serializer& s) const override;
    void deserialize(Deserializer& s) override;

    template <typename T>
    std::optional<typename T::value_type> getDefaultValue(const T* prop) const;

    void storeDefault();
    std::vector<
        std::pair<util::AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp() const override;
};

template <typename C,
          decltype(static_cast<PropertyOwner*>(std::declval<C*>()),
                   std::declval<C>().getDisplayName(), std::declval<C>().getIdentifier(), int())>
std::unique_ptr<TestPropertyComposite> TestPropertyComposite::make(C* orig) {
    std::string ident = orig->getIdentifier();
    std::replace(ident.begin(), ident.end(), ' ', '_');
    return std::unique_ptr<TestPropertyComposite>(
        new TestPropertyComposite(orig, orig->getDisplayName(), ident));
}

/*
 * Derived from TestProperty, for all Properties having a value that is
 * supported by DataFormat<>.
 */
template <typename T>
class TestPropertyTyped : public TestProperty {
    using value_type = typename T::value_type;
    static constexpr size_t numComponents = DataFormat<value_type>::components();

    NetworkPath<T> typedProperty_;
    value_type defaultValue_;
    std::array<NetworkPath<OptionPropertyInt>, numComponents> effectOption_;

    TestPropertyTyped() = default;

    std::array<util::PropertyEffect, numComponents> selectedEffects() const;
    std::unique_ptr<bool> deactivated_;

    void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                  const TestProperty*) const override;

public:
    OptionPropertyInt* getEffectOption(size_t) const;
    T* getTypedProperty() const;

    size_t totalCheckedComponents() const override;
    bool* deactivated(size_t) override;
    const bool* deactivated(size_t) const override;

    const static std::string& getClassIdentifier() {
        const static std::string name =
            std::string("org.inviwo.TestPropertyTyped") + PropertyTraits<T>::classIdentifier();
        return name;
    }
    friend class TestPropertyFactoryHelper;

    std::string textualDescription(unsigned int indent) const override;

    std::string getValueString(std::shared_ptr<TestResult>) const override;

    std::optional<util::PropertyEffect> getPropertyEffect(
        std::shared_ptr<TestResult> newTestResult,
        std::shared_ptr<TestResult> oldTestResult) const override;

    std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>) const override;

    std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>,
                       std::shared_ptr<TestResult>) const override;

    TestPropertyTyped(T* original);
    virtual ~TestPropertyTyped() = default;
    void setToDefault() const override;
    const value_type& getDefaultValue() const;

    void serialize(Serializer& s) const override;
    void deserialize(Deserializer& s) override;

    void storeDefault();
    std::vector<
        std::pair<util::AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp() const override;
};

/*
 * Container for a test result.
 * Allows access to the tested value of each property, as well as the
 * resulted number of pixels.
 */
class TestResult {
private:
    const std::vector<TestProperty*>& defaultValues;
    const Test test;
    const size_t pixels;
    const std::filesystem::path imgPath;

public:
    size_t getNumberOfPixels() { return pixels; }
    const std::filesystem::path& getImagePath() { return imgPath; }

    template <typename T>
    typename T::value_type getValue(const T* prop) const;

    TestResult(const std::vector<TestProperty*>& defaultValues, const Test& t, size_t val,
               const std::filesystem::path& imgPath)
        : defaultValues(defaultValues), test(t), pixels(val), imgPath(imgPath) {}
};

template <typename T, size_t numComp = DataFormat<T>::components()>
std::optional<util::PropertyEffect> propertyEffect(
    const T& val_old, const T& val_new,
    const std::array<util::PropertyEffect, numComp>& selectedEffects) {

    std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
    for (size_t i = 0; res && i < numComp; i++) {
        auto compEff = util::propertyEffect(selectedEffects[i],
                                            util::GetComponent<T, numComp>::get(val_new, i),
                                            util::GetComponent<T, numComp>::get(val_old, i));
        res = util::combine(*res, compEff);
    }
    return res;
}

template <typename F>
void TestPropertyComposite::for_each_checked(F f) const {
    for (const auto& prop : subProperties) {
        if (prop->getBoolComp()->isChecked()) f(prop.get());
    }
}

class TestPropertyFactory : public Factory<TestProperty> {
    static const std::unordered_map<std::string, std::function<std::unique_ptr<TestProperty>()>>
        members;

public:
    std::unique_ptr<TestProperty> create(const std::string& key) const override;
    bool hasKey(const std::string& key) const override;

    friend class TestPropertyCompositeFactory;
};
class TestPropertyCompositeFactory : public Factory<TestPropertyComposite> {
public:
    std::unique_ptr<TestPropertyComposite> create(const std::string& key) const override;
    bool hasKey(const std::string& key) const override;
};

template <typename T>
std::optional<typename T::value_type> TestPropertyComposite::getDefaultValue(const T* prop) const {
    for (const auto& subProp : subProperties) {
        if (auto p = dynamic_cast<TestPropertyTyped<T>*>(subProp.get()); p != nullptr) {
            if (p->getTypedProperty() == prop) return p->getDefaultValue();
        } else if (auto p = dynamic_cast<TestPropertyComposite*>(subProp.get()); p != nullptr) {
            if (auto res = p->getDefaultValue(prop); res != std::nullopt) return res;
        }
    }
    return std::nullopt;
}
template <typename T>
typename T::value_type TestResult::getValue(const T* prop) const {
    for (const auto& t : test) {
        if (auto p = std::dynamic_pointer_cast<PropertyAssignmentTyped<T>>(t);
            p && reinterpret_cast<T*>(p->getProperty()) == prop && !(p->isDeactivated()))
            return p->getValue();
    }

    for (auto def : defaultValues) {
        if (auto p = dynamic_cast<TestPropertyTyped<T>*>(def); p != nullptr) {
            if (p->getTypedProperty() == prop) return p->getDefaultValue();
        } else if (auto p = dynamic_cast<TestPropertyComposite*>(def); p != nullptr) {
            if (auto res = p->getDefaultValue(prop); res != std::nullopt) return *res;
        }
    }
    std::cerr << "could not get value for " << prop << " " << typeid(T).name() << std::endl;
    exit(1);
}

/*
 * Tuple containing all testable property types except CompositeProperty.
 * If you want to add support for a new property type, you probably want to add
 * it to this list.
 */
using PropertyTypes = std::tuple<IntProperty, FloatProperty, DoubleProperty, IntMinMaxProperty>;

using TestingError = std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>,
                                util::PropertyEffect, size_t, size_t>;

void testingErrorToBinary(std::vector<unsigned char>&, const std::vector<TestProperty*>&,
                          const TestingError&);

}  // namespace inviwo
