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
#ifndef SOFA_COMPONENT_ENGINE_TRANSFORMPOSITION_H
#define SOFA_COMPONENT_ENGINE_TRANSFORMPOSITION_H
#include "config.h"



#include <sofa/defaulttype/Vec.h>
#include <sofa/core/DataEngine.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/Mat.h>
#include <sofa/core/objectmodel/DataFileName.h>
#include <sofa/helper/OptionsGroup.h>

namespace sofa
{

namespace component
{

namespace engine
{

/**
 * This class transforms the positions of one DataFields into new positions after applying a transformation
This transformation can be either : projection on a plane (plane defined by an origin and a normal vector),
translation, rotation, scale and some combinations of translation, rotation and scale, affine or read from a
transformation file
 */
template <class DataTypes>
class TransformPosition : public core::DataEngine
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(TransformPosition,DataTypes),core::DataEngine);
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::Real Real;
    typedef sofa::helper::vector<unsigned int> SetIndex;
    typedef sofa::defaulttype::Vec<16,Real> Vec16;
    typedef sofa::defaulttype::Vec<4,Real> Vec4;
    typedef sofa::defaulttype::Mat<4,4,Real> Mat4x4;
    typedef sofa::defaulttype::Mat<3,3,Real> Mat3x3;

    typedef enum
    {
        PROJECT_ON_PLANE,
        TRANSLATION,
        ROTATION,
        RANDOM,
        SCALE,
        SCALE_TRANSLATION,
        SCALE_ROTATION_TRANSLATION,
        AFFINE
    } TransformationMethod;

protected:

    TransformPosition();

    ~TransformPosition() override {}

    void getTransfoFromTxt();//read a transformation in a txt or xfm file
    void getTransfoFromTrm();//read a transformation in a trm file
    void getTransfoFromTfm();//read a transformation in a tfm file
    void selectTransformationMethod();

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
        //      if (dynamic_cast<MechanicalState<DataTypes>*>(context->getMechanicalState()) == nullptr)
        //        return false;
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

    static std::string templateName(const TransformPosition<DataTypes>* = nullptr)
    {
        return DataTypes::Name();
    }

protected:

    TransformationMethod transformationMethod;
    Data<Coord> f_origin; ///< origin used by projectOnPlane
    Data<VecCoord> f_inputX; ///< input position
    Data<VecCoord> f_outputX; ///< ouput position
    Data<Coord> f_normal; ///< normal used by projectOnPlane
    Data<Coord> f_translation; ///< translation
    Data<Coord> f_rotation; ///< rotation
    Data<Coord> f_scale; ///< scale
    Data<Mat4x4> f_affineMatrix; ///< affine transformation
    Data<sofa::helper::OptionsGroup> f_method; ///< the method of the transformation
    Data<long> f_seed; ///< the seed for the random generator
    Data<Real> f_maxRandomDisplacement; ///< the maximum displacement for the random generator
    Data<SetIndex> f_fixedIndices; ///< the indices of the elements that are not transformed
    core::objectmodel::DataFileName f_filename; ///< filename of an affine matrix. Supported extensions are: .trm, .tfm, .xfm and .txt(read as .xfm)
    Data<bool> f_drawInput; ///< Draw input points
    Data<bool> f_drawOutput; ///< Draw output points
    Data<Real> f_pointSize; ///< Point size

};

#if  !defined(SOFA_COMPONENT_ENGINE_TRANSFORMPOSITION_CPP)
extern template class SOFA_GENERAL_ENGINE_API TransformPosition<defaulttype::Vec3Types>;
 
#endif

} // namespace engine

} // namespace component

} // namespace sofa

#endif
