/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <fancymeshrenderer/calcnormals.h>

namespace inviwo {
    Mesh* CalcNormals::processMesh(const Mesh* const input, Mode mode)
    {
        //short cut: pass through
        Mesh* m = input->clone();
        if (mode == Mode::PassThrough)
        {
            return m;
        }

        //get input buffers
        const BufferBase* bv = nullptr;
        BufferBase* bn = nullptr;
        for (int i=0; i<m->getNumberOfBuffers(); ++i)
        {
            if (m->getBuffers().at(i).first.type == BufferType::PositionAttrib)
            {
                bv = m->getBuffers().at(i).second.get();
            }
            else if (m->getBuffers().at(i).first.type == BufferType::NormalAttrib)
            {
                bn = m->getBuffers().at(i).second.get();
            }
        }
        if (bv == nullptr || bn == nullptr)
        {
            LogWarn("no position or normal buffer found in the mesh");
            return m;
        }
        auto vertices = bv->getRepresentation<BufferRAM>();
        auto normals = bn->getEditableRepresentation<BufferRAM>();
        
        //reset normals
        for (int i=0; i<normals->getSize(); ++i)
        {
            normals->setFromDVec3(i, dvec3(0));
        }

        //loop over index buffers
        for (int i=0; i<m->getNumberOfIndicies(); ++i)
        {
            if (m->getIndexMeshInfo(i).dt != DrawType::Triangles) continue; //only triangles are supported
            auto ct = m->getIndexMeshInfo(i).ct;
            if (ct != ConnectivityType::None)
            {
                LogWarn("Only triangle lists are currently supported, not fans or strips");
                continue;
            }
            const IndexBuffer* ib = m->getIndices(i);
            auto indices = ib->getRAMRepresentation();
            for (int j=0; j<ib->getSize(); j+=3)
            {
                auto a = indices->get(j);
                auto b = indices->get(j+1);
                auto c = indices->get(j+2);
                dvec3 va = vertices->getAsDVec3(a);
                dvec3 vb = vertices->getAsDVec3(b);
                dvec3 vc = vertices->getAsDVec3(c);
                //compute triangle normal
                dvec3 n = cross(vb-va ,vc-va);
                double l = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
                if (l < std::numeric_limits<float>::epsilon())
                {
                    //degenerated triangle
                    continue;
                }
                //weighting factor
                double weightA;
                double weightB;
                double weightC;
                switch (mode)
                {
                case Mode::WeightArea: 
                    //area = norm of cross product
                    weightA = 1;
                    weightB = 1;
                    weightC = 1;
                    break;
                case Mode::WeightAngle: {
                    //based on the angle between the edges
                    dvec3 ea = vb - vc; ea /= sqrt(ea.x*ea.x + ea.y*ea.y + ea.z*ea.z);
                    dvec3 eb = vc - va; eb /= sqrt(eb.x*eb.x + eb.y*eb.y + eb.z*eb.z);
                    dvec3 ec = vb - va; ec /= sqrt(ec.x*ec.x + ec.y*ec.y + ec.z*ec.z);
                    weightA = acos(dot(eb, ec)) / l;
                    weightB = acos(dot(ea, ec)) / l;
                    weightC = acos(dot(ea, eb)) / l;
                    break; 
                }
                case Mode::WeightNMax: {
                    dvec3 ea = vb - vc; double la = sqrt(ea.x*ea.x + ea.y*ea.y + ea.z*ea.z); ea /= la;
                    dvec3 eb = vc - va; double lb = sqrt(eb.x*eb.x + eb.y*eb.y + eb.z*eb.z); eb /= lb;
                    dvec3 ec = vb - va; double lc = sqrt(ec.x*ec.x + ec.y*ec.y + ec.z*ec.z); ec /= lc;
                    weightA = sin(acos(dot(eb, ec))) / (l * lb*lc);
                    weightB = sin(acos(dot(ea, ec))) / (l * la*lc);
                    weightC = sin(acos(dot(ea, eb))) / (l * la*lb);
                    break;
                }
                default:
                    weightA = 1 / l; //no weighting
                    weightB = weightA;
                    weightC = weightA;
                }
                //add it to the vertices
                normals->setFromDVec3(a, normals->getAsDVec3(a) + (n*weightA));
                normals->setFromDVec3(b, normals->getAsDVec3(b) + (n*weightB));
                normals->setFromDVec3(c, normals->getAsDVec3(c) + (n*weightC));
            }
        }

        //normalize normals
        for (int i = 0; i<normals->getSize(); ++i)
        {
            dvec3 n = normals->getAsDVec3(i);
            double l = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
            if (l > std::numeric_limits<float>::epsilon())
                normals->setFromDVec3(i, n / l);
        }

        //done
        return m;
    }
} // namespace

