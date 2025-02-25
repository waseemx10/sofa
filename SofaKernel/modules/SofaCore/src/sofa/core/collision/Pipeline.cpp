/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <sofa/core/collision/Pipeline.h>
#include <sofa/core/objectmodel/BaseNode.h>

namespace sofa
{

namespace core
{

namespace collision
{
//using namespace core::objectmodel;
//using namespace core::behavior;

Pipeline::Pipeline()
    : intersectionMethod(nullptr),
      broadPhaseDetection(nullptr),
      narrowPhaseDetection(nullptr),
      contactManager(nullptr),
      groupManager(nullptr)
{
}

Pipeline::~Pipeline()
{
}


const BroadPhaseDetection *Pipeline::getBroadPhaseDetection() const
{
    return broadPhaseDetection;
}


const NarrowPhaseDetection *Pipeline::getNarrowPhaseDetection() const
{
    return narrowPhaseDetection;
}

#if 0

void Pipeline::init()
{
    simulation::Node* root = dynamic_cast<simulation::Node*>(getContext());
    if(root == nullptr) return;
    intersectionMethods.clear();
    root->getTreeObjects<Intersection>(&intersectionMethods);
    intersectionMethod = (intersectionMethods.empty() ? nullptr : intersectionMethods[0]);
    broadPhaseDetections.clear();
    root->getTreeObjects<BroadPhaseDetection>(&broadPhaseDetections);
    broadPhaseDetection = (broadPhaseDetections.empty() ? nullptr : broadPhaseDetections[0]);
    narrowPhaseDetections.clear();
    root->getTreeObjects<NarrowPhaseDetection>(&narrowPhaseDetections);
    narrowPhaseDetection = (narrowPhaseDetections.empty() ? nullptr : narrowPhaseDetections[0]);
    contactManagers.clear();
    root->getTreeObjects<ContactManager>(&contactManagers);
    contactManager = (contactManagers.empty() ? nullptr : contactManagers[0]);
    groupManagers.clear();
    root->getTreeObjects<CollisionGroupManager>(&groupManagers);
    groupManager = (groupManagers.empty() ? nullptr : groupManagers[0]);

    if (intersectionMethod==nullptr)
        intersectionMethod = new sofa::component::collision::DiscreteIntersection;
}

void Pipeline::reset()
{
    computeCollisionReset();
}

void Pipeline::computeCollisionReset()
{
    simulation::Node* root = dynamic_cast<simulation::Node*>(getContext());
    if(root == nullptr) return;
    if (broadPhaseDetection!=nullptr && broadPhaseDetection->getIntersectionMethod()!=intersectionMethod)
        broadPhaseDetection->setIntersectionMethod(intersectionMethod);
    if (narrowPhaseDetection!=nullptr && narrowPhaseDetection->getIntersectionMethod()!=intersectionMethod)
        narrowPhaseDetection->setIntersectionMethod(intersectionMethod);
    if (contactManager!=nullptr && contactManager->getIntersectionMethod()!=intersectionMethod)
        contactManager->setIntersectionMethod(intersectionMethod);
    doCollisionReset();
}

void Pipeline::computeCollisionDetection()
{
    simulation::Node* root = dynamic_cast<simulation::Node*>(getContext());
    if(root == nullptr) return;
    sofa::helper::vector<CollisionModel*> collisionModels;
    root->getTreeObjects<CollisionModel>(&collisionModels);
    doCollisionDetection(collisionModels);
}

void Pipeline::computeCollisionResponse()
{
    simulation::Node* root = dynamic_cast<simulation::Node*>(getContext());
    if(root == nullptr) return;
    doCollisionResponse();
}

#endif



bool Pipeline::insertInNode( objectmodel::BaseNode* node )
{
    node->addCollisionPipeline(this);
    Inherit1::insertInNode(node);
    return true;
}

bool Pipeline::removeInNode( objectmodel::BaseNode* node )
{
    node->removeCollisionPipeline(this);
    Inherit1::removeInNode(node);
    return true;
}




} // namespace collision

} // namespace core

} // namespace sofa

