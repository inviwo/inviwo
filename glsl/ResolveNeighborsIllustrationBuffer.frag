/**
 * Resolves the neighborhood of a fragment and sets the initial conditions for the silhouettes+halos
 */

#include "IllustrationBuffer.hglsl"

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding=0) buffer surfaceInfoBufferIn
{
    vec2 surfaceInfoIn[]; //depth+gradient 
};
layout(std430, binding=1) buffer neighborBufferOut
{
    ivec4 neighborsOut[];
};
layout(std430, binding=2) buffer smoothingBufferOut
{
    vec2 smoothingOut[]; //beta + gamma
};

//Computes the likelihood that fragment i and j are neighbors
//  direction = pixel_coordinate[i] - pixel_coordinate[j]
float continuityMeasure(int i, int j, ivec2 direction);
//Returns count and start of the lists at the given position
//Keeps image boundary in mind
ivec2 getCountAndStart(ivec2 pos);
//finds the neighbor of fragment i at the given neighbor pixel
int findNeighbor(int i, ivec2 pos, ivec2 dir, ivec2 csA, out int neighborIndex);

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<screenSize.x 
	    && coords.y<screenSize.y ){

        int count = int(imageLoad(illustrationBufferCountImg, coords).x);
        if (count > 0) {
            int start = int(imageLoad(illustrationBufferIdxImg, coords).x);
            for (int i=0; i<count; ++i) {
                //find neighbors
                ivec4 neighbors;
                ivec4 neighborIndices;
                neighbors.x = findNeighbor(start + i, coords, ivec2(-1, 0), ivec2(count, start), neighborIndices.x);
                neighbors.y = findNeighbor(start + i, coords, ivec2(+1, 0), ivec2(count, start), neighborIndices.y);
                neighbors.z = findNeighbor(start + i, coords, ivec2(0, -1), ivec2(count, start), neighborIndices.z);
                neighbors.w = findNeighbor(start + i, coords, ivec2(0, +1), ivec2(count, start), neighborIndices.w);
                for (int j=0; j<4; ++j)
                    if (neighborIndices[j]==-1) neighborIndices[j] = i;
                //initialize field for silhouette + halo
                vec2 smoothing = vec2(0);
                if (any(lessThan(neighbors, ivec4(0)))) {
                    //red cell
                    smoothing.x = 1;
                } else if (any(greaterThan(neighborIndices, ivec4(i)))) {
                    //blue cell
                    smoothing.y = 1;
                } else if (any(lessThan(neighborIndices, ivec4(i)))) {
                    //green cell
                    smoothing.x = 0;
                }
                //write out
                neighborsOut[start + i] = neighbors;
                smoothingOut[start + i] = smoothing;
            }
        }
    }
    discard;
}

float continuityMeasure(int i, int j, ivec2 direction)
{
    //first simplistic version, just compare the depth
    return abs(surfaceInfoIn[i].x - surfaceInfoIn[j].x);
}

ivec2 getCountAndStart(ivec2 pos)
{
    if(pos.x>=0 && pos.y>=0 
	    && pos.x<screenSize.x 
	    && pos.y<screenSize.y ){

        uint count = imageLoad(illustrationBufferCountImg, pos).x;
        if (count > 0) {
            uint start = imageLoad(illustrationBufferIdxImg, pos).x;
            return ivec2(count, start);
        }
    }
    return ivec2(0,0);
}

int findNeighbor(int i, ivec2 pos, ivec2 dir, ivec2 csA, out int neighborIndex)
{
    //Algorithm 2 from the paper "Smart Transparency ..."
    ivec2 csB = getCountAndStart(pos + dir);
    int neighbor = -1;
    neighborIndex = -1;
    float eNeighbor = 100000000000.0; //inf
    //find the best candidate for A among all Bs
    for (int j=csB.y; j<csB.x+csB.y; ++j) 
    {
        float eB = continuityMeasure(i, j, dir);
        if (eB < eNeighbor) {
            neighbor = j;
            eNeighbor = eB;
            neighborIndex = j-csB.y;
        }
    }
    //check if there is a better candidate among all A's
    //TODO: move this to a separate pass? Then it can be optimized, no need for a loop then
    if (neighbor >= 0) {
        for (int i2=csA.y; i2<csA.x+csA.y; ++i2) {
            float eA = continuityMeasure(i2, neighbor, dir);
            if (eA < eNeighbor) {
                neighborIndex = -1;
                return -1; //relation not mutual
            }
        }
    }
    return neighbor;
}