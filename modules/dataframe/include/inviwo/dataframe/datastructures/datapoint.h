#ifndef IVW_DATAPOINT_H
#define IVW_DATAPOINT_H

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <sstream>

namespace inviwo {

class DataPointBase {
public:
    DataPointBase() = default;
    DataPointBase(DataPointBase &rhs) = default;
    virtual ~DataPointBase() = default;
    virtual std::string toString() const = 0;
};

template <class T>
class DataPoint : public DataPointBase {
public:
    DataPoint() = delete;
    DataPoint(T data);
    DataPoint(DataPoint &rhs) = default;
    virtual ~DataPoint() = default;

    T getData();
    std::string toString() const override;

private:
    T data_;
};

template <typename T>
DataPoint<T>::DataPoint(T data) {
    data_ = data;
}
template <typename T>
T DataPoint<T>::getData() {
    return data_;
}

template <typename T>
std::string DataPoint<T>::toString() const {
    std::stringstream ss;
    ss << data_;
    return ss.str();
}

}  // namespace inviwo

#endif  // IVW_DATAPOINT_H