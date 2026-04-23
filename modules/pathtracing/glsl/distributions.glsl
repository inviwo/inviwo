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

//counting success
int geomertric1_uni(inout uint randHash, float p) {

    int k = 0;
    bool cont = true;

    do {
        k++;
        if(randomize(randHash) > p) {
            cont = false;
        }

    } while (cont);

    return k;
}

//counting failure
int geomertric0_uni(inout uint randHash, float p) {

    int k = -1;
    bool cont = true;

    do {
        k++;
        
        if(randomize(randHash) > p) {
            cont = false;
        }

    } while (cont);

    return k;
}