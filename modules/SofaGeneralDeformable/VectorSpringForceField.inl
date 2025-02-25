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
#ifndef SOFA_COMPONENT_INTERACTIONFORCEFIELD_VECTORSPRINGFORCEFIELD_INL
#define SOFA_COMPONENT_INTERACTIONFORCEFIELD_VECTORSPRINGFORCEFIELD_INL

#include <SofaGeneralDeformable/VectorSpringForceField.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/helper/io/XspLoader.h>
#include <sofa/core/objectmodel/KeypressedEvent.h>
#include <SofaBaseTopology/TopologyData.inl>
#include <sofa/helper/system/config.h>
#include <sofa/helper/system/gl.h>
#include <sofa/simulation/Simulation.h>
#include <cassert>
#include <iostream>



namespace sofa
{

namespace component
{

namespace interactionforcefield
{

template<class DataTypes>
void VectorSpringForceField<DataTypes>::EdgeDataHandler::applyCreateFunction(unsigned int, Spring &t,
        const core::topology::BaseMeshTopology::Edge & e,
        const sofa::helper::vector<unsigned int> & ancestors,
        const sofa::helper::vector<double> & coefs)
{
    if (ff)
    {

        //EdgeSetGeometryAlgorithms<DataTypes> *ga=topology->getEdgeSetGeometryAlgorithms();
        //t.restLength=ga->computeRestEdgeLength(index);
        const typename DataTypes::VecCoord& x0 = ff->getObject1()->read(core::ConstVecCoordId::restPosition())->getValue();
        t.restVector = x0[e[1]] - x0[e[0]];
        if (ancestors.size()>0)
        {
            t.kd=t.ks=0;
            //            const topology::EdgeData<Spring> &sa=ff->getSpringArray();
            const helper::vector<Spring> &sa=ff->getSpringArray().getValue();
            unsigned int i;
            for (i=0; i<ancestors.size(); ++i)
            {
                t.kd+=typename DataTypes::Real(sa[i].kd*coefs[i]);
                t.ks+=typename DataTypes::Real(sa[i].ks*coefs[i]);
            }
        }
        else
        {
            t.kd=ff->getStiffness();
            t.ks=ff->getViscosity();
        }

    }

}

template <class DataTypes>
class VectorSpringForceField<DataTypes>::Loader : public sofa::helper::io::XspLoaderDataHook
{
public:
    typedef typename DataTypes::Real Real;
    typedef typename DataTypes::Coord Coord;
    VectorSpringForceField<DataTypes>* dest;
    Loader(VectorSpringForceField<DataTypes>* dest) : dest(dest) {}
    virtual void addVectorSpring(size_t m1, size_t m2, SReal ks, SReal kd, SReal initpos, SReal restx, SReal resty, SReal restz)
    {
        SOFA_UNUSED(initpos);
        dest->addSpring(m1,m2,ks,kd,Coord(Real(restx),Real(resty),Real(restz)));
    }
};

template <class DataTypes>
bool VectorSpringForceField<DataTypes>::load(const char *filename)
{
    if (filename && filename[0])
    {
        Loader loader(this);
        return helper::io::XspLoader::Load(filename, loader);
    }
    else return false;
}

template <class DataTypes>
void VectorSpringForceField<DataTypes>::resizeArray(unsigned int n)
{
    helper::vector<Spring>& springArrayData = *(springArray.beginEdit());
    springArrayData.resize(n);
    springArray.endEdit();
}

template <class DataTypes>
void VectorSpringForceField<DataTypes>::addSpring(int m1, int m2, SReal ks, SReal kd, Coord restVector)
{
    helper::vector<Spring>& springArrayData = *(springArray.beginEdit());

    if (useTopology && m_topology)
    {
        topology::EdgeSetTopologyContainer::EdgeID e = m_topology->getEdgeIndex(unsigned(m1),unsigned(m2));
        if (e != sofa::defaulttype::InvalidID)
            springArrayData[e]=Spring(Real(ks),Real(kd),restVector);
    }
    else
    {
        springArrayData.push_back(Spring(Real(ks),Real(kd),restVector));
        edgeArray.push_back(core::topology::BaseMeshTopology::Edge(m1,m2));
    }
}

template <class DataTypes>
VectorSpringForceField<DataTypes>::VectorSpringForceField()
    : VectorSpringForceField(nullptr, nullptr)
{
}

template <class DataTypes>
VectorSpringForceField<DataTypes>::VectorSpringForceField(MechanicalState* _object)
    : VectorSpringForceField(_object, _object)
{
}

template <class DataTypes>
VectorSpringForceField<DataTypes>::VectorSpringForceField(MechanicalState* _object1, MechanicalState* _object2)
    : Inherit(_object1, _object2)
    , m_potentialEnergy( 0.0 ), useTopology( false ), usingMask(false)
    , springArray( initData(&springArray, "springs", "springs data"))
    , m_filename( initData(&m_filename,std::string(""),"filename","File name from which the spring informations are loaded") )
    , m_stiffness( initData(&m_stiffness,1.0,"stiffness","Default edge stiffness used in absence of file information") )
    , m_viscosity( initData(&m_viscosity,1.0,"viscosity","Default edge viscosity used in absence of file information") )
    , m_useTopology( initData(&m_useTopology, false, "useTopology", "Activate/Desactivate topology mode of the component (springs on each edge)"))
    , l_topology(initLink("topology", "link to the topology container"))    
    , m_topology(nullptr)
    , edgeHandler(nullptr)
{
}

template<class DataTypes>
VectorSpringForceField<DataTypes>::~VectorSpringForceField()
{
    if(edgeHandler) delete edgeHandler;
}

template <class DataTypes>
void VectorSpringForceField<DataTypes>::init()
{
    if (m_useTopology.getValue()) 
    {
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
    }

    if(!edgeHandler)
    {
        if(m_topology)
        {
            edgeHandler = new EdgeDataHandler(this,&springArray);
            springArray.createTopologicalEngine(m_topology,edgeHandler);
            springArray.registerTopologicalData();
        }
    }
    this->getContext()->get(edgeGeo);
    this->getContext()->get(edgeMod);

    this->Inherit::init();
}

template <class DataTypes>
void VectorSpringForceField<DataTypes>::bwdInit()
{
    this->Inherit::bwdInit();
    helper::vector<Spring>& springArrayData = *(springArray.beginEdit());

    if (springArrayData.empty())
    {
        if (!m_filename.getValue().empty())
        {
            // load the springs from a file
            load(m_filename.getFullPath().c_str());
            return;
        }

        if (m_topology != nullptr)
        {
            // create springs based on the mesh topology
            useTopology = true;
            createDefaultSprings();
            this->f_listening.setValue(true);
        }
        else
        {
            int n = this->mstate1->getSize();
            sout << "VectorSpringForceField: linking "<<n<<" pairs of points." << sendl;
            springArrayData.resize(n);
            edgeArray.resize(n);
            for (int i=0; i<n; ++i)
            {
                edgeArray[i][0] = i;
                edgeArray[i][1] = i;
                springArrayData[i].ks=Real(m_stiffness.getValue());
                springArrayData[i].kd=Real(m_viscosity.getValue());
                springArrayData[i].restVector = Coord();
            }
        }
        springArray.endEdit();
    }
}

template <class DataTypes>
void VectorSpringForceField<DataTypes>::createDefaultSprings()
{
    sout << "Creating "<< m_topology->getNbEdges() <<" Vector Springs from EdgeSetTopology"<<sendl;

    helper::vector<Spring>& springArrayData = *(springArray.beginEdit());

    springArrayData.resize(m_topology->getNbEdges());
    //EdgeLengthArrayInterface<Real,DataTypes> elai(springArray);
    //edgeGEO->computeEdgeLength(elai);
    const VecCoord& x0 = this->mstate1->read(core::ConstVecCoordId::restPosition())->getValue();
    unsigned int i;
    for (i=0; i<m_topology->getNbEdges(); ++i)
    {
        springArrayData[i].ks=Real(m_stiffness.getValue());
        springArrayData[i].kd=Real(m_viscosity.getValue());
        springArrayData[i].restVector = x0[m_topology->getEdge(i)[1]]-x0[m_topology->getEdge(i)[0]];
    }

    springArray.endEdit();
}
template<class DataTypes>
void VectorSpringForceField<DataTypes>::handleEvent( core::objectmodel::Event* e )
{
    if (useTopology)
    {
        if (sofa::core::objectmodel::KeypressedEvent::checkEventType(e))
        {
            sofa::core::objectmodel::KeypressedEvent* ke = static_cast<sofa::core::objectmodel::KeypressedEvent*>( e );

            /// handle ctrl+d key
            if (ke->getKey()=='D')
            {
                if (m_topology->getNbEdges()>12)
                {

                    sofa::helper::vector<unsigned int> edgeArray;
                    edgeArray.push_back(12);
                    edgeMod->removeEdges(edgeArray);
                }
                //            edgeMod->splitEdges(edgeArray);
            }
        }
    }
}

template<class DataTypes>
//void VectorSpringForceField<DataTypes>::addForce(VecDeriv& f, const VecCoord& p, const VecDeriv& v)
void VectorSpringForceField<DataTypes>::addForce(const core::MechanicalParams* /*mparams*/, DataVecDeriv& data_f1, DataVecDeriv& data_f2, const DataVecCoord& data_x1, const DataVecCoord& data_x2, const DataVecDeriv& data_v1, const DataVecDeriv& data_v2 )
{

    VecDeriv&       f1 = *data_f1.beginEdit();
    const VecCoord& x1 =  data_x1.getValue();
    const VecDeriv& v1 =  data_v1.getValue();
    VecDeriv&       f2 = *data_f2.beginEdit();
    const VecCoord& x2 =  data_x2.getValue();
    const VecDeriv& v2 =  data_v2.getValue();

    //assert(this->mstate);
    m_potentialEnergy = 0;

    Coord u;

    f1.resize(x1.size());
    f2.resize(x2.size());

    helper::vector<Spring>& springArrayData = *(springArray.beginEdit());

    if(useTopology)
    {

        Deriv force;
        for (unsigned int i=0; i<m_topology->getNbEdges(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=m_topology->getEdge(i);
            const Spring &s=springArrayData[i];
            // paul---------------------------------------------------------------
            Deriv current_direction = x2[e[1]]-x1[e[0]];
            Deriv squash_vector = current_direction - s.restVector;
            Deriv relativeVelocity = v2[e[1]]-v1[e[0]];
            force = (squash_vector * s.ks) + (relativeVelocity * s.kd);

            f1[e[0]]+=force;
            f2[e[1]]-=force;
        }

    }
    else
    {

        Deriv force;
        for (size_t i=0; i<edgeArray.size(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=edgeArray[i];
            const Spring &s=springArrayData[i];
            // paul---------------------------------------------------------------
            Deriv current_direction = x2[e[1]]-x1[e[0]];
            Deriv squash_vector = current_direction - s.restVector;
            Deriv relativeVelocity = v2[e[1]]-v1[e[0]];
            force = (squash_vector * s.ks) + (relativeVelocity * s.kd);

            f1[e[0]]+=force;
            f2[e[1]]-=force;
        }
    }
    springArray.endEdit();

    data_f1.endEdit();
    data_f2.endEdit();
}

template<class DataTypes>
void VectorSpringForceField<DataTypes>::addDForce(const core::MechanicalParams* mparams, DataVecDeriv& data_df1, DataVecDeriv& data_df2, const DataVecDeriv& data_dx1, const DataVecDeriv& data_dx2)
{
    VecDeriv&        df1 = *data_df1.beginEdit();
    VecDeriv&        df2 = *data_df2.beginEdit();
    const VecDeriv&  dx1 =  data_dx1.getValue();
    const VecDeriv&  dx2 =  data_dx2.getValue();
    Real kFactor       =  Real(mparams->kFactorIncludingRayleighDamping(this->rayleighStiffness.getValue()));

    Deriv dforce,d;

    df1.resize(dx1.size());
    df2.resize(dx2.size());

    const helper::vector<Spring>& springArrayData = springArray.getValue();

    if(useTopology)
    {

        for (unsigned int i=0; i<m_topology->getNbEdges(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=m_topology->getEdge(i);
            const Spring &s=springArrayData[i];
            d = dx2[e[1]]-dx1[e[0]];
            dforce = d*(s.ks*kFactor);
            df1[e[0]]+=dforce;
            df2[e[1]]-=dforce;
        }

    }
    else
    {

        for (size_t i=0; i<edgeArray.size(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=edgeArray[i];
            const Spring &s=springArrayData[i];
            d = dx2[e[1]]-dx1[e[0]];
            dforce = d*(s.ks*kFactor);
            df1[e[0]]+=dforce;
            df2[e[1]]-=dforce;
        }
    }

    data_df1.endEdit();
    data_df2.endEdit();

}

template<class DataTypes>
void VectorSpringForceField<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    using namespace sofa::defaulttype;

    if (!((this->mstate1 == this->mstate2)?vparams->displayFlags().getShowForceFields():vparams->displayFlags().getShowInteractionForceFields()))
        return;
    //const VecCoord& p = this->mstate->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x1 =this->mstate1->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x2 =this->mstate2->read(core::ConstVecCoordId::position())->getValue();


    std::vector< Vector3 > points;
    if(useTopology)
    {
        for (unsigned int i=0; i<springArray.getValue().size(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=m_topology->getEdge(i);
            //const Spring &s=springArray[i];

            points.push_back(Vector3(x1[e[0]]));
            points.push_back(Vector3(x2[e[1]]));
        }

    }
    else
    {

        for (unsigned int i=0; i<springArray.getValue().size(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=edgeArray[i];
            //const Spring &s=springArray[i];

            points.push_back(Vector3(x1[e[0]]));
            points.push_back(Vector3(x2[e[1]]));
        }
    }
    vparams->drawTool()->drawLines(points, 3, Vec<4,float>(1,0,0,1));
}



template<class DataTypes>
void VectorSpringForceField<DataTypes>::updateForceMask()
{
    if(useTopology)
    {
        for (unsigned int i=0; i<m_topology->getNbEdges(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=m_topology->getEdge(i);
            this->mstate1->forceMask.insertEntry(e[0]);
            this->mstate2->forceMask.insertEntry(e[1]);
        }
    }
    else
    {
        for (size_t i=0; i<edgeArray.size(); i++)
        {
            const core::topology::BaseMeshTopology::Edge &e=edgeArray[i];
            this->mstate1->forceMask.insertEntry(e[0]);
            this->mstate2->forceMask.insertEntry(e[1]);
        }
    }
}


} // namespace interactionforcefield

} // namespace component

} // namespace sofa

#endif  /* SOFA_COMPONENT_INTERACTIONFORCEFIELD_VECTORSPRINGFORCEFIELD_INL */
