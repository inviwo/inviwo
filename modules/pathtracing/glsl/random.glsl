/*  
    Hashes copied from Mark Jarzynski's and Marc Olano's 2020 paper
    Hash Functions for GPU Rendering

    These hashes are provided under Creative Commons CC BY-ND 3.0
*/

// largest val representable by a 32bit uint
float uintToFloatbyDiv(uint v) { return v / 4294967295.0f; }

vec2 uintToFloatbyDiv(uvec2 v) { return v / 4294967295.0f; }

vec3 uintToFloatbyDiv(uvec3 v) { return v / 4294967295.0f; }

vec4 uintToFloatbyDiv(uvec4 v) { return v / 4294967295.0f; }

uint getHashSeed(uint x, uint y, uint timeBits) {
    uint globalDimX = x;
    uint globalDimY = y;
    uint gid = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * globalDimX;

    uint hashSeed = gid + floatBitsToUint(timeBits);
    return hashSeed;
}

uint pcg(uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;

    return (word >> 22u) ^ word;
}

uint pcg_rehash(inout uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    v = (word >> 22u) ^ word;
    return v;
}

uvec2 pcg2d(uvec2 v) {
    v = v * 1664525u + 1013904223u;

    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;

    v = v ^ (v >> 16u);

    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;

    v = v ^ (v >> 16u);

    return v;
}

uvec3 pcg3d(uvec3 v) {
    v = v * 1664525u + 1013904223u;

    v.x += v.y * v.z;
    v.y += v.x * v.z;
    v.z += v.x * v.y;

    v = v ^ (v >> 16u);

    v.x += v.y * v.z;
    v.y += v.x * v.z;
    v.z += v.x * v.y;

    return v;
}

uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;

    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;

    v = v ^ (v >> 16u);

    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;

    return v;
}

float random_1dto1d(uint v) { return uintToFloatbyDiv(pcg(v)); }

float random_1dto1d_f(float v) { return uintToFloatbyDiv(pcg(floatBitsToUint(v))); }

vec3 random_1dto3d(uint v) {
    uint hash = pcg(v);
    uint hash2 = pcg(v + hash);
    uint hash3 = pcg(v + hash2);

    return uintToFloatbyDiv(uvec3(hash, hash2, hash3));
}

vec2 random_2dto2d(uvec2 v) { return uintToFloatbyDiv(pcg2d(v)); }

float random_2dto1d(uvec2 v) { return uintToFloatbyDiv(pcg(pcg(v.x) + v.y)); }
