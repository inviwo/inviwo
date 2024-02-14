#include "random.glsl"

int poisson_uni(inout uint randHash, float lambda) {
    int k = -1;
    float p = 1.f;
    float lambda_exp = exp(-lambda);
    do {
        k++;
        p *= randomize(randHash);
    } while(p > lambda_exp);
    return k;
}

int geomertric_uni() {
    return 0;
}