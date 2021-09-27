#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace contourtree {

const char REGULAR = 0;
const char MINIMUM = 1;
const char MAXIMUM = 2;
const char SADDLE = 4;

// Following the nomenclature of original Carr paper.
// JoinTree -> maxima and SplitTree -> minima
enum TreeType {TypeJoinTree, TypeSplitTree, TypeContourTree};

typedef float scalar_t;

}

#endif // CONSTANTS_H
