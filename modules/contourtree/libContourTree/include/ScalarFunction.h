#ifndef SCALARFUNCTION
#define SCALARFUNCTION

#include <vector>
#include <stdint.h>
#include <constants.h>

namespace contourtree {

class ScalarFunction {

public:
    virtual int getMaxDegree() = 0;
    virtual int getVertexCount() = 0;
    virtual int getStar(int64_t v, std::vector<int64_t> &star) = 0;
    virtual bool lessThan(int64_t v1, int64_t v2) = 0;
    virtual scalar_t getFunctionValue(int64_t v) = 0;
};

}

#endif // SCALARFUNCTION

