/* ---
    static.frag
    by Statial
    05 July 2013

    Crediting the user Spatial from stackoverflow with this implementation
    the expected value is 0.5 between 0 and 1, as expected.
    Not necesarily uniform by this standard however. Consider better hashes
*/

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

/*  ---
    Hashes copied from Mark Jarzynski's and Marc Olano's 2020 paper
    Hash Functions for GPU Rendering
    
    These hashes are provided under Creative Commons CC BY-ND 3.0
    --- */

// largest val representable by a 32bit uint
float uintToFloatbyDiv(uint v) {
    return v/4294967295.0f;
}

vec2 uintToFloatbyDiv(uvec2 v) {
    return v/4294967295.0f;
}

vec3 uintToFloatbyDiv(uvec3 v) {
    return v/4294967295.0f;
}

vec4 uintToFloatbyDiv(uvec4 v) {
    return v/4294967295.0f;
}

uint pcg(uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state)
        * 277803737u;

    return (word >> 22u) ^ word;
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

    v.x += v.y*v.z;
    v.y += v.x*v.z;
    v.z += v.x*v.y;

    v = v ^ (v>>16u);

    v.x += v.y*v.z;
    v.y += v.x*v.z;
    v.z += v.x*v.y;

    return v;
}

uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;

    v = v ^ (v>>16u);

    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;

    return v;
}

// use floatBitsToUint to preseve float bit
float random_1dto1d(float v) {
    return uintToFloatbyDiv(pcg(floatBitsToUint(v)));
}

float random_1dto1d(uint v) {
    return uintToFloatbyDiv(pcg(v));
}

vec3 random_1dto3d (uint v) {
    uint hash = pcg(v);
    uint hash2 = pcg(v + hash);
    uint hash3 = pcg(v + hash2);

    return uintToFloatbyDiv(uvec3(
        hash,
        hash2,
        hash3
    )); 
}

vec2 random_2dto2d (uvec2 v) {
    return uintToFloatbyDiv(pcg2d(v));
}

float random_2dto1d (uvec2 v) {
    return uintToFloatbyDiv(pcg(pcg(v.x) + v.y));
}
