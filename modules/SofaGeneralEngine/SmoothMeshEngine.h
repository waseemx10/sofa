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
#ifndef SOFA_COMPONENT_ENGINE_SMOOTHMESHENGINE_H
#define SOFA_COMPONENT_ENGINE_SMOOTHMESHENGINE_H
#include "config.h"

#include <sofa/core/DataEngine.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/core/topology/BaseMeshTopology.h>


namespace sofa
{

namespace component
{

namespace engine
{

/**
 * This class computes the laplacian smooth of a mesh
 */
template <class DataTypes>
class SmoothMeshEngine : public core::DataEngine
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(SmoothMeshEngine,DataTypes),core::DataEngine);
    typedef typename DataTypes::Real Real;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::VecCoord VecCoord;

protected:

    SmoothMeshEngine();

    ~SmoothMeshEngine() override {}
public:
    void init() override;
    void reinit() override;
    void doUpdate() override;
	void computeBBox(const core::ExecParams* params, bool onlyVisible) override;
    void draw(const core::visual::VisualParams* vparams) override;

    Data<VecCoord> input_position; ///< Input position
    Data<helper::vector <unsigned int> > input_indices; ///< Position indices that need to be smoothed, leave empty for all positions
    Data<VecCoord> output_position; ///< Output position

    Data<unsigned int> nb_iterations; ///< Number of iterations of laplacian smoothing

    Data<bool> showInput; ///< showInput
    Data<bool> showOutput; ///< showOutput

    virtual std::string getTemplateName() const override
    {
        return templateName(this);
    }

    static std::string templateName(const SmoothMeshEngine<DataTypes>* = nullptr)
    {
        return DataTypes::Name();
    }

    /// Link to be set to the topology container in the component graph.
    SingleLink<SmoothMeshEngine<DataTypes>, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;

protected:
    /// Pointer to the current topology
    sofa::core::topology::BaseMeshTopology* m_topology;
};

#if  !defined(SOFA_COMPONENT_ENGINE_SMOOTHMESHENGINE_CPP)
extern template class SOFA_GENERAL_ENGINE_API SmoothMeshEngine<defaulttype::Vec3Types>;
 
#endif

} // namespace engine

} // namespace component

} // namespace sofa

#endif
