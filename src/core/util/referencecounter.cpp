#include <inviwo/core/util/referencecounter.h>



inviwo::ReferenceCounter::ReferenceCounter() : referenceCount_(1) {

}

inviwo::ReferenceCounter::~ReferenceCounter() {
    ivwAssert(getRefCount() == 0, "Deleting object with reference count != 0");
}

int inviwo::ReferenceCounter::increaseRefCount() {
    return ++referenceCount_;
}

int inviwo::ReferenceCounter::decreaseRefCount() {
    return --referenceCount_;
}

int inviwo::ReferenceCounter::getRefCount() const {
    return referenceCount_;
}
