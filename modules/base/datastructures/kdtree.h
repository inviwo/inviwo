/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2014-2015 Inviwo Foundation
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*********************************************************************************/

#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <vector>
#include <sstream>
#include <algorithm>

#include <inviwo/core/util/glm.h>

namespace inviwo {

    template <unsigned char N, typename T, typename P>
class KDTree;
template <unsigned char N, typename T, typename P>
class KDNode;

template <unsigned char N, typename T, typename P>
struct KDNodeDistWrapper {
    KDNode<N, T, P> *node;
    P sqDist;
    bool operator<(const KDNodeDistWrapper<N, T, P> &rhs) const { return sqDist > rhs.sqDist; }
};


template <unsigned char N, typename T = char, typename P = double>
class KDNode {
    friend class KDTree<N, T, P>;

    KDNode *leftChild_;
    KDNode *rightChild_;
    T data_;
    unsigned char dimmension_;
    P pos_[N];

    bool goRight(const P pos[N]) const;
    bool compare(const P pos[N]) const;

public:
    KDNode(const P pos[N], const T &data, KDNode *parent = 0);
    virtual ~KDNode();

    KDNode *clone();

    bool isLeaf() const;
    bool isRightLeaf() const;
    bool isLeftLeaf() const;

    unsigned long depth() const;
    unsigned long size() const;

    T &get();
    const T &get() const;
    T *getDataAsPointer() { return &data_; }
    P *getPosition();

    bool isOk() const;

    KDNode *findMin(unsigned int dim);
    KDNode *findMax(unsigned int dim);

    KDNode *insert(const P pos[N], const T &data);
    KDNode *find(const P pos[N]);
    KDNode *findParent(const KDNode *n);

    KDNode *getRightChild() { return rightChild_; }
    KDNode *getLeftChild() { return leftChild_; }
    std::pair<KDNode *, KDNode *> getChilds() {
        return std::pair<KDNode *, KDNode *>(leftChild_, rightChild_);
    }

    KDNode<N, T, P> *findNearest(const P pos[N], KDNode *nearest);
    void findNNearest(const P pos[N], size_t amount,
                      std::vector<KDNodeDistWrapper<N, T, P> > &current);
    void findCloseTo(const P pos[N], const P squaredDistance, std::vector<KDNode *> &nodes);

    static void swap(KDNode<N, T, P> *n0, KDNode<N, T, P> *n1, KDNode<N, T, P> *p0, KDNode<N, T, P> *p1);

    void getAsVector(std::vector<KDNode *> &nodes) {
        nodes.push_back(this);
        if (leftChild_) leftChild_->getAsVector(nodes);
        if (rightChild_) rightChild_->getAsVector(nodes);
    }
};

// using char as T since it is only 8 bit.
// the T should be optional (eg, if each node has some value in addition to the position)
template <unsigned char N, typename T = char, typename P = double>
class KDTree {
public:
    typedef KDNode<N, T, P> Node;

private:
    Node *root_;

public:
    KDTree();
    virtual ~KDTree();

    KDTree(const KDTree &tree) { root_ = tree.root_->clone(); }
    KDTree &operator=(const KDTree &rhs) {
        if (this != &rhs) {
            delete root_;
            root_ = rhs.root_->clone();
        }
        return *this;
    }

    void erase(Node *node, KDNode<N, T, P> *parent = nullptr);

    bool isOk() const;
    bool empty() const { return root_ == nullptr; }

    void clear();

    unsigned long depth() const;
    unsigned long size() const;

    std::vector<Node *> getAsVector();

    Node *getRoot() { return root_; }

    Node *findMin(unsigned int dim);
    Node *findMax(unsigned int dim);

    Node *insert(const P pos[N], const T &data);
    Node *find(const P pos[N]);

    Node *findNearest(const P pos[N]);
    std::vector<Node *> findCloseTo(const P pos[N], const P distance);
    std::vector<Node *> findNNearest(const P pos[N], int amount);
};

template <unsigned char N , typename T = char, typename P = double>
class KDTreeGlm : public KDTree < N, T, P> {
public:
    KDTreeGlm() : KDTree<N, T, P>() {}
    virtual ~KDTreeGlm() {}

    typedef KDNode<N, T, P> Node;
    typedef KDTree<N, T, P> Tree;

    Node *insert(const Vector<N, P> &pos, const T &data) {
        return Tree::insert(glm::value_ptr(pos), data);
    }
    Node *find(const Vector<N, P> &pos) {
        return Tree::find(glm::value_ptr(pos));
    }

    Node *findNearest(const Vector<N, P> &pos) {
        return Tree::findNearest(glm::value_ptr(pos));
    }

    Node *findNearest(const Vector<N, P> &pos, const int &amount) {
        return Tree::findNNearest(glm::value_ptr(pos), amount);
    }

    std::vector<Node*> findCloseTo(const Vector<N, P> &pos, const P distance) {
        return Tree::findCloseTo(glm::value_ptr(pos), distance);
    }
};

template <typename T = char, typename P = double>
using K2DTree = KDTreeGlm < 2, T, P > ;
template <typename T = char, typename P = double>
using K3DTree = KDTreeGlm < 3, T, P > ;
template <typename T = char, typename P = double>
using K4DTree = KDTreeGlm < 4, T, P >;


template <unsigned char N, typename P = double>
Vector<N, P>  ptrToVec(const P* f){ 
    Vector<N, P> vec;
    for (size_t i = 0; i < N; ++i) vec[i] = f[i];
    return vec; 
}

// Tree Implementation

template <unsigned char N, typename T, typename P>
KDTree<N, T, P>::KDTree()
    : root_(nullptr) {}

template <unsigned char N, typename T, typename P>
KDTree<N, T, P>::~KDTree() {
    delete root_;
}

template <unsigned char N, typename T, typename P>
unsigned long KDTree<N, T, P>::depth() const {
    if (root_ == 0) return 0;
    return root_->depth();
}

template <unsigned char N, typename T, typename P>
unsigned long KDTree<N, T, P>::size() const {
    if (root_ == 0) return 0;
    return root_->size();
}

template <unsigned char N, typename T, typename P>
bool KDTree<N, T, P>::isOk() const {
    if (root_ == 0) return true;
    return root_->isOk();
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDTree<N, T, P>::findMin(unsigned int d) {
    if (root_ == 0) return 0;
    return root_->findMin(d);
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDTree<N, T, P>::findMax(unsigned int d) {
    if (root_ == 0) return 0;
    return root_->findMax(d);
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDTree<N, T, P>::insert(const P pos[N], const T &data) {
    if (root_ == 0) {
        root_ = new KDNode<N, T, P>(pos, data);
        return root_;
    }

    return root_->insert(pos, data);
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDTree<N, T, P>::find(const P pos[N]) {
    if (root_ == 0) return 0;
    return root_->find(pos);
}


template <unsigned char N, typename T, typename P>
void KDTree<N, T, P>::erase(KDNode<N, T, P> *node, KDNode<N, T, P> *parent) {
    if (node == 0) {
        std::cerr << "Trying to delete a null node" << std::endl;
        return;
    }

    if (node->isLeaf()) {
        if (node == root_) {  // Root node;
            root_ = 0;
        }
        else {
            if (!parent)
                parent = root_->findParent(node);

            if (parent->leftChild_ == node) {
                parent->leftChild_ = 0;
            }
            else if (parent->rightChild_ == node) {
                parent->rightChild_ = 0;
            }
        }
        delete node;
        return;
    }

    KDNode<N, T, P> *min;
    if (!parent)
        parent = root_->findParent(node);

    if (!node->isRightLeaf()) {
        min = node->rightChild_->findMin(node->dimmension_);
        KDNode<N, T, P>::swap(node, min, parent, node->findParent(min));
        if (root_ == node) root_ = min;
        erase(node);
    } else {
        min = node->leftChild_->findMin(node->dimmension_);
        KDNode<N, T, P>::swap(node, min, parent, node->findParent(min));
        std::swap(min->rightChild_, min->leftChild_);
        if (root_ == node) root_ = min;
        erase(node);
    }
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDTree<N, T, P>::findNearest(const P pos[N]) {
    if (root_ == 0) return 0;
    return root_->findNearest(pos, 0);
}

template <unsigned char N, typename T, typename P>
std::vector<KDNode<N, T, P> *> KDTree<N, T, P>::findCloseTo(const P pos[N], P distance) {
    std::vector<KDNode<N, T, P> *> nodes;
    if (root_ == 0) return nodes;
    root_->findCloseTo(pos, distance * distance, nodes);
    return nodes;
}

template <unsigned char N, typename T, typename P>
std::vector<KDNode<N, T, P> *> KDTree<N, T, P>::getAsVector() {
    std::vector<Node *> v;
    if (root_) root_->getAsVector(v);
    return v;
}

template <unsigned char N, typename T, typename P>
void KDTree<N, T, P>::clear() {
    if (root_ == 0) return;
    delete root_;
    root_ = nullptr;
}

template <unsigned char N, typename T, typename P>
std::vector<KDNode<N, T, P> *> KDTree<N, T, P>::findNNearest(const P pos[N], int amount) {
    std::vector<KDNodeDistWrapper<N, T, P> > nearnodes;
    std::vector<Node *> nodes;
    if (root_ == 0) return nodes;
    root_->findNNearest(pos, amount, nearnodes);
    for (size_t i = 0; i < nearnodes.size(); ++i) {
        nodes.push_back(nearnodes[i].node);
    }
    return nodes;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P>::KDNode(const P pos[N], const T &data, KDNode *parent)
    : leftChild_(0), rightChild_(0), data_(data) {
    for (size_t i = 0; i < N; i++) {
        pos_[i] = pos[i];
    }
    dimmension_ = parent == 0 ? 0 : (parent->dimmension_ + 1) % N;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P>::~KDNode() {
    delete leftChild_;
    delete rightChild_;
}
template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::clone() {
    KDNode<N, T, P> *newNode = new KDNode<N, T, P>(pos_, data_, 0);
    newNode->dimmension_ = dimmension_;
    if (leftChild_) {
        newNode->leftChild_ = leftChild_->clone();
    }
    if (rightChild_) {
        newNode->rightChild_ = rightChild_->clone();
    }
    return newNode;
}

template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::isOk() const {
    if (isLeaf()) return true;
    if (!isLeftLeaf() && goRight(leftChild_->pos_)) {
        std::cout << "This: " << this << std::endl;
        std::cout << "leftChild_: " << leftChild_ << std::endl;
        return false;
    }
    if (!isRightLeaf() && !goRight(rightChild_->pos_)) {
        std::cout << "This: " << this << std::endl;
        std::cout << "rightChild_: " << rightChild_ << std::endl;
        return false;
    }

    if (!isLeftLeaf() && !leftChild_->isOk()) return false;

    if (!isRightLeaf() && !rightChild_->isOk()) return false;

    return true;
}

template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::isLeaf() const {
    return isRightLeaf() && isLeftLeaf();
}
template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::isRightLeaf() const {
    return rightChild_ == 0;
}
template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::isLeftLeaf() const {
    return leftChild_ == 0;
}

template <unsigned char N, typename T, typename P>
unsigned long KDNode<N, T, P>::depth() const {
    unsigned long l = 0, r = 0;
    if (!isRightLeaf()) r = rightChild_->depth();
    if (!isLeftLeaf()) l = leftChild_->depth();
    return 1 + std::max(l, r);
}

template <unsigned char N, typename T, typename P>
unsigned long KDNode<N, T, P>::size() const {
    unsigned long l = 0, r = 0;
    if (!isRightLeaf()) r = rightChild_->size();
    if (!isLeftLeaf()) l = leftChild_->size();
    return 1 + l + r;
}

template <unsigned char N, typename T, typename P>
T &KDNode<N, T, P>::get() {
    return data_;
}

template <unsigned char N, typename T, typename P>
const T &KDNode<N, T, P>::get() const {
    return data_;
}

template <unsigned char N, typename T, typename P>
P *KDNode<N, T, P>::getPosition() {
    return pos_;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::insert(const P pos[N], const T &data) {
    bool right = goRight(pos);
    if (right) {
        if (rightChild_ == 0) {
            rightChild_ = new KDNode(pos, data, this);
            return rightChild_;
        }
        return rightChild_->insert(pos, data);
    } else {
        if (leftChild_ == 0) {
            leftChild_ = new KDNode(pos, data, this);
            return leftChild_;
        }
        return leftChild_->insert(pos, data);
    }
}

template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::goRight(const P pos[N]) const {
    return pos[dimmension_] >= pos_[dimmension_];
}

template <unsigned char N, typename T, typename P>
bool KDNode<N, T, P>::compare(const P pos[N]) const {
    for (unsigned int i = 0; i < N; i++) {
        if (pos[i] != pos_[i]) return false;
    }
    return true;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::findMin(unsigned int d) {
    if (dimmension_ == d) {
        return isLeftLeaf() ? this : leftChild_->findMin(d);
    }
    if (isLeaf())  // leaf node
        return this;

    KDNode *l, *r, *minChild;
    if (isLeftLeaf()) {  // if left is empty, minimum value of dimensions d is either this node or
                         // in right subtree
        minChild = rightChild_->findMin(d);
    } else if (isRightLeaf())
        minChild = leftChild_->findMin(d);
    else {
        l = leftChild_->findMin(d);
        r = rightChild_->findMin(d);
        minChild = (l->pos_[d] <= r->pos_[d]) ? l : r;
    }
    return (minChild->pos_[d] <= pos_[d]) ? minChild : this;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::findMax(unsigned int d) {
    if (dimmension_ == d) {
        return isRightLeaf() ? this : rightChild_->findMax(d);
    }
    if (isLeaf()) return this;

    KDNode *l, *r, *maxChild;
    if (isLeftLeaf()) {  // if left is empty, minimum value of dimensions d is either this node or
                         // in right subtree
        maxChild = rightChild_->findMax(d);
    } else if (isRightLeaf())
        maxChild = leftChild_->findMax(d);
    else {
        l = leftChild_->findMax(d);
        r = rightChild_->findMax(d);
        maxChild = (l->pos_[d] >= r->pos_[d]) ? l : r;
    }
    return (maxChild->pos_[d] >= pos_[d]) ? maxChild : this;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::find(const P pos[N]) {
    bool found = true;
    for (unsigned char i = 0; i < N && found; i++) {
        found &= pos_[i] == pos[i];
    }
    if (found) {
        return this;
    }

    if (goRight(pos)) {
        return isRightLeaf() ? nullptr : rightChild_->find(pos);
    } else {
        return isLeftLeaf() ? nullptr : leftChild_->find(pos);
    }
}



template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::findParent(const KDNode *n) {
    if (leftChild_ == n) return this;
    if (rightChild_ == n) return this;
    if (goRight(n->pos_)) {
        return isRightLeaf() ? nullptr : rightChild_->findParent(n);
    }
    else {
        return isLeftLeaf() ? nullptr : leftChild_->findParent(n);
    }
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *__closestTo(const P pos[N], KDNode<N, T, P> *n0, KDNode<N, T, P> *n1) {
    if (n0 == 0) return n1;
    if (n1 == 0 || n0 == n1) return n0;
    P d0 = 0, d1 = 0, dx;
    for (size_t i = 0; i < N; i++) {
        dx = pos[i] - n0->getPosition()[i];
        d0 += dx * dx;

        dx = pos[i] - n1->getPosition()[i];
        d1 += dx * dx;
    }
    /*
    this is not needed
    d0 = std::sqrt(d0);
    d1 = std::sqrt(d1);
    */
    return (d0 <= d1) ? n0 : n1;
}

template <unsigned char N, typename T, typename P>
KDNode<N, T, P> *KDNode<N, T, P>::findNearest(const P pos[N], KDNode<N, T, P> *nearest) {
    if (isLeaf()) {
        nearest = __closestTo<N, T, P>(pos, this, nearest);
        return nearest;
    }

    P d0, d1;
    if (goRight(pos)) {  // pos is within right sub tree
        if (!isRightLeaf()) {  // Look forit in right sub tree
            nearest = __closestTo<N, T, P>(pos, nearest, rightChild_->findNearest(pos, nearest));
        }
        nearest = __closestTo<N, T, P>(pos, nearest, this);
        if (nearest == this && !isLeftLeaf()) {  // if current node is the closest in
            nearest = __closestTo<N, T, P>(pos, this, leftChild_->findNearest(pos, nearest));
        } else if (!isLeftLeaf()) {  // can it
            d0 = pos[dimmension_] - this->pos_[dimmension_];
            d1 = pos[dimmension_] - nearest->pos_[dimmension_];
            d0 = d0 * d0;
            d1 = d1 * d1;

            if (d0 <= d1) {
                nearest = __closestTo<N, T, P>(pos, nearest, leftChild_->findNearest(pos, nearest));
            }
        }
    } else {
        if (!isLeftLeaf()) {  // Look forit in right sub tree
            nearest = __closestTo<N, T, P>(pos, nearest, leftChild_->findNearest(pos, nearest));
        }
        nearest = __closestTo<N, T, P>(pos, nearest, this);
        if (nearest == this && !isRightLeaf()) {  // if current node is the closest in
            nearest = __closestTo<N, T, P>(pos, this, rightChild_->findNearest(pos, nearest));
        } else if (!isRightLeaf()) {  // can it
            d0 = pos[dimmension_] - this->pos_[dimmension_];
            d1 = pos[dimmension_] - nearest->pos_[dimmension_];
            d0 = d0 * d0;
            d1 = d1 * d1;

            if (d0 < d1) {
                nearest =
                    __closestTo<N, T, P>(pos, nearest, rightChild_->findNearest(pos, nearest));
            }
        }
    }
    return nearest;
}

template <unsigned char N, typename T, typename P>
P __sqDist(const P p0[N], const P p1[N]) {
    P d = 0, a;
    for (size_t i = 0; i < N; i++) {
        a = p0[i] - p1[i];
        d += a * a;
    }
    return d;
}

template <unsigned char N, typename T, typename P>
void KDNode<N, T, P>::findCloseTo(const P pos[N], const P sqDist, std::vector<KDNode *> &nodes) {
    if (__sqDist<N, T, P>(pos, pos_) < sqDist) {
        nodes.push_back(this);
    }
    P d;
    bool right = false, left = false;
    // BUG ??
    if (goRight(pos)) {
        if (!isRightLeaf()) {
            right = true;
        }
        if (!isLeftLeaf()) {
            d = pos_[dimmension_] - pos[dimmension_];
            if (d * d <= sqDist) left = true;
        }
    } else {
        if (!left && !isLeftLeaf()) {
            left = true;
        }
        if (!right && !isRightLeaf()) {
            d = pos_[dimmension_] - pos[dimmension_];
            if (d * d <= sqDist) right = true;
        }
    }
    if (right) {
        rightChild_->findCloseTo(pos, sqDist, nodes);
    }
    if (left) {
        leftChild_->findCloseTo(pos, sqDist, nodes);
    }
}

#define __FF__ std::cout << "Failed at " << __FILE__ << "@" << __LINE__ << std::endl
template <unsigned char N, typename T, typename P>
void KDNode<N, T, P>::swap(KDNode<N, T, P> *n0, KDNode<N, T, P> *n1, KDNode<N, T, P> *p0, KDNode<N, T, P> *p1) {
    if (n0 == n1 || n0 == 0 || n1 == 0) return;

    KDNode<N, T, P> *r0 = n0->rightChild_;
    KDNode<N, T, P> *l0 = n0->leftChild_;
    KDNode<N, T, P> *r1 = n1->rightChild_;
    KDNode<N, T, P> *l1 = n1->leftChild_;

    if (p0 == n1) {  // if n0 is a child to n1
        swap(n1, n0 , p1 , p0);  // swap in "other" direction
        return;
    }
    if (p1 == n0) {  // n1 is a child to n0
        bool left = l0 == n1;
        n0->leftChild_ = l1;
        n0->rightChild_ = r1;

        n1->leftChild_ = left ? n0 : l0;
        n1->rightChild_ = left ? r0 : n0;

        
        if (p0 != 0) {
            if (p0->leftChild_ == n0) p0->leftChild_ = n1;    //
            if (p0->rightChild_ == n0) p0->rightChild_ = n1;  //
        }
    } else {
        n0->leftChild_ = l1;
        n0->rightChild_ = r1;
        n1->leftChild_ = l0;
        n1->rightChild_ = r0;

        if (p0 != 0 && p0->leftChild_ == n0) {
            p0->leftChild_ = n1;
        } else if (p0 != 0 && p0->rightChild_ == n0) {
            p0->rightChild_ = n1;
        } else if (p0 != 0) {
            __FF__;
        }

        if (p1->leftChild_ == n1) {
            p1->leftChild_ = n0;
        } else if (p1->rightChild_ == n1) {
            p1->rightChild_ = n0;
        } else {
            __FF__;
        }
    }

    std::swap(n0->dimmension_, n1->dimmension_);
}

template <unsigned char N, typename T, typename P>
void KDNode<N, T, P>::findNNearest(const P pos[N], size_t amount,
                                   std::vector<KDNodeDistWrapper<N, T, P> > &current) {
    P sqDist = __sqDist<N, T, P>(pos, pos_);
    if (current.size() < amount) {
        KDNodeDistWrapper<N, T, P> holder;
        holder.node = this;
        holder.sqDist = sqDist;
        current.push_back(holder);
        std::sort(current.begin(), current.end());
    } else if (sqDist <= current[0].sqDist) {
        KDNodeDistWrapper<N, T, P> holder;
        holder.node = this;
        holder.sqDist = sqDist;
        current.push_back(holder);
        std::sort(current.begin(), current.end());
        while (current.size() > amount) {
            current.erase(current.begin());
        }
    }

    P d;
    bool right = false, left = false;
    if (current.size() < amount) {
        left = !isLeftLeaf();
        right = !isRightLeaf();
    } else if (goRight(pos)) {
        if (!isRightLeaf()) {
            right = true;
        }
        if (!isLeftLeaf()) {
            d = pos_[dimmension_] - pos[dimmension_];
            if (d * d <= current[0].sqDist) left = true;
        }
    } else {
        if (!left && !isLeftLeaf()) {
            left = true;
        }
        if (!right && !isRightLeaf()) {
            d = pos_[dimmension_] - pos[dimmension_];
            if (d * d <= current[0].sqDist) right = true;
        }
    }
    if (right) {
        rightChild_->findNNearest(pos, amount, current);
    }
    if (left) {
        leftChild_->findNNearest(pos, amount, current);
    }
}
}
#endif
