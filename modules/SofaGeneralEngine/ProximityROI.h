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
#ifndef SOFA_COMPONENT_ENGINE_PROXIMITYROI_H
#define SOFA_COMPONENT_ENGINE_PROXIMITYROI_H
#include "config.h"



#include <sofa/defaulttype/Vec.h>
#include <sofa/core/DataEngine.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/core/loader/MeshLoader.h>

namespace sofa
{

namespace component
{

namespace engine
{

/**
 * This class find the point at a given distance from a set of points
 */
template <class DataTypes>
class ProximityROI : public core::DataEngine
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(ProximityROI,DataTypes),core::DataEngine);
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::Real Real;
    typedef defaulttype::Vec<3,Real> Vec3;
    typedef defaulttype::Vec<6,Real> Vec6;
    typedef sofa::core::topology::BaseMeshTopology::SetIndex SetIndex;

    typedef typename DataTypes::CPos CPos;

    typedef unsigned int PointID;
protected:

    ProximityROI();

    ~ProximityROI() override {}
public:
    void init() override;

    void reinit() override;

    void doUpdate() override;

    void draw(const core::visual::VisualParams* vparams) override;

    /// Pre-construction check method called by ObjectFactory.
    /// Check that DataTypes matches the MechanicalState.
    template<class T>
    static bool canCreate(T*& obj, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
    {
        if (!arg->getAttribute("template"))
        {
            // only check if this template is correct if no template was given
            if (context->getMechanicalState() && dynamic_cast<sofa::core::behavior::MechanicalState<DataTypes>*>(context->getMechanicalState()) == nullptr)
                return false; // this template is not the same as the existing MechanicalState
        }

        return BaseObject::canCreate(obj, context, arg);
    }

    /// Construction method called by ObjectFactory.
    template<class T>
    static typename T::SPtr create(T* tObj, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
    {
        return core::objectmodel::BaseObject::create(tObj, context, arg);
    }

    virtual std::string getTemplateName() const override
    {
        return templateName(this);
    }

    static std::string templateName(const ProximityROI<DataTypes>* = nullptr)
    {
        return DataTypes::Name();
    }


protected:

public:
    //Input
    Data< helper::vector<Vec3> > centers; ///< Center(s) of the sphere(s)
    Data< helper::vector<Real> > radii; ///< Radius(i) of the sphere(s)
    Data<unsigned int> f_num; ///< Maximum number of points to select
    Data<VecCoord> f_X0; ///< Rest position coordinates of the degrees of freedom

    //Output
    Data<SetIndex> f_indices; ///< Indices of the points contained in the ROI
    Data<VecCoord > f_pointsInROI; ///< Points contained in the ROI
    Data< helper::vector<Real> > f_distanceInROI; ///< distance between the points contained in the ROI and the closest center.

    Data<SetIndex> f_indicesOut; ///< Indices of the points not contained in the ROI


    //Parameter
    Data<bool> p_drawSphere; ///< Draw shpere(s)
    Data<bool> p_drawPoints; ///< Draw Points
    Data<double> _drawSize; ///< rendering size for box and topological elements
};

#if  !defined(SOFA_COMPONENT_ENGINE_PROXIMITYROI_CPP)
extern template class SOFA_GENERAL_ENGINE_API ProximityROI<defaulttype::Vec3Types>;
 
#endif

} // namespace engine

} // namespace component

} // namespace sofa

#endif //SOFA_COMPONENT_ENGINE_PROXIMITYROI_H
