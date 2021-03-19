#include "utils/sampler3d.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

in vec4 texCoord_;

void main() {

vec4 original = texture(volume, texCoord_.xyz);
float l, m, n, o;

#ifdef NORMALIZE_CHANNEL_0
    l = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 0);
#else
    l = original[0];
#endif

#ifdef NORMALIZE_CHANNEL_1
    m = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 1);
#else
    m  = original[1];
#endif

#ifdef NORMALIZE_CHANNEL_2
    n = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 2);
#else
    n = original[2];
#endif

#ifdef NORMALIZE_CHANNEL_3
    o = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 3);
#else
    o = original[3];
#endif

    FragData0 = vec4(l, m, n, o);
}
