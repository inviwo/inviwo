#include "utils/sampler3d.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;
uniform int channel = 0;

in vec4 texCoord_;

void main() {
    float v = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, channel);

    FragData0 = vec4(v);
}
