 
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
#ifndef SOFA_COMPONENT_ENGINE_DifferenceEngine_H
#define SOFA_COMPONENT_ENGINE_DifferenceEngine_H

#include "config.h"
#include <sofa/core/DataEngine.h>
#include <sofa/defaulttype/Vec.h>

namespace sofa
{

namespace component
{

namespace engine
{

/// Computing the difference between two vector of dofs
/// output = input - substractor
template <class TDataType>
class DifferenceEngine : public core::DataEngine
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(DifferenceEngine,TDataType),core::DataEngine);

    typedef TDataType DataType;
    typedef typename DataType::value_type Real;
    typedef helper::vector<DataType> VecData;
    typedef helper::vector<Real> VecReal;

    DifferenceEngine();

    ~DifferenceEngine() override {}

    void init() override;

    void reinit() override;

    void doUpdate() override;

    virtual std::string getTemplateName() const override
    {
        return templateName(this);
    }

    static std::string templateName(const DifferenceEngine<TDataType>* = nullptr)
    {
        return defaulttype::DataTypeInfo<TDataType>::name();
    }


protected:

    Data<VecData> d_input; ///< input vector
    Data<VecData> d_substractor; ///< vector to substract to input
    Data<VecData> d_output; ///< output vector = input-substractor

};

#if  !defined(SOFA_COMPONENT_ENGINE_DifferenceEngine_CPP)
extern template class SOFA_GENERAL_ENGINE_API DifferenceEngine<defaulttype::Vec1d>;
extern template class SOFA_GENERAL_ENGINE_API DifferenceEngine<defaulttype::Vec3d>;

#endif

} // namespace engine

} // namespace component

} // namespace sofa

#endif
