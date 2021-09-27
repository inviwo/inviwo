#ifndef DISJOINTSETS_HPP
#define DISJOINTSETS_HPP

#include <stdint.h>
#include <vector>
#include <cassert>

namespace contourtree {

/*
 * Assumes signed primitives
 */
template <class T>
class DisjointSets
{
public:
    std::vector<T> set;

public:
    DisjointSets(){}
    DisjointSets(uint64_t size);

    void merge(const T& ele1, const T& ele2);
    T find(const T& x);

private:
    void mergeSet(const T& root1, const T& root2);
};


template <class T>
DisjointSets<T>::DisjointSets(uint64_t size) {
    set.resize(size, (T)(-1));
}

template<class T>
void DisjointSets<T>::merge(const T &ele1, const T &ele2) {
    this->mergeSet(find(ele1),find(ele2));
}

/**
 * Perform a find with path compression.
 *
 * @param x
 *            the element being searched for.
 * @return the set containing x.
 */
template<class T>
T DisjointSets<T>::find(const T &x) {
    T f = set[x];
    if (f < 0) {
        return x;
    } else {
        int xx = find(f);
        set[x] = xx;
        return xx;
    }
}

/**
 * Union two disjoint sets using the height heuristic. root1 and root2 are
 * distinct and represent set names.
 *
 * @param root1
 *            the root of set 1.
 * @param root2
 *            the root of set 2.
 */
template <class T>
void DisjointSets<T>::mergeSet(const T& root1, const T& root2) {
    if (root1 == root2)
        return;

    int r1 = set[root1];
    int r2 = set[root2];

    if (r2 < r1) {
        set[root1] = root2;
    } else {
        if (r1 == r2) {
            // Update height if same
            r1--;
            set[root1] = r1;
        }
        // Make root1 new root
        set[root2] = root1;
    }
}

} // namespace


#endif // DISJOINTSETS_HPP

