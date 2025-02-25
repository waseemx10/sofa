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
#include <SofaConstraint/StickContactConstraint.inl>
#include <SofaMeshCollision/BarycentricContactMapper.h>
#include <SofaMeshCollision/RigidContactMapper.h>


namespace sofa
{

namespace component
{

namespace collision
{

using namespace defaulttype;
using namespace sofa::helper;
using simulation::Node;
using namespace sofa::core::collision;

Creator<Contact::Factory, StickContactConstraint<PointModel, PointModel> > PointPointStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<LineModel, SphereModel> > LineSphereStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<LineModel, PointModel> > LinePointStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<LineModel, LineModel> > LineLineStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<TriangleModel, SphereModel> > TriangleSphereStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<TriangleModel, PointModel> > TrianglePointStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<TriangleModel, LineModel> > TriangleLineStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<TriangleModel, TriangleModel> > TriangleTriangleStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<SphereModel, SphereModel> > SphereSphereStickContactConstraintClass("StickContactConstraint",true);
Creator<Contact::Factory, StickContactConstraint<SphereModel, PointModel> > SpherePointStickContactConstraintClass("StickContactConstraint",true);

} // namespace collision

} // namespace component

} // namespace sofa
