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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/base/datastructures/kdtree.h>

namespace inviwo{

TEST(KDTreeTests, init) {
    KDTree<3,char,float> tree;
}


TEST(KDTreeTests, randomPointsTest) {
    srand(0); // seed to always be the same random numbers

    K3DTree<int, float> tree;
    int size = 1000;
    for(int i = 0;i<size;i++){
        glm::vec3 p (rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX)); 
        tree.insert(p,i);
    }

    EXPECT_EQ(tree.size() , size);
    EXPECT_FALSE(tree.empty());
}


TEST(KDTreeTests, minMaxTest) {

    srand(0); // seed to always be the same random numbers

    K3DTree<int, float> tree;
    int size = 1000;
    for(int i = 0;i<size;i++){
        glm::vec3 p (rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX)); 
        tree.insert(p,i);
    }

    EXPECT_EQ(tree.size() , size);
    EXPECT_FALSE(tree.empty());

    tree.insert(glm::vec3(1,1,1),1);
    tree.insert(glm::vec3(0,0,0),1);

    EXPECT_EQ(tree.size() , size+2);

    glm::vec3 min;
    glm::vec3 max;
    min.x = tree.findMin(0)->getPosition()[0];
    min.y = tree.findMin(1)->getPosition()[1];
    min.z = tree.findMin(2)->getPosition()[2];

    max.x = tree.findMax(0)->getPosition()[0];
    max.y = tree.findMax(1)->getPosition()[1];
    max.z = tree.findMax(2)->getPosition()[2];

    EXPECT_EQ(min.x , 0);
    EXPECT_EQ(min.y , 0);
    EXPECT_EQ(min.z , 0);

    EXPECT_EQ(max.x , 1);
    EXPECT_EQ(max.y , 1);
    EXPECT_EQ(max.z , 1);


    tree.insert(glm::vec3(2,3,4),1);
    tree.insert(glm::vec3(-1,-2,-3),1);



    min.x = tree.findMin(0)->getPosition()[0];
    min.y = tree.findMin(1)->getPosition()[1];
    min.z = tree.findMin(2)->getPosition()[2];

    max.x = tree.findMax(0)->getPosition()[0];
    max.y = tree.findMax(1)->getPosition()[1];
    max.z = tree.findMax(2)->getPosition()[2];

    EXPECT_EQ(max.x , 2);
    EXPECT_EQ(max.y , 3);
    EXPECT_EQ(max.z , 4);

    EXPECT_EQ(min.x , -1);
    EXPECT_EQ(min.y , -2);
    EXPECT_EQ(min.z , -3);
}



TEST(KDTreeTests, findNClosests) {

    srand(0); // seed to always be the same random numbers

    K3DTree<int, float> tree;
    int size = 1000;
    for(int i = 0;i<size;i++){
        glm::vec3 p (rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX)); 
        tree.insert(p,i);
    }

    EXPECT_EQ(tree.size() , size);
    EXPECT_FALSE(tree.empty());

    glm::vec3 p(0.5f,0.5f,0.5f);

    std::vector< K3DTree<int, float>::Node*> n10 = tree.findNNearest(glm::value_ptr(p),10);
    EXPECT_EQ(n10.size() , 10);
    std::vector<K3DTree<int, float>::Node*> n20 = tree.findNNearest(glm::value_ptr(p),20);
    EXPECT_EQ(n20.size() , 20);
    std::vector<K3DTree<int, float>::Node*> n100 = tree.findNNearest(glm::value_ptr(p),100);
    EXPECT_EQ(n100.size() , 100);
}



}