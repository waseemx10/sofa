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
#ifndef SOFA_COMPONENT_MAPPING_BARYCENTRICMAPPERTETRAHEDRONSETTOPOLOGY_INL
#define SOFA_COMPONENT_MAPPING_BARYCENTRICMAPPERTETRAHEDRONSETTOPOLOGY_INL

#include "BarycentricMapperTetrahedronSetTopology.h"

namespace sofa
{

namespace component
{

namespace mapping
{

template <class In, class Out>
BarycentricMapperTetrahedronSetTopology<In,Out>::BarycentricMapperTetrahedronSetTopology(topology::TetrahedronSetTopologyContainer* fromTopology, topology::PointSetTopologyContainer* toTopology)
    : Inherit1(fromTopology, toTopology),
      m_fromContainer(fromTopology),
      m_fromGeomAlgo(nullptr)
{}


template <class In, class Out>
int BarycentricMapperTetrahedronSetTopology<In,Out>::addPointInTetra ( const int tetraIndex, const SReal* baryCoords )
{
    helper::vector<MappingData>& vectorData = *(d_map.beginEdit());
    vectorData.resize ( d_map.getValue().size() +1 );
    MappingData& data = *vectorData.rbegin();
    d_map.endEdit();
    data.in_index = tetraIndex;
    data.baryCoords[0] = ( Real ) baryCoords[0];
    data.baryCoords[1] = ( Real ) baryCoords[1];
    data.baryCoords[2] = ( Real ) baryCoords[2];
    return (int)d_map.getValue().size()-1;
}

template <class In, class Out>
helper::vector<Tetrahedron> BarycentricMapperTetrahedronSetTopology<In,Out>::getElements()
{
    return this->m_fromTopology->getTetrahedra();
}

template <class In, class Out>
helper::vector<SReal> BarycentricMapperTetrahedronSetTopology<In,Out>::getBaryCoef(const Real* f)
{
    return getBaryCoef(f[0],f[1],f[2]);
}

template <class In, class Out>
helper::vector<SReal> BarycentricMapperTetrahedronSetTopology<In,Out>::getBaryCoef(const Real fx, const Real fy, const Real fz)
{
    helper::vector<SReal> tetrahedronCoef{(1-fx-fy-fz),fx,fy,fz};
    return tetrahedronCoef;
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In,Out>::computeBase(Mat3x3d& base, const typename In::VecCoord& in, const Tetrahedron& element)
{
    Mat3x3d matrixTranspose;
    base[0] = in[element[1]]-in[element[0]];
    base[1] = in[element[2]]-in[element[0]];
    base[2] = in[element[3]]-in[element[0]];
    matrixTranspose.transpose(base);
    base.invert(matrixTranspose);
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In,Out>::computeCenter(Vector3& center, const typename In::VecCoord& in, const Tetrahedron& element)
{
    center = ( in[element[0]]+in[element[1]]+in[element[2]]+in[element[3]] ) *0.25;
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In,Out>::computeDistance(double& d, const Vector3& v)
{
    d = std::max ( std::max ( -v[0],-v[1] ), std::max ( -v[2],v[0]+v[1]+v[2]-1 ) );
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In,Out>::addPointInElement(const int elementIndex, const SReal* baryCoords)
{
    addPointInTetra(elementIndex,baryCoords);
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In, Out>::processTopologicalChanges(const typename Out::VecCoord& out, const typename In::VecCoord& in, core::topology::Topology* t)
{
    using sofa::core::behavior::MechanicalState;

    if (t != m_toTopology) return;

    if (m_toTopology->beginChange() == m_toTopology->endChange())
        return;

    auto itBegin = m_toTopology->beginChange();
    auto itEnd = m_toTopology->endChange();

    helper::WriteAccessor < Data< helper::vector<MappingData > > > vectorData = d_map;
    vectorData.resize(out.size());

    for (auto changeIt = itBegin; changeIt != itEnd; ++changeIt) {
        const core::topology::TopologyChangeType changeType = (*changeIt)->getChangeType();
        switch (changeType)
        {
            //TODO: implementation of BarycentricMapperHexahedronSetTopology<In,Out>::handleTopologyChange()
        case core::topology::POINTSINDICESSWAP:  ///< For PointsIndicesSwap.
        {
            const core::topology::PointsIndicesSwap * ps = static_cast<const core::topology::PointsIndicesSwap*>(*changeIt);
            MappingData copy = vectorData[ps->index[0]];
            vectorData[ps->index[0]] = vectorData[ps->index[1]];
            vectorData[ps->index[1]] = copy;
            break;
        }
        case core::topology::POINTSADDED:        ///< For PointsAdded.
        {
            const core::topology::PointsAdded * pa = static_cast<const core::topology::PointsAdded*>(*changeIt);
            auto& array = pa->getElementArray();

            for (unsigned i = 0; i<array.size(); i++) {
                unsigned pid = array[i];
                processAddPoint(Out::getCPos(out[pid]),
                    in,
                    vectorData[pid]);
            }

            break;
        }
        case core::topology::POINTSREMOVED:      ///< For PointsRemoved.
        {
            // nothing to do
            break;
        }
        case core::topology::POINTSRENUMBERING:  ///< For PointsRenumbering.
        {
            const core::topology::PointsRenumbering * pr = static_cast<const core::topology::PointsRenumbering*>(*changeIt);
            auto& array = pr->getIndexArray();
            auto& inv_array = pr->getinv_IndexArray();
            for (unsigned i = 0; i<array.size(); i++) {
                MappingData copy = vectorData[array[i]];
                vectorData[inv_array[i]] = vectorData[array[i]];
                vectorData[array[i]] = copy;
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

template <class In, class Out>
void BarycentricMapperTetrahedronSetTopology<In, Out>::processAddPoint(const sofa::defaulttype::Vec3d & pos, const typename In::VecCoord& in, MappingData & vectorData)
{
    const sofa::helper::vector<core::topology::BaseMeshTopology::Tetrahedron>& tetrahedra = this->m_fromTopology->getTetrahedra();

    sofa::defaulttype::Vector3 coefs;
    int index = -1;
    double distance = std::numeric_limits<double>::max();
    for (unsigned int t = 0; t < tetrahedra.size(); t++)
    {
        sofa::defaulttype::Mat3x3d base;
        sofa::defaulttype::Vector3 center;
        computeBase(base, in, tetrahedra[t]);
        computeCenter(center, in, tetrahedra[t]);

        sofa::defaulttype::Vec3d v = base * (pos - in[tetrahedra[t][0]]);
        double d = std::max(std::max(-v[0], -v[1]), std::max(-v[2], v[0] + v[1] + v[2] - 1));

        if (d>0) d = (pos - center).norm2();

        if (d<distance)
        {
            coefs = v;
            distance = d;
            index = t;
        }
    }

    vectorData.in_index = index;
    vectorData.baryCoords[0] = (Real)coefs[0];
    vectorData.baryCoords[1] = (Real)coefs[1];
    vectorData.baryCoords[2] = (Real)coefs[2];
}

} // namespace mapping

} // namespace component

} // namespace sofa

#endif
