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
#ifndef SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_ProjectToPlaneConstraint_INL
#define SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_ProjectToPlaneConstraint_INL

#include <sofa/core/topology/BaseMeshTopology.h>
#include <SofaBoundaryCondition/ProjectToPlaneConstraint.h>
#include <SofaBaseLinearSolver/SparseMatrix.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/Simulation.h>
#include <iostream>

#include <SofaBaseTopology/TopologySubsetData.inl>

namespace sofa
{

namespace component
{

namespace projectiveconstraintset
{

template< class DataTypes>
bool ProjectToPlaneConstraint<DataTypes>::FCPointHandler::applyTestCreateFunction(unsigned int, const sofa::helper::vector<unsigned int> &, const sofa::helper::vector<double> &)
{
    if (fc)
    {
        return false;
    }
    else
    {
        return false;
    }
}

template< class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::FCPointHandler::applyDestroyFunction(unsigned int pointIndex, core::objectmodel::Data<value_type> &)
{
    if (fc)
    {
        fc->removeConstraint((unsigned int) pointIndex);
    }
}

template <class DataTypes>
ProjectToPlaneConstraint<DataTypes>::ProjectToPlaneConstraint()
    : core::behavior::ProjectiveConstraintSet<DataTypes>(nullptr)
    , f_indices( initData(&f_indices,"indices","Indices of the fixed points") )
    , f_origin( initData(&f_origin,CPos(),"origin","A point in the plane"))
    , f_normal( initData(&f_normal,CPos(),"normal","Normal vector to the plane"))
    , f_drawSize( initData(&f_drawSize,(SReal)0.0,"drawSize","0 -> point based rendering, >0 -> radius of spheres") )
    , l_topology(initLink("topology", "link to the topology container"))
    , data(new ProjectToPlaneConstraintInternalData<DataTypes>())    
    , m_pointHandler(nullptr)
{
    f_indices.beginEdit()->push_back(0);
    f_indices.endEdit();
}


template <class DataTypes>
ProjectToPlaneConstraint<DataTypes>::~ProjectToPlaneConstraint()
{
    if (m_pointHandler)
        delete m_pointHandler;

    delete data;
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::clearConstraints()
{
    f_indices.beginEdit()->clear();
    f_indices.endEdit();
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::addConstraint(unsigned int index)
{
    f_indices.beginEdit()->push_back(index);
    f_indices.endEdit();
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::removeConstraint(unsigned int index)
{
    removeValue(*f_indices.beginEdit(),index);
    f_indices.endEdit();
}

// -- Constraint interface


template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::init()
{
    this->core::behavior::ProjectiveConstraintSet<DataTypes>::init();

    if (l_topology.empty())
    {
        msg_info() << "link to Topology container should be set to ensure right behavior. First Topology found in current context will be used.";
        l_topology.set(this->getContext()->getMeshTopology());
    }

    sofa::core::topology::BaseMeshTopology* _topology = l_topology.get();

    if (_topology)
    {
        msg_info() << "Topology path used: '" << l_topology.getLinkedPath() << "'";

        // Initialize functions and parameters
        m_pointHandler = new FCPointHandler(this, &f_indices);
        f_indices.createTopologicalEngine(_topology, m_pointHandler);
        f_indices.registerTopologicalData();
    }
    else
    {
        msg_info() << "No topology component found at path: " << l_topology.getLinkedPath() << ", nor in current context: " << this->getContext()->name;
    }

    const Indices & indices = f_indices.getValue();

    unsigned int maxIndex=this->mstate->getSize();
    for (unsigned int i=0; i<indices.size(); ++i)
    {
        const unsigned int index=indices[i];
        if (index >= maxIndex)
        {
            serr << "Index " << index << " not valid!" << sendl;
            removeConstraint(index);
        }
    }

    reinit();

}

template <class DataTypes>
void  ProjectToPlaneConstraint<DataTypes>::reinit()
{

    // normalize the normal vector
    CPos n = f_normal.getValue();
    if( n.norm()==0 )
        n[1]=0;
    else n *= 1/n.norm();
    f_normal.setValue(n);

    // create the matrix blocks corresponding to the projection to the plane: I-nn^t or to the identity
    Block bProjection;
    for(unsigned i=0; i<bsize; i++)
        for(unsigned j=0; j<bsize; j++)
        {
            if(i==j)
            {
                bProjection[i][j] = 1 - n[i]*n[j];
            }
            else
            {
                bProjection[i][j] =    - n[i]*n[j];
            }
        }

    // get the indices sorted
    Indices tmp = f_indices.getValue();
    std::sort(tmp.begin(),tmp.end());

    // resize the jacobian
    unsigned numBlocks = this->mstate->getSize();
    unsigned blockSize = DataTypes::deriv_total_size;
    jacobian.resize( numBlocks*blockSize,numBlocks*blockSize );

    // fill the jacobian in ascending order
    unsigned i=0;
    Indices::const_iterator it = tmp.begin();
    while( i<numBlocks )
    {
        if(  it!=tmp.end() && i==*it )  // constrained particle: set diagonal to projection block, and  the cursor to the next constraint
        {
            jacobian.insertBackBlock(i,i,bProjection); // only one block to create
            it++;
        }
        else           // unconstrained particle: set diagonal to identity block
        {
            jacobian.insertBackBlock(i,i,Block::s_identity); // only one block to create
        }
        i++;
    }
    jacobian.compress();
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::projectMatrix( sofa::defaulttype::BaseMatrix* M, unsigned offset )
{
    J.copy(jacobian, M->colSize(), offset); // projection matrix for an assembled state
    BaseSparseMatrix* E = dynamic_cast<BaseSparseMatrix*>(M);
    assert(E);
    E->compressedMatrix = J.compressedMatrix * E->compressedMatrix * J.compressedMatrix;
}



template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::projectResponse(const core::MechanicalParams* mparams, DataVecDeriv& resData)
{
    helper::WriteAccessor<DataVecDeriv> res ( mparams, resData );
    jacobian.mult(res.wref(),res.ref());
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::projectJacobianMatrix(const core::MechanicalParams* /*mparams*/ , DataMatrixDeriv& /*cData*/)
{
    serr<<"projectJacobianMatrix(const core::MechanicalParams*, DataMatrixDeriv& ) is not implemented" << sendl;
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::projectVelocity(const core::MechanicalParams* mparams, DataVecDeriv& vdata)
{
    projectResponse(mparams,vdata);
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::projectPosition(const core::MechanicalParams* /*mparams*/ , DataVecCoord& xData)
{
    VecCoord& x = *xData.beginEdit();

    const CPos& n = f_normal.getValue();
    const CPos& o = f_origin.getValue();

    const Indices& indices = f_indices.getValue();
    for(unsigned i=0; i<indices.size(); i++ )
    {
        // replace the point with its projection to the plane
//        x[indices[i]] -= n * ((x[indices[i]]-o)*n);
        const CPos xi = DataTypes::getCPos( x[indices[i]] );
        DataTypes::setCPos( x[indices[i]], xi - n * ((xi-o)*n) );
    }

    xData.endEdit();
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::applyConstraint(defaulttype::BaseMatrix * /*mat*/, unsigned int /*offset*/)
{
    serr << "applyConstraint is not implemented " << sendl;
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::applyConstraint(defaulttype::BaseVector * /*vect*/, unsigned int /*offset*/)
{
    serr<<"ProjectToPlaneConstraint<DataTypes>::applyConstraint(defaulttype::BaseVector *vect, unsigned int offset) is not implemented "<< sendl;
}

template <class DataTypes>
void ProjectToPlaneConstraint<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels()) return;
    if (!this->isActive()) return;
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();

    vparams->drawTool()->saveLastState();

    const Indices & indices = f_indices.getValue();

    if( f_drawSize.getValue() == 0) // old classical drawing by points
    {
        std::vector< sofa::defaulttype::Vector3 > points;
        sofa::defaulttype::Vector3 point;
        for (Indices::const_iterator it = indices.begin();
                it != indices.end();
                ++it)
        {
            point = DataTypes::getCPos(x[*it]);
            points.push_back(point);
        }
        vparams->drawTool()->drawPoints(points, 10, sofa::defaulttype::Vec<4,float>(1,0.5,0.5,1));
    }
    else // new drawing by spheres
    {
        std::vector< sofa::defaulttype::Vector3 > points;
        sofa::defaulttype::Vector3 point;

        for (Indices::const_iterator it = indices.begin();
                it != indices.end();
                ++it)
        {
            point = DataTypes::getCPos(x[*it]);
            points.push_back(point);
        }
        vparams->drawTool()->drawSpheres(points, (float)f_drawSize.getValue(), sofa::defaulttype::Vec<4,float>(1.0f,0.35f,0.35f,1.0f));
    }

    vparams->drawTool()->restoreLastState();
}



} // namespace constraint

} // namespace component

} // namespace sofa

#endif


