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
#ifndef SOFA_SIMULATION_TREE_GNODE_H
#define SOFA_SIMULATION_TREE_GNODE_H

#include <SofaSimulationTree/tree.h>
#include <sofa/simulation/Node.h>



namespace sofa
{

namespace simulation
{
namespace tree
{


/** Define the structure of the scene. Contains (as pointer lists) Component objects and children GNode objects.
 */
class SOFA_SIMULATION_TREE_API GNode : public simulation::Node
{
public:
    typedef Node::DisplayFlags DisplayFlags;
    SOFA_CLASS(GNode, simulation::Node);

protected:
    GNode( const std::string& name="", GNode* parent=nullptr  );

    virtual ~GNode() override;

public:
    //Pure Virtual method from Node
    virtual Node::SPtr createChild(const std::string& nodeName) override;

    virtual void moveChild(BaseNode::SPtr node) override;

    //Pure Virtual method from BaseNode
    /// Add a child node
    virtual void doAddChild(BaseNode::SPtr node) override;

    /// Remove a child node
    virtual void doRemoveChild(BaseNode::SPtr node) override;

    /// Move a node from another node
    virtual void doMoveChild(BaseNode::SPtr node, BaseNode::SPtr prev) override;

    /// Remove the current node from the graph: consists in removing the link to its parent
    void detachFromGraph() override;

    /// Get a list of parent node
    Parents getParents() const override;

    /// returns number of parents
    size_t getNbParents() const override;

    /// return the first parent (returns nullptr if no parent)
    BaseNode* getFirstParent() const override;

    /// Test if the given node is a parent of this node.
    bool hasParent(const BaseNode* node) const override
    {
        return parent() == node;
    }

    /// Test if the given context is a parent of this context.
    bool hasParent(const BaseContext* context) const
    {
        if (context == nullptr) return parent() == nullptr;
        else return parent()->getContext() == context;
    }

    /// Test if the given context is an ancestor of this context.
    /// An ancestor is a parent or (recursively) the parent of an ancestor.
    bool hasAncestor(const BaseNode* node) const override
    {
        return hasAncestor(node->getContext());
    }

    /// Test if the given context is an ancestor of this context.
    /// An ancestor is a parent or (recursively) the parent of an ancestor.
    bool hasAncestor(const BaseContext* context) const override;


    /// Generic object access, given a set of required tags, possibly searching up or down from the current context
    ///
    /// Note that the template wrapper method should generally be used to have the correct return type,
    void* getObject(const sofa::core::objectmodel::ClassInfo& class_info, const sofa::core::objectmodel::TagSet& tags, SearchDirection dir = SearchUp) const override;

    /// Generic object access, given a path from the current context
    ///
    /// Note that the template wrapper method should generally be used to have the correct return type,
    void* getObject(const sofa::core::objectmodel::ClassInfo& class_info, const std::string& path) const override;

    /// Generic list of objects access, given a set of required tags, possibly searching up or down from the current context
    ///
    /// Note that the template wrapper method should generally be used to have the correct return type,
    void getObjects(const sofa::core::objectmodel::ClassInfo& class_info, GetObjectsCallBack& container, const sofa::core::objectmodel::TagSet& tags, SearchDirection dir = SearchUp) const override;


    /// Mesh Topology that is relevant for this context
    /// (within it or its parents until a mapping is reached that does not preserve topologies).
    core::topology::BaseMeshTopology* getActiveMeshTopology() const override;


    /// Called during initialization to corectly propagate the visual context to the children
    void initVisualContext() override;

    /// Update the whole context values, based on parent and local ContextObjects
    void updateContext() override;

    /// Update the simulation context values(gravity, time...), based on parent and local ContextObjects
    void updateSimulationContext() override;

    static GNode::SPtr create(GNode*, core::objectmodel::BaseObjectDescription* arg)
    {
        GNode::SPtr obj = GNode::SPtr();
        obj->parse(arg);
        return obj;
    }


    /// return the smallest common parent between this and node2 (returns nullptr if separated sub-graphes)
    Node* findCommonParent( simulation::Node* node2 ) override;

protected:

    inline GNode* parent() const { return l_parent.get(); }

    SingleLink<GNode,GNode,BaseLink::FLAG_DOUBLELINK> l_parent;

    /// Execute a recursive action starting from this node.
    void doExecuteVisitor(simulation::Visitor* action, bool=false) override;
    // VisitorScheduler can use doExecuteVisitor() method
    friend class simulation::VisitorScheduler;


};

} // namespace tree

} // namespace simulation

} // namespace sofa

#endif
