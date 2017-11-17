/**
 * Resolves the neighborhood of a fragment and sets the initial conditions for the silhouettes+halos
 */

#include "IllustrationBuffer.hglsl"

//The host defines the following macro
//#define PROCESSING_STEP
//Which is:
// 1=Find initial neighbors
// 2=Check for mutual relation
// 

//Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
//Input interpolated fragment position
smooth in vec4 fragPos;

//Computes the likelihood that fragment i and j are neighbors
//  direction = pixel_coordinate[i] - pixel_coordinate[j]
float continuityMeasure(int i, int j, ivec2 direction);
//Returns count and start of the lists at the given position
//Keeps image boundary in mind
ivec2 getCountAndStart(ivec2 pos);
//finds the neighbor of fragment i at the given neighbor pixel
int findNeighbor(int i, ivec2 pos, ivec2 dir, ivec2 csA);

void main(void) {
	ivec2 coords=ivec2(gl_FragCoord.xy);
	
	if(coords.x>=0 && coords.y>=0 
	    && coords.x<screenSize.x 
	    && coords.y<screenSize.y ){

        int count = int(imageLoad(illustrationBufferCountImg, coords).x);
        if (count > 0) {
            int start = int(imageLoad(illustrationBufferIdxImg, coords).x);
            for (int i=0; i<count; ++i) {
                FragmentData d = illustrationDataIn[start + i];
                //find neighbors
                d.neighbors.x = findNeighbor(start + i, coords, ivec2(-1, 0), ivec2(count, start));
                d.neighbors.y = findNeighbor(start + i, coords, ivec2(+1, 0), ivec2(count, start));
                d.neighbors.z = findNeighbor(start + i, coords, ivec2(0, -1), ivec2(count, start));
                d.neighbors.w = findNeighbor(start + i, coords, ivec2(0, +1), ivec2(count, start));
                ivec4 neighborIndices = ivec4(
                    d.neighbors.x >= 0 ? illustrationDataIn[d.neighbors.x].index : i,
                    d.neighbors.y >= 0 ? illustrationDataIn[d.neighbors.y].index : i,
                    d.neighbors.z >= 0 ? illustrationDataIn[d.neighbors.z].index : i,
                    d.neighbors.w >= 0 ? illustrationDataIn[d.neighbors.w].index : i
                );
                //initialize field for silhouette + halo
                if (any(lessThan(d.neighbors, ivec4(0)))) {
                    //red cell
                    d.silhouetteHighlight = 1;
                } else if (any(greaterThan(neighborIndices, ivec4(i)))) {
                    //blue cell
                    d.haloHighlight = 1;
                } else if (any(lessThan(neighborIndices, ivec4(i)))) {
                    //green cell
                    d.silhouetteHighlight = 0;
                }
                //write out
                illustrationDataOut[start + i] = d;
            }
        }
    }
    discard;
}

float continuityMeasure(int i, int j, ivec2 direction)
{
    //first simplistic version, just compare the depth
    return abs(illustrationDataIn[i].depth - illustrationDataIn[j].depth);
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

int findNeighbor(int i, ivec2 pos, ivec2 dir, ivec2 csA)
{
    //Algorithm 2 from the paper "Smart Transparency ..."
    ivec2 csB = getCountAndStart(pos + dir);
    int neighbor = -1;
    float eNeighbor = 100000000000.0; //inf
    //find the best candidate for A among all Bs
    for (int j=csB.y; j<csB.x+csB.y; ++j) 
    {
        float eB = continuityMeasure(i, j, dir);
        if (eB < eNeighbor) {
            neighbor = j;
            eNeighbor = eB;
        }
    }
    //check if there is a better candidate among all A's
    //TODO: move this to a separate pass? Then it can be optimized, no need for a loop then
    if (neighbor >= 0) {
        for (int i2=csA.y; i2<csA.x+csA.y; ++i2) {
            float eA = continuityMeasure(i2, neighbor, dir);
            if (eA < eNeighbor) {
                return -1; //relation not mutual
            }
        }
    }
    return neighbor;
}