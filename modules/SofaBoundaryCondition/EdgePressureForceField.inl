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
#ifndef SOFA_COMPONENT_FORCEFIELD_EDGEPRESSUREFORCEFIELD_INL
#define SOFA_COMPONENT_FORCEFIELD_EDGEPRESSUREFORCEFIELD_INL

#include <SofaBoundaryCondition/EdgePressureForceField.h>
#include <SofaBaseTopology/TopologySparseData.inl>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/defaulttype/RGBAColor.h>
#include <vector>
#include <set>


// #define DEBUG_TRIANGLEFEM

namespace sofa
{

namespace component
{

namespace forcefield
{

template <class DataTypes>
EdgePressureForceField<DataTypes>::EdgePressureForceField()
    : edgePressureMap(initData(&edgePressureMap, "edgePressureMap", "map between edge indices and their pressure"))
    ,pressure(initData(&pressure, "pressure", "Pressure force per unit area"))
    , edgeIndices(initData(&edgeIndices,"edgeIndices", "Indices of edges separated with commas where a pressure is applied"))
    , edges(initData(&edges, "edges", "List of edges where a pressure is applied"))
    , normal(initData(&normal,"normal", "Normal direction for the plane selection of edges"))
    , dmin(initData(&dmin,(Real)0.0, "dmin", "Minimum distance from the origin along the normal direction"))
    , dmax(initData(&dmax,(Real)0.0, "dmax", "Maximum distance from the origin along the normal direction"))
    , arrowSizeCoef(initData(&arrowSizeCoef,(SReal)0.0, "arrowSizeCoef", "Size of the drawn arrows (0->no arrows, sign->direction of drawing"))
    , p_intensity(initData(&p_intensity,"p_intensity", "pressure intensity on edge normal"))
    , p_binormal(initData(&p_binormal,"binormal", "Binormal of the 2D plane"))
    , p_showForces(initData(&p_showForces, (bool)false, "showForces", "draw arrows of edge pressures"))
    , l_topology(initLink("topology", "link to the topology container"))
    , m_topology(nullptr)
{
    _completeTopology = nullptr;
}

template <class DataTypes> EdgePressureForceField<DataTypes>::~EdgePressureForceField()
{
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::init()
{
    this->core::behavior::ForceField<DataTypes>::init();

    if (l_topology.empty())
    {
        msg_info() << "link to Topology container should be set to ensure right behavior. First Topology found in current context will be used.";
        l_topology.set(this->getContext()->getMeshTopology());
    }

    m_topology = l_topology.get();
    msg_info() << "Topology path used: '" << l_topology.getLinkedPath() << "'";

    if (m_topology == nullptr)
    {
        msg_error() << "No topology component found at path: " << l_topology.getLinkedPath() << ", nor in current context: " << this->getContext()->name;
        sofa::core::objectmodel::BaseObject::d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Invalid);
        return;
    }

    this->getContext()->get(edgeGeo);
    assert(edgeGeo!=0);

    if (edgeGeo==nullptr)
    {
        msg_error() << " object must have an EdgeSetTopology.";
        sofa::core::objectmodel::BaseObject::d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Invalid);
        return;
    }


    _completeTopology = nullptr;
    this->getContext()->get(_completeTopology, core::objectmodel::BaseContext::SearchUp);

    if(_completeTopology == nullptr && edgeIndices.getValue().empty() && edges.getValue().empty())
    {
        msg_error() << "Either a pressure vector or a TriangleSetTopology is required.";
    }

    // init edgesubsetData engine
    edgePressureMap.createTopologicalEngine(m_topology);
    edgePressureMap.registerTopologicalData();

    if (dmin.getValue()!=dmax.getValue())
    {
        selectEdgesAlongPlane();
    }
    if (edgeIndices.getValue().size()>0)
    {
        selectEdgesFromString();
    }
    if (edges.getValue().size()>0)
    {
        selectEdgesFromEdgeList();
    }

    initEdgeInformation();
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::addForce(const sofa::core::MechanicalParams* /*mparams*/, DataVecDeriv &  dataF, const DataVecCoord &  /*dataX */, const DataVecDeriv & /*dataV*/ )
{
    VecDeriv& f = *(dataF.beginEdit());
    Deriv force;

    //edgePressureMap.activateSubsetData();
    const sofa::helper::vector <unsigned int>& my_map = edgePressureMap.getMap2Elements();
    const sofa::helper::vector<EdgePressureInformation>& my_subset = edgePressureMap.getValue();
    for (unsigned int i=0; i<my_map.size(); ++i)
    {
        force=my_subset[i].force/2;
        f[m_topology->getEdge(my_map[i])[0]]+=force;
        f[m_topology->getEdge(my_map[i])[1]]+=force;
    }

    dataF.endEdit();
    updateEdgeInformation();
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::addDForce(const core::MechanicalParams* mparams, DataVecDeriv& /* d_df */, const DataVecDeriv& /* d_dx */)
{
    //TODO: remove this line (avoid warning message) ...
    mparams->setKFactorUsed(true);
}

template <class DataTypes>
SReal EdgePressureForceField<DataTypes>::getPotentialEnergy(const core::MechanicalParams* /*mparams*/, const DataVecCoord&  /* x */) const
{
    msg_error() << "Get potentialEnergy not implemented" ;
    return 0.0;
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::setDminAndDmax(const SReal _dmin, const SReal _dmax)
{
    dmin.setValue((Real)_dmin); dmax.setValue((Real)_dmax);
}

template<class DataTypes>
bool EdgePressureForceField<DataTypes>::isPointInPlane(Coord p)
{
    Real d=dot(p,normal.getValue());
    if ((d>dmin.getValue())&& (d<dmax.getValue()))
        return true;
    else
        return false;
}

template<class DataTypes>
void EdgePressureForceField<DataTypes>::initEdgeInformation()
{
    if (!this->mstate.get())
        msg_error() << " No mechanical Object linked.";

    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();

    if (x.empty())
        return;

    const helper::vector<Real>& intensities = p_intensity.getValue();

    const sofa::helper::vector <unsigned int>& my_map = edgePressureMap.getMap2Elements();

    sofa::helper::vector<EdgePressureInformation>& my_subset = *(edgePressureMap).beginEdit();

    if(pressure.getValue().norm() > 0 )
    {
        for (unsigned int i=0; i<my_map.size(); ++i)
        {
            my_subset[i].length=edgeGeo->computeRestEdgeLength(my_map[i]);
            my_subset[i].force=pressure.getValue()*my_subset[i].length;
        }
    }
    else if (m_topology && intensities.size() > 0)
    {
        // binormal provided
        if(p_binormal.isSet())
        {
            Coord binormal = p_binormal.getValue();
            binormal.normalize();
            for(unsigned int i = 0; i < my_map.size() ; i++)
            {
                core::topology::BaseMeshTopology::Edge e = m_topology->getEdge(my_map[i]);  // FF,13/03/2012: This seems more consistent

                Coord tang = x[e[1]] - x[e[0]]; tang.normalize();
                Coord normal = binormal.cross(tang);
                normal.normalize();

                EdgePressureInformation ei;
                Real intensity = (intensities.size() > 1 && intensities.size() > (unsigned int) i) ? intensities[i] : intensities[0];
                ei.length = edgeGeo->computeRestEdgeLength(i);
                ei.force = normal * intensity * ei.length ;
                edgePressureMap[i] = ei;
            }
        }
        else
            // if no pressure is provided, assume that boundary edges received pressure along their normal
        {
            for(unsigned i = 0; i < my_map.size() ; i++)
            {
                core::topology::BaseMeshTopology::Edge e = m_topology->getEdge(my_map[i]), f;

                Vec3d tang, n1, n2;
                n2 = Vec3d(0,0,1);
                tang = x[e[1]] - x[e[0]]; tang.normalize();

                Vec3d sum;
                bool found = false;
                size_t k = 0;
                while ((!found) && (k < _completeTopology->getNbEdges()))
                {
                    f = _completeTopology->getEdge(k);

                    Vec3d l1 = x[f[0]] - x[e[0]];
                    Vec3d l2 = x[f[1]] - x[e[1]];

                    if((l1.norm() < 1e-6) && (l2.norm() < 1e-6))
                    {
                        found = true;
                    }
                    else
                        k++;

                }

                core::topology::BaseMeshTopology::TrianglesAroundEdge t_a_E = _completeTopology->getTrianglesAroundEdge(k);

                if(t_a_E.size() == 1) // 2D cases
                {
                    core::topology::BaseMeshTopology::Triangle t = _completeTopology->getTriangle(t_a_E[0]);
                    Vec3d vert;


                    if((t[0] == e[0]) || (t[0] == e[1]))
                    {
                        if((t[1] == e[0]) || (t[1] == e[1]))
                            vert = x[t[2]];
                        else
                            vert = x[t[1]];
                    }
                    else
                        vert = x[t[0]];

                    Vec3d tt = vert - x[e[0]];
                    n1 = n2.cross(tang);
                    if(n1*tt < 0)
                    {
                        n1 = -n1;
                    }

                    EdgePressureInformation ei;
                    Real intensity = (intensities.size() > 1 && intensities.size() > (unsigned int) i) ? intensities[i] : intensities[0];
                    ei.length = edgeGeo->computeRestEdgeLength(i);
                    ei.force = n1 * ei.length * intensity;
                    edgePressureMap[i] = ei;
                }
            }
        }
    }

    edgePressureMap.endEdit();
    return;
}

template<class DataTypes>
void EdgePressureForceField<DataTypes>::updateEdgeInformation()
{
    if (!this->mstate.get())
        msg_error() << " No mechanical Object linked.";

    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();

    if (x.empty())
    {
        return;
    }

    const sofa::helper::vector <unsigned int>& my_map = edgePressureMap.getMap2Elements();
    sofa::helper::vector<EdgePressureInformation>& my_subset = *(edgePressureMap).beginEdit();
    for (unsigned int i=0; i<my_map.size(); ++i)
    {
        sofa::defaulttype::Vec3d p1 = x[m_topology->getEdge(my_map[i])[0]];
        sofa::defaulttype::Vec3d p2 = x[m_topology->getEdge(my_map[i])[1]];
        sofa::defaulttype::Vec3d orig(0,0,0);

        sofa::defaulttype::Vec3d tang = p2 - p1;
        tang.norm(); /// @todo: shouldn't this be normalize() ?

        Deriv myPressure;

        if( (p1[0] - orig[0]) * tang[1] > 0)
            myPressure[0] = (Real)tang[1];
        else
            myPressure[0] = - (Real)tang[1];

        if( (p1[1] - orig[1]) * tang[0] > 0)
            myPressure[1] = (Real)tang[0];
        else
            myPressure[1] = - (Real)tang[0];

        //my_subset[i].force=pressure.getValue()*(my_subset[i].length);
        my_subset[i].force=myPressure*(my_subset[i].length);

    }
    edgePressureMap.endEdit();
    initEdgeInformation();
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::selectEdgesAlongPlane()
{
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::restPosition())->getValue();
    std::vector<bool> vArray;
    unsigned int i;

    vArray.resize(x.size());

    for( i=0; i<x.size(); ++i)
    {
        vArray[i]=isPointInPlane(x[i]);
    }

    sofa::helper::vector<EdgePressureInformation>& my_subset = *(edgePressureMap).beginEdit();
    helper::vector<unsigned int> inputEdges;


    for (size_t n=0; n<m_topology->getNbEdges(); ++n)
    {
        if ((vArray[m_topology->getEdge(n)[0]]) && (vArray[m_topology->getEdge(n)[1]]))
        {
            // insert a dummy element : computation of pressure done later
            EdgePressureInformation t;
            my_subset.push_back(t);
            inputEdges.push_back(n);
        }
    }
    edgePressureMap.endEdit();
    edgePressureMap.setMap2Elements(inputEdges);

    return;
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::selectEdgesFromIndices(const helper::vector<unsigned int>& inputIndices)
{
    edgePressureMap.setMap2Elements(inputIndices);

    sofa::helper::vector<EdgePressureInformation>& my_subset = *(edgePressureMap).beginEdit();

    unsigned int sizeTest = m_topology->getNbEdges();

    for (unsigned int i = 0; i < inputIndices.size(); ++i)
    {
        EdgePressureInformation t;
        my_subset.push_back(t);

        if (inputIndices[i] >= sizeTest)
            msg_error() << " Edge indice: " << inputIndices[i] << " is out of edge indices bounds. This could lead to non desired behavior." ;
    }
    edgePressureMap.endEdit();

    return;
}

template <class DataTypes>
void EdgePressureForceField<DataTypes>::selectEdgesFromString()
{
    const helper::vector<unsigned int>& inputString = edgeIndices.getValue();
    selectEdgesFromIndices(inputString);
}

template<class DataTypes>
void EdgePressureForceField<DataTypes>::selectEdgesFromEdgeList()
{
    const helper::vector<core::topology::BaseMeshTopology::Edge>& inputEdges = edges.getValue();
    const helper::vector<core::topology::BaseMeshTopology::Edge>& topologyEdges = m_topology->getEdges();

    helper::vector<unsigned int> indices(inputEdges.size());

    for(unsigned int i=0; i<inputEdges.size(); i++)
    {
        core::topology::BaseMeshTopology::Edge inputEdge = inputEdges[i];
        for(unsigned int j=0; j<topologyEdges.size(); j++)
        {
            core::topology::BaseMeshTopology::Edge topologyEdge = topologyEdges[j];
            //If they are the same edge
            if(inputEdge[0] == topologyEdge[0] && inputEdge[1] == topologyEdge[1])
            {
                indices[i] = j;
            }
        }
    }

    selectEdgesFromIndices(indices);
}

template<class DataTypes>
void EdgePressureForceField<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    if (!p_showForces.getValue())
        return;

    vparams->drawTool()->saveLastState();

    SReal aSC = arrowSizeCoef.getValue();

    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();
    vparams->drawTool()->disableLighting();

    const sofa::defaulttype::RGBAColor& color = sofa::defaulttype::RGBAColor::yellow();

    std::vector<sofa::defaulttype::Vector3> vertices;

    const sofa::helper::vector <unsigned int>& my_map = edgePressureMap.getMap2Elements();
    const sofa::helper::vector<EdgePressureInformation>& my_subset = edgePressureMap.getValue();

    for (unsigned int i=0; i<my_map.size(); ++i)
    {
        sofa::defaulttype::Vector3 p = (x[m_topology->getEdge(my_map[i])[0]] + x[m_topology->getEdge(my_map[i])[1]]) / 2.0;
        vertices.push_back(p);

        sofa::defaulttype::Vec3d f = my_subset[i].force;
        //f.normalize();
        f *= aSC;
        vertices.push_back(p + f);
    }

    vparams->drawTool()->drawLines(vertices, 1, color);

    vparams->drawTool()->restoreLastState();
}

} // namespace forcefield

} // namespace component

} // namespace sofa

#endif // SOFA_COMPONENT_FORCEFIELD_EDGEPRESSUREFORCEFIELD_INL
