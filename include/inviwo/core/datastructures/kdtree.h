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
#include <inviwo/core/common/inviwo.h>

#define KD_TYPE dimmensions,dataType,floatPrecision
#define KD_TEMPLATE template <unsigned int dimmensions , typename dataType,typename floatPrecision>
#define KD_TREE        KDTree<KD_TYPE>
#define KD_NODE        KDNode<KD_TYPE>

namespace inviwo{

KD_TEMPLATE class KDTree;
KD_TEMPLATE class KDNode;

KD_TEMPLATE struct __KD_NEAR_NODE__{
    KD_NODE *node;
    float sqDist;
    bool operator<(const __KD_NEAR_NODE__<KD_TYPE> &rhs) const { return sqDist > rhs.sqDist; }
};

template <unsigned int dimmensions, typename dataType = char, typename floatPrecision = double>
class KDNode {
    friend class KD_TREE;
    KDNode *_parent;
    KDNode *_left;
    KDNode *_right;
    dataType _data;
    unsigned int _dimmension;
    floatPrecision _pos[dimmensions];

    bool _goRight(const floatPrecision pos[dimmensions])const;
    bool _compare(const floatPrecision pos[dimmensions])const;
public:
    KDNode(const floatPrecision pos[dimmensions],
           const dataType &data,KDNode *parent = 0);
    virtual ~KDNode();

    KDNode* clone();
    
    bool isLeaf()const;
    bool isRightLeaf()const;
    bool isLeftLeaf()const;

    unsigned long depth()const;
    unsigned long size()const;

    dataType &get();
    const dataType &get()const;
    dataType *getDataAsPointer(){return &_data;}
    floatPrecision* getPosition();

    bool isOk()const;

    KDNode *findMin(unsigned int dim);
    KDNode *findMax(unsigned int dim);

    KDNode *insert(const floatPrecision pos[dimmensions], const dataType &data);
    KDNode *find(const floatPrecision pos[dimmensions]);
    
    KDNode *getRightChild(){return _right;}
    KDNode *getLeftChild(){return _left;}
    std::pair<KDNode*,KDNode*> getChilds() {
        return std::pair<KDNode*, KDNode*>(_left,_right);
    }

    KD_NODE* findNearest(const floatPrecision pos[dimmensions], KDNode *nearest);
    void findNNearest(const floatPrecision pos[dimmensions], 
                      size_t amount, 
                      std::vector<__KD_NEAR_NODE__<KD_TYPE> > &current);
    void findCloseTo(const floatPrecision pos[dimmensions], 
                     const floatPrecision squaredDistance, 
                     std::vector<KDNode*> &nodes);

    static void swap(KD_NODE* n0,KD_NODE* n1);

    void getAsVector(std::vector<KDNode*> &nodes){
        nodes.push_back(this);
        if(_left) _left->getAsVector(nodes);
        if(_right) _right->getAsVector(nodes);
    }

    virtual std::string toString() const;
};


//using char as datatype since it is only 8 bit.  
//the dataType should be optional (eg, if each node has some value in addition to the position) 
template <unsigned int dimmensions, typename dataType = char, typename floatPrecision = double> 
class KDTree {
public:
    typedef KDNode<KD_TYPE> Node;
private:
    Node *root;
public:
    KDTree();
    virtual ~KDTree();

    KDTree(const KDTree &tree){
        root = tree.root->clone();
    }
    KDTree& operator=( const KDTree& rhs ){
        if(this != &rhs){
            delete root;
            root = rhs.root->clone();
        }
        return *this;
    }

    void erase(Node *node);

    bool isOk()const;
    bool empty()const{return root==0;}

    void clear();

    unsigned long depth()const;
    unsigned long size()const;
    
    std::vector<Node*> getAsVector();

    Node *getRoot(){return root;}

    Node* findMin(unsigned int dim);
    Node* findMax(unsigned int dim);

    Node *insert(const floatPrecision pos[dimmensions], const dataType &data);
    Node *find(const floatPrecision pos[dimmensions]);
    
    Node *findNearest(const floatPrecision pos[dimmensions]);
    std::vector<Node*> findCloseTo(const floatPrecision pos[dimmensions], const floatPrecision distance);
    std::vector<Node*> findNNearest(const floatPrecision pos[dimmensions], int amount);

    virtual std::string toString() const;
};


template <typename dataType = char,typename floatPrecision = float>
class K2DTree : public KDTree<2, dataType, floatPrecision> {
public:
    K2DTree() : KDTree<2, dataType, floatPrecision>() {
    }
    virtual ~K2DTree(){}

    typedef KDNode<2, dataType, floatPrecision> Node;
    typedef KDTree<2, dataType, floatPrecision> Tree;

    Node *insert(const glm::vec2 pos, const dataType &data){
        return Tree::insert((const float *)glm::value_ptr(pos), data);
    }
    Node *find(const glm::vec2 pos){
        return Tree::find((const float *)glm::value_ptr(pos));
    }

    Node *findNearest(const glm::vec2 pos){
        return Tree::findNearest((const float *)glm::value_ptr(pos));
    }

    Node *findNearest(const glm::vec2 pos, const int &amount){
        return Tree::findNNearest((const float *)glm::value_ptr(pos), amount);
    }

    std::vector<Node*> findCloseTo(const glm::vec2 pos, const floatPrecision distance){
        return Tree::findCloseTo((const float *)glm::value_ptr(pos), distance);
    }
};

template <typename dataType = char,typename floatPrecision = float>
class K3DTree : public KDTree<3,dataType,floatPrecision> {
public:
    K3DTree() : KDTree<3,dataType,floatPrecision>() {}
    virtual ~K3DTree(){}
    
    typedef KDNode<3, dataType, floatPrecision> Node;
    typedef KDTree<3, dataType, floatPrecision> Tree;

    Node *insert(const glm::vec3 pos, const dataType &data){
        return Tree::insert((const float *)glm::value_ptr(pos), data);
    }
    Node *find(const glm::vec3 pos){
        return Tree::find((const float *)glm::value_ptr(pos));
    }
    
    Node *findNearest(const glm::vec3 pos){
        return Tree::findNearest((const float *)glm::value_ptr(pos));
    }

    Node *findNearest(const glm::vec3 pos, const int &amount){
        return Tree::findNNearest((const float *)glm::value_ptr(pos), amount);
    }

    std::vector<Node*> findCloseTo(const glm::vec3 pos, const floatPrecision distance){
        return Tree::findCloseTo((const float *)glm::value_ptr(pos), distance);
    }
};

template <typename dataType = char,typename floatPrecision = float>
class K4DTree : public KDTree<4, dataType, floatPrecision> {
public:
    K4DTree() : KDTree<4, dataType, floatPrecision>() {}
    virtual ~K4DTree(){}

    typedef KDNode<4, dataType, floatPrecision> Node;
    typedef KDTree<4, dataType, floatPrecision> Tree;

    Node *insert(const glm::vec4 pos, const dataType &data){
        return Tree::insert((const float *)glm::value_ptr(pos), data);
    }
    Node *find(const glm::vec4 pos){
        return Tree::find((const float *)glm::value_ptr(pos));
    }
    
    Node *findNearest(const glm::vec4 pos){
        return Tree::findNearest((const float *)glm::value_ptr(pos));
    }

    Node *findNearest(const glm::vec4 pos, const int &amount){
        return Tree::findNNearest((const float *)glm::value_ptr(pos), amount);
    }

    std::vector<Node*> findCloseTo(const glm::vec4 pos, const floatPrecision distance){
        return Tree::findCloseTo((const float *)glm::value_ptr(pos), distance);
    }
};

inline glm::vec2 floatPtrToVec2(const float *f){return glm::vec2(f[0],f[1]);}
inline glm::vec3 floatPtrToVec3(const float *f){return glm::vec3(f[0],f[1],f[2]);}
inline glm::vec4 floatPtrToVec4(const float *f){return glm::vec4(f[0],f[1],f[2],f[3]);}

}

// Tree Implementation

#ifndef _KDTREE_IMP_
#define _KDTREE_IMP_

#include <inviwo/core/util/logcentral.h>

namespace inviwo{

KD_TEMPLATE KD_TREE::KDTree(){
    root = 0;
}

KD_TEMPLATE KD_TREE::~KDTree(){
    delete root;
}

KD_TEMPLATE unsigned long KD_TREE::depth()const{
    if(root == 0)
        return 0;
    return root->depth();
}

KD_TEMPLATE unsigned long KD_TREE::size()const{
    if(root == 0)
        return 0;
    return root->size();
}

KD_TEMPLATE bool KD_TREE::isOk()const{
    if(root == 0)
        return true;
    return root->isOk();
}

KD_TEMPLATE KD_NODE* KD_TREE::findMin(unsigned int d){
    if(root == 0)
        return 0;
    return root->findMin(d);
}

KD_TEMPLATE KD_NODE* KD_TREE::findMax(unsigned int d){
    if(root == 0)
        return 0;
    return root->findMax(d);
}

KD_TEMPLATE KD_NODE* KD_TREE::insert(const floatPrecision pos[dimmensions], const dataType &data){
    if(root == 0){
        root = new KD_NODE(pos,data);
        return root;
    }

    return root->insert(pos,data);
}

KD_TEMPLATE KD_NODE* KD_TREE::find(const floatPrecision pos[dimmensions]){
    if(root == 0)
        return 0;
    return root->find(pos);
}

KD_TEMPLATE void KD_TREE::erase(KD_NODE *node){
    if(node == 0){
        std::cerr << "Trying to delete a null node" << std::endl;
        assert(node != 0);
        return;
    }

    if(node->isLeaf()){
        KD_NODE* parent = node->_parent;
        if(parent == 0){ //Root node;
            root = 0;
        }
        else if(parent->_left == node){
            parent->_left = 0;
        }else if(parent->_right == node){
            parent->_right = 0;
        }else{
            std::cerr << "Incorrect parent or something" << __FILE__ << "@" << __LINE__ << std::endl;
            assert(false);
            return;
        }
        delete node;
        return;
    }
    
    KD_NODE* max;
    if(!node->isRightLeaf()){
        max = node->_right->findMin(node->_dimmension);
        KD_NODE::swap(node,max);
        if(root == node) root = max;
        erase(node);
        return;
    }
    else{
        max = node->_left->findMin(node->_dimmension);
        KD_NODE::swap(node,max);
        std::swap(max->_right , max->_left);
        if(root == node) root = max;
        erase(node);
        return;
    }
}

KD_TEMPLATE KD_NODE * KD_TREE::findNearest(const floatPrecision pos[dimmensions]){
    if(root == 0)
        return 0;    
    return root->findNearest(pos,0);
}

KD_TEMPLATE std::vector<KD_NODE*> KD_TREE::findCloseTo(const floatPrecision pos[dimmensions],floatPrecision distance){
    std::vector<KD_NODE*> nodes;
    if(root == 0)
        return nodes;
    root->findCloseTo(pos,distance*distance,nodes);
    return nodes;
}

KD_TEMPLATE std::string KD_TREE::toString()const{
    std::stringstream ss;
    ss << typeid(*this).name() << " " << ((void *)this);
    return ss.str();
}


KD_TEMPLATE std::vector<KD_NODE*> KD_TREE::getAsVector(){
    std::vector<Node*> v;
    if(root) root->getAsVector(v);
    return v;
}


KD_TEMPLATE void KD_TREE::clear(){
    if(root == 0) return;
    delete root;
    root = 0;
}

KD_TEMPLATE std::vector<KD_NODE*> KD_TREE::findNNearest(const floatPrecision pos[dimmensions],int amount){
    std::vector<__KD_NEAR_NODE__<KD_TYPE> > nearnodes;
    std::vector<Node*> nodes;
    if(root == 0)
        return nodes;
    root->findNNearest(pos,amount,nearnodes);
    for(size_t i = 0;i<nearnodes.size();++i){
        nodes.push_back(nearnodes[i].node);
    }
    return nodes;
}


}
#endif

// Node Implementation

#ifndef _KDNODE_IMP_
#define _KDNODE_IMP_

#include <algorithm>

namespace inviwo{

KD_TEMPLATE KD_NODE::KDNode(const floatPrecision pos[dimmensions],const dataType &data,KDNode *parent)
    : _parent(parent)
    , _left(0)
    , _right(0)
    , _data(data) {
    
    for(size_t i = 0;i<dimmensions;i++){
        _pos[i] = pos[i];
    }
    _dimmension = _parent == 0 ? 0 : (_parent->_dimmension+1)%dimmensions;
}

KD_TEMPLATE KD_NODE::~KDNode(){
    delete _left;
    delete _right;
}
KD_TEMPLATE KD_NODE* KD_NODE::clone(){
    KD_NODE* newNode = new KD_NODE(_pos,_data,0);
    newNode->_dimmension = _dimmension;
    if(_left){
        newNode->_left = _left->clone();
        newNode->_left->_parent = newNode;
    }
    if(_right){
        newNode->_right = _right->clone();
        newNode->_right->_parent = newNode;
    }
    return newNode;
}

KD_TEMPLATE bool KD_NODE::isOk()const{
    if(isLeaf())
        return true;
    if(!isLeftLeaf() && _goRight(_left->_pos)){
        std::cout << "This: " << this << std::endl;
        std::cout << "_left: " << _left << std::endl;
        return false;
    }
    if(!isRightLeaf() && !_goRight(_right->_pos)){
        std::cout << "This: " << this << std::endl;
        std::cout << "_right: " << _right << std::endl;
        return false;
    }

    if(!isLeftLeaf() && !_left->isOk())
        return false;

    if(!isRightLeaf() && !_right->isOk())
        return false;

    return true;
}

KD_TEMPLATE bool KD_NODE::isLeaf()const{
    return isRightLeaf() && isLeftLeaf();
}
KD_TEMPLATE bool KD_NODE::isRightLeaf()const{
    return _right == 0;
}
KD_TEMPLATE bool KD_NODE::isLeftLeaf()const{
    return _left == 0;
}


KD_TEMPLATE unsigned long KD_NODE::depth()const{
    unsigned long l = 0,r = 0;
    if(!isRightLeaf())
        r = _right->depth();
    if(!isLeftLeaf())
        l = _left->depth();
    return 1 + std::max(l,r);

}

KD_TEMPLATE unsigned long KD_NODE::size()const{
    unsigned long l = 0,r = 0;
    if(!isRightLeaf())
        r = _right->size();
    if(!isLeftLeaf())
        l = _left->size();
    return 1 + l + r;
}

KD_TEMPLATE dataType &KD_NODE::get(){
    return _data;
}

KD_TEMPLATE const dataType &KD_NODE::get()const{
    return _data;
}

KD_TEMPLATE floatPrecision* KD_NODE::getPosition(){
    return _pos;
}

KD_TEMPLATE
KD_NODE* KD_NODE::insert(const floatPrecision pos[dimmensions], const dataType &data){
    bool right = _goRight(pos);
    if(right){
        if(_right == 0){
            _right = new KDNode(pos,data,this);    
            return _right;
        }
        return _right->insert(pos,data);
    }
    else{
        if(_left == 0){
            _left = new KDNode(pos,data,this);    
            return _left;
        }
        return _left->insert(pos,data);
    }
}

KD_TEMPLATE bool KD_NODE::_goRight(const floatPrecision pos[dimmensions])const{
    return pos[_dimmension] >= _pos[_dimmension];
}

KD_TEMPLATE bool KD_NODE::_compare(const floatPrecision pos[dimmensions])const{
    for(unsigned int i = 0;i<dimmensions;i++){
        if(pos[i] != _pos[i])
            return false;
    }
    return true;
}

KD_TEMPLATE KD_NODE* KD_NODE::findMin(unsigned int d){
    if(_dimmension == d){
        return isLeftLeaf() ? this : _left->findMin(d);
    }
    if(isLeaf()) // leaf node
        return this;

    KDNode *l,*r,*minChild;
    if(isLeftLeaf()){ //if left is empty, minimum value of dimensions d is either this node or in right subtree
        minChild = _right->findMin(d);
    }else if(isRightLeaf())
        minChild = _left->findMin(d);
    else{
        l = _left->findMin(d);
        r = _right->findMin(d);
        minChild = (l->_pos[d] <= r->_pos[d]) ? l : r;
    }
    return (minChild->_pos[d] <= _pos[d]) ? minChild : this;
}


KD_TEMPLATE KD_NODE* KD_NODE::findMax(unsigned int d){
    if(_dimmension == d){
        return isRightLeaf() ? this : _right->findMax(d);
    }
    if(isLeaf())
        return this;
    

    KDNode *l,*r,*maxChild;
    if(isLeftLeaf()){ //if left is empty, minimum value of dimensions d is either this node or in right subtree
        maxChild = _right->findMax(d);
    }else if(isRightLeaf())
        maxChild = _left->findMax(d);
    else{
        l = _left->findMax(d);
        r = _right->findMax(d);
        maxChild = (l->_pos[d] >= r->_pos[d]) ? l : r;
    }
    return (maxChild->_pos[d] >= _pos[d]) ? maxChild : this;
}

KD_TEMPLATE KD_NODE* KD_NODE::find(const floatPrecision pos[dimmensions]){
    if(_compare(pos))
        return this;
    if(_goRight(pos)){
        return isRightLeaf() ? 0 : _right->find(pos);
    }else{
        return isLeftLeaf()  ? 0 : _left->find(pos);
    }
}


KD_TEMPLATE KD_NODE* __closestTo(const floatPrecision pos[dimmensions],KD_NODE *n0,KD_NODE *n1){
    if(n0 == 0)
        return n1;
    if(n1 == 0 || n0 == n1)
        return n0;
    float d0 = 0,d1 = 0,dx;
    for(size_t i = 0;i<dimmensions;i++){
        dx = pos[i] - n0->getPosition()[i];
        d0 += dx*dx;
        
        dx = pos[i] - n1->getPosition()[i];
        d1 += dx*dx;
    }
    /* 
    this is not needed
    d0 = std::sqrt(d0);
    d1 = std::sqrt(d1);
    */
    return (d0 <= d1) ? n0 : n1;
}


KD_TEMPLATE KD_NODE* KD_NODE::findNearest(const floatPrecision pos[dimmensions],KD_NODE *nearest){
    if(isLeaf()){
        nearest = __closestTo<KD_TYPE>(pos,this,nearest);
        return nearest;
    }

    float d0,d1;
    if(_goRight(pos)){  //pos is within right sub tree
        if(!isRightLeaf()){ //Look forit in right sub tree
            nearest = __closestTo<KD_TYPE>(pos,nearest,_right->findNearest(pos,nearest));
        }
        nearest = __closestTo<KD_TYPE>(pos,nearest,this);
        if(nearest == this && !isLeftLeaf()){//if current node is the closest in 
            nearest = __closestTo<KD_TYPE>(pos,this,_left->findNearest(pos,nearest));
        }
        else if(!isLeftLeaf()){ //can it 
            d0 = pos[_dimmension] - this->_pos[_dimmension];
            d1 = pos[_dimmension] - nearest->_pos[_dimmension];
            d0 = d0*d0;
            d1 = d1*d1;

            if(d0<=d1){
                nearest = __closestTo<KD_TYPE>(pos,nearest,_left->findNearest(pos,nearest));
            }    
        }
    }
    else{ 
        if(!isLeftLeaf()){ //Look forit in right sub tree
            nearest = __closestTo<KD_TYPE>(pos,nearest,_left->findNearest(pos,nearest));
        }
        nearest = __closestTo<KD_TYPE>(pos,nearest,this);
        if(nearest == this && !isRightLeaf()){//if current node is the closest in 
            nearest = __closestTo<KD_TYPE>(pos,this,_right->findNearest(pos,nearest));
        }
        else if(!isRightLeaf()){ //can it 
            d0 = pos[_dimmension] - this->_pos[_dimmension];
            d1 = pos[_dimmension] - nearest->_pos[_dimmension];
            d0 = d0*d0;
            d1 = d1*d1;

            if(d0<d1){
                nearest = __closestTo<KD_TYPE>(pos,nearest,_right->findNearest(pos,nearest));
            }    
        }
    }
    return nearest;
}

KD_TEMPLATE floatPrecision __sqDist(const floatPrecision p0[dimmensions],const floatPrecision p1[dimmensions]){
    floatPrecision d = 0,a;
    for(size_t i = 0;i<dimmensions;i++){
        a = p0[i] - p1[i];
        d += a*a;
    }
    return d;
}

KD_TEMPLATE void KD_NODE::findCloseTo(const floatPrecision pos[dimmensions],const floatPrecision sqDist,std::vector<KDNode*> &nodes){
    if(__sqDist<KD_TYPE>(pos,_pos) < sqDist){
        nodes.push_back(this);
    }
    float d;
    bool right = false,left = false;
//BUG ??
    if(_goRight(pos)){
        if(!isRightLeaf()){
            right = true;
        }
        if(!isLeftLeaf()){
            d = _pos[_dimmension] - pos[_dimmension];
            if(d*d<=sqDist)
                left = true;
        }
    }else{
        if(!left && !isLeftLeaf()){
            left = true;
        }
        if(!right && !isRightLeaf()){
            d = _pos[_dimmension] - pos[_dimmension];
            if(d*d<=sqDist)
                right = true;
        }
    }
    if(right){
        _right->findCloseTo(pos,sqDist,nodes);
    }
    if(left){
        _left->findCloseTo(pos,sqDist,nodes);
    }
}



#define __FF__ std::cout << "Failed at " << __FILE__ << "@" << __LINE__ << std::endl
KD_TEMPLATE void KD_NODE::swap(KD_NODE* n0,KD_NODE* n1){
    if(n0 == n1  || n0 == 0 || n1 == 0 )
        return;

    KD_NODE* r0 = n0->_right;
    KD_NODE* l0 = n0->_left;
    KD_NODE* p0 = n0->_parent;
    KD_NODE* r1 = n1->_right;
    KD_NODE* l1 = n1->_left;
    KD_NODE* p1 = n1->_parent;

    if(p0 == n1){ //if n0 is a child to n1
        swap(n1,n0); //swap in "other" direction
        return;
    }
    if(p1 == n0){ //n1 is a child to n0
        bool left = l0 == n1;
        n0->_parent = n1;
        n0->_left = l1;
        n0->_right = r1;

        n1->_parent = p0;
        n1->_left  = left ? n0 : l0;
        n1->_right = left ? r0 : n0;

        if(r1 != 0) r1->_parent = n0;
        if(l1 != 0) l1->_parent = n0;

        if(p0 != 0){
            if(p0->_left  == n0) p0->_left = n1; //
            if(p0->_right == n0) p0->_right = n1; //
        }
        
        if(left && r0 != 0){ //since l0 == n1 we've already set the parent above
            r0->_parent = n1;
        }
        else if(!left && l0 != 0){
            l0->_parent = n1;
        }
        
    }else{
        if(r0 != 0) r0->_parent = n1;
        if(l0 != 0) l0->_parent = n1;
        if(r1 != 0) r1->_parent = n0;
        if(l1 != 0) l1->_parent = n0;
        n0->_left = l1;
        n0->_right = r1;
        n0->_parent = p1;
        n1->_left = l0;
        n1->_right = r0;
        n1->_parent = p0;

        if(p0 != 0 && p0->_left == n0){
            p0->_left = n1;
        }else if(p0 != 0 && p0->_right == n0){
            p0->_right = n1;
        }else if(p0 != 0){
            __FF__;    
        }
            
        if(p1->_left == n1){
            p1->_left = n0;
        }else if(p1->_right == n1){
            p1->_right = n0;
        }else{
            __FF__;    
        }
    }

    std::swap(n0->_dimmension,n1->_dimmension);
}


KD_TEMPLATE std::string KD_NODE::toString()const{
    std::stringstream ss;
    ss << typeid(*this).name() << " " << (void*)this << " ";
    //ss << _tree << " ";
    //ss << _data << " [";
    for(size_t i = 0;i<dimmensions;i++){
        ss << " "<< _pos[i];
    }
    ss << " ] " << _dimmension;

    ss << (void*)_parent << " ";
    ss << (void*)_left << " ";
    ss << (void*)_right << " ";
    return ss.str();
}

KD_TEMPLATE void KD_NODE::findNNearest(const floatPrecision pos[dimmensions],
                                       size_t amount,
                                       std::vector<__KD_NEAR_NODE__<KD_TYPE> > &current) {
    float sqDist = __sqDist<KD_TYPE>(pos,_pos);
    if(current.size() < amount){
        __KD_NEAR_NODE__<KD_TYPE> holder;
        holder.node = this;
        holder.sqDist = sqDist;
        current.push_back(holder);
        std::sort(current.begin(),current.end());
    }
    else if(sqDist<=current[0].sqDist){
        __KD_NEAR_NODE__<KD_TYPE> holder;
        holder.node = this;
        holder.sqDist = sqDist;
        current.push_back(holder);
        std::sort(current.begin(),current.end());
        while(current.size()>amount){
            current.erase(current.begin());
        }
    }
    
    float d;
    bool right = false,left = false;
    if(current.size()<amount){
        left = !isLeftLeaf();
        right = !isRightLeaf();
    }
    else if(_goRight(pos)){
        if(!isRightLeaf()){
            right = true;
        }
        if(!isLeftLeaf()){
            d = _pos[_dimmension] - pos[_dimmension];
            if(d*d<=current[0].sqDist)
                left = true;
        }
    }else{
        if(!left && !isLeftLeaf()){
            left = true;
        }
        if(!right && !isRightLeaf()){
            d = _pos[_dimmension] - pos[_dimmension];
            if(d*d<=current[0].sqDist)
                right = true;
        }
    }
    if(right){
        _right->findNNearest(pos,amount,current);
    }
    if(left){
        _left->findNNearest(pos,amount,current);
    }

}
    
}
#endif


#endif
