/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "vgMesh.h"


namespace Vitelotte
{


template < typename _Scalar, int _Dim, int _Chan >
const typename VGMesh<_Scalar, _Dim, _Chan>::NodeValue
    VGMesh<_Scalar, _Dim, _Chan>::UnconstrainedNode =
        VGMesh<_Scalar, _Dim, _Chan>::NodeValue::Constant(
            std::numeric_limits<Scalar>::quiet_NaN());


template < typename _Scalar, int _Dim, int _Chan >
VGMesh<_Scalar, _Dim, _Chan>::VGMesh(unsigned attributes)
    : m_attributes(0)
{
    m_positions = addVertexProperty<Vector>("v:position", Vector::Zero());

    setAttributes(attributes);
}


template < typename _Scalar, int _Dim, int _Chan >
VGMesh<_Scalar, _Dim, _Chan>&
VGMesh<_Scalar, _Dim, _Chan>::operator=(const Self& rhs)
{
    if(&rhs != this)
    {
        // FIXME: SurfaceMesh's operator= wont work with properties of different types.
        PatateCommon::SurfaceMesh::operator=(rhs);

        m_nodes = rhs.m_nodes;

        m_attributes = rhs.m_attributes;

        m_positions = vertexProperty<Vector>("v:position");
        m_vertexGradientConstraint = rhs.m_vertexGradientConstraint;

        m_toVertexValueNodes = getHalfedgeProperty<Node>("h:toVertexValueNode");
        m_fromVertexValueNodes = getHalfedgeProperty<Node>("h:fromVertexValueNode");
        m_edgeValueNodes = getHalfedgeProperty<Node>("h:edgeValueNode");
        m_edgeGradientNodes = getHalfedgeProperty<Node>("h:edgeGradientNode");

        m_vertexGradientDummyNodes = rhs.m_vertexGradientDummyNodes;
//        m_edgeConstraintFlag = getEdgeProperty<bool>("e:constraintFlag");
    }
    return *this;
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::reserve(
        unsigned nvertices, unsigned nedges, unsigned nfaces, unsigned nnodes)
{
    PatateCommon::SurfaceMesh::reserve(nvertices, nedges, nfaces);
    m_nodes.reserve(nnodes);
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::clear()
{
    PatateCommon::SurfaceMesh::clear();
    m_nodes.clear();
}


template < typename _Scalar, int _Dim, int _Chan >
PatateCommon::SurfaceMesh::Vertex
VGMesh<_Scalar, _Dim, _Chan>::addVertex(const Vector& pos)
{
    Vertex v = PatateCommon::SurfaceMesh::addVertex();
    position(v) = pos;
    return v;
}


template < typename _Scalar, int _Dim, int _Chan >
bool
VGMesh<_Scalar, _Dim, _Chan>::isValid(Node n) const
{
    return 0 <= n.idx() && n.idx() < m_nodes.size();
}


template < typename _Scalar, int _Dim, int _Chan >
void VGMesh<_Scalar, _Dim, _Chan>::setAttributes(unsigned attributes)
{
#define PATATE_FEM_MESH_SET_ATTRIBUTE(_bit, _field, _name) do {\
        if(!(m_attributes & _bit) && (attributes & _bit))\
            _field = addHalfedgeProperty<Node>(_name, Node());\
        else if((m_attributes & _bit) && !(attributes & _bit))\
            removeHalfedgeProperty(_field);\
    } while(0)

//    if(!(m_attributes & VertexGradientConstraint) &&
//            (attributes & VertexGradientConstraint))
//        m_vertexGradientConstrained = addVertexProperty<bool>(
//                    "v:vertexGradientConstrained", false);
//    else if((m_attributes & VertexGradientConstraint) &&
//            !(attributes & VertexGradientConstraint))
//        removeVertexProperty(m_vertexGradientConstrained);

    PATATE_FEM_MESH_SET_ATTRIBUTE(
                TO_VERTEX_VALUE_FLAG,   m_toVertexValueNodes,   "h:toVertexValueNode");
    PATATE_FEM_MESH_SET_ATTRIBUTE(
                FROM_VERTEX_VALUE_FLAG, m_fromVertexValueNodes, "h:fromVertexValueNode");
    PATATE_FEM_MESH_SET_ATTRIBUTE(
                EDGE_VALUE_FLAG,        m_edgeValueNodes,       "h:edgeValueNode");
    PATATE_FEM_MESH_SET_ATTRIBUTE(
                EDGE_GRADIENT_FLAG,     m_edgeGradientNodes,    "h:edgeGradientNode");


    m_attributes = attributes;

#undef PATATE_FEM_MESH_SET_ATTRIBUTE
}


//template < typename _Scalar, int _Dim, int _Chan >
//void
//VGMesh<_Scalar, _Dim, _Chan>::setEdgeConstraintFlag(bool on)
//{
//    if(on && !hasEdgeConstraintFlag())
//        m_edgeConstraintFlag = edgeProperty("e:constraintFlag", false);
//    else if(!on && hasEdgeConstraintFlag())
//        removeEdgeProperty(m_edgeConstraintFlag);
//}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::HalfedgeAttribute
VGMesh<_Scalar, _Dim, _Chan>::
    oppositeAttribute(HalfedgeAttribute attr) const
{
    switch(attr)
    {
    case TO_VERTEX_VALUE:
        return FROM_VERTEX_VALUE;
    case FROM_VERTEX_VALUE:
        return TO_VERTEX_VALUE;
    case EDGE_VALUE:
    case EDGE_GRADIENT:
        return attr;
    }
    abort();
}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::Node
VGMesh<_Scalar, _Dim, _Chan>::
    halfedgeNode(Halfedge h, HalfedgeAttribute attr) const
{
    return const_cast<Self*>(this)->halfedgeNode(h, attr);
}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::Node&
VGMesh<_Scalar, _Dim, _Chan>::
    halfedgeNode(Halfedge h, HalfedgeAttribute attr)
{
    switch(attr)
    {
    case TO_VERTEX_VALUE:
        assert(hasToVertexValue());
        return toVertexValueNode(h);
    case FROM_VERTEX_VALUE:
        assert(hasToVertexValue() || hasFromVertexValue());
        if(hasFromVertexValue())
        {
            return fromVertexValueNode(h);
        }
        else
        {
            return toVertexValueNode(prevHalfedge(h));
        }
    case EDGE_VALUE:
        assert(hasEdgeValue());
        return edgeValueNode(h);
    case EDGE_GRADIENT:
        assert(hasEdgeGradient());
        return edgeGradientNode(h);
    }
    abort();
}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::Node
VGMesh<_Scalar, _Dim, _Chan>::
    halfedgeOppositeNode(Halfedge h, HalfedgeAttribute attr) const
{
    return const_cast<Self*>(this)->halfedgeOppositeNode(h, attr);
}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::Node&
VGMesh<_Scalar, _Dim, _Chan>::
    halfedgeOppositeNode(Halfedge h, HalfedgeAttribute attr)
{
    return halfedgeNode(oppositeHalfedge(h), oppositeAttribute(attr));
}


template < typename _Scalar, int _Dim, int _Chan >
void VGMesh<_Scalar, _Dim, _Chan>::
    setGradientConstraint(Vertex v, const Gradient& grad)
{
    assert(hasVertexGradientConstraint());

    bool newConstraint = !isGradientConstraint(v);

    m_vertexGradientConstraint[v] = grad;

    if(newConstraint)
    {
        HalfedgeAroundVertexCirculator hit = halfedges(v);
        HalfedgeAroundVertexCirculator hend = hit;
        do
        {
            m_vertexGradientDummyNodes[*hit] = addNode();
            m_vertexGradientDummyNodes[oppositeHalfedge(*hit)] = addNode();
            ++hit;
        } while(hit != hend);
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void VGMesh<_Scalar, _Dim, _Chan>::removeGradientConstraint(Vertex v)
{
    assert(hasVertexGradientConstraint());

    m_vertexGradientConstraint.erase(v);

    HalfedgeAroundVertexCirculator hit = halfedges(v);
    HalfedgeAroundVertexCirculator hend = hit;
    do
    {
        m_vertexGradientDummyNodes.erase(*hit);
        m_vertexGradientDummyNodes.erase(oppositeHalfedge(*hit));
        ++hit;
    } while(hit != hend);
}


template < typename _Scalar, int _Dim, int _Chan >
unsigned VGMesh<_Scalar, _Dim, _Chan>::nVertexGradientConstraints(Halfedge h) const
{
    assert(hasVertexGradientConstraint());

    return isGradientConstraint(fromVertex(h)) + isGradientConstraint(toVertex(h));
}


template < typename _Scalar, int _Dim, int _Chan >
unsigned VGMesh<_Scalar, _Dim, _Chan>::nVertexGradientConstraints(Face f) const
{
    assert(hasVertexGradientConstraint());

    VertexAroundFaceCirculator vit = vertices(f);
    VertexAroundFaceCirculator vend = vit;
    unsigned count = 0;

    do
    {
        if(isGradientConstraint(*vit)) ++count;
        ++vit;
    } while(vit != vend);

    return count;
}


template < typename _Scalar, int _Dim, int _Chan >
typename VGMesh<_Scalar, _Dim, _Chan>::Node
VGMesh<_Scalar, _Dim, _Chan>::addNode(const NodeValue& nodeValue)
{
    m_nodes.push_back(nodeValue);
    return Node(m_nodes.size() - 1);
}

template < typename _Scalar, int _Dim, int _Chan >
bool
VGMesh<_Scalar, _Dim, _Chan>::hasUnknowns() const
{
    for(int i = 0; i < nNodes(); ++i)
        if(!isConstraint(Node(i)))
            return true;
    return false;
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::
    setVertexNode(Node node, Halfedge from, Halfedge to)
{
    assert(fromVertex(from) == fromVertex(to));
    assert(hasToVertexValue());

    Halfedge h = from;
    do
    {
        if(hasFromVertexValue() && !isBoundary(h))
            fromVertexValueNode(h) = node;

        h = prevHalfedge(h);

        if(!isBoundary(h))
            toVertexValueNode(h) = node;

        h = oppositeHalfedge(h);
    }
    while(h != to && h != from);
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::
    setSingularity(Node fromNode, Node toNode, Halfedge from, Halfedge to)
{
    assert(fromVertex(from) == fromVertex(to));
    assert(hasToVertexValue() && hasFromVertexValue());
    assert(isConstraint(fromNode) && isConstraint(toNode));

    NodeValue fromValue = nodeValue(fromNode);
    NodeValue   toValue = nodeValue(  toNode);

    const Vector& v = position(fromVertex(from));
    Vector fromVec = position(toVertex(from)) - v;
    Vector   toVec = position(toVertex(  to)) - v;

    Scalar fromAngle = std::atan2(fromVec.y(), fromVec.x());
    Scalar toAngle   = std::atan2(  toVec.y(),   toVec.x());
    Scalar totalAngle = toAngle - fromAngle;
    if(totalAngle < 1.e-8) totalAngle += 2.*M_PI;

    Node n = fromNode;
    Halfedge h = from;
    Halfedge last = oppositeHalfedge(to);
    do
    {
        if(!isBoundary(h))
            fromVertexValueNode(h) = n;

        h = prevHalfedge(h);

        if(h != last)
        {
            // Current Halfedge is reversed.
            Vector vec = position(fromVertex(h)) - v;
            Scalar angle = std::atan2(vec.y(), vec.x()) - fromAngle;
            if(angle < 1.e-8) angle += 2.*M_PI;
            Scalar a = angle / totalAngle;
            n = addNode((1.-a) * fromValue + a * toValue);
        }
        else
            n = toNode;

        if(!isBoundary(h))
            toVertexValueNode(h) = n;

        h = oppositeHalfedge(h);
    }
    while(h != to && h != from);
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::simplifyConstraints()
{
    assert(hasToVertexValue());

    std::vector<Halfedge> consEdges;
    consEdges.reserve(12);

    for(VertexIterator vit = verticesBegin();
        vit != verticesEnd(); ++vit)
    {
        consEdges.clear();
        findConstrainedEdgesSimplify(*vit, consEdges);

        Halfedge prev = consEdges.back();
        std::vector<Halfedge>::iterator cit = consEdges.begin();
        for(; cit != consEdges.end(); ++cit)
        {
            simplifyVertexArcConstraints(prev, *cit);
            prev = *cit;
        }
    }

    if(hasEdgeValue() || hasEdgeGradient())
    {
        for(EdgeIterator eit = edgesBegin();
            eit != edgesEnd(); ++eit)
        {
            simplifyEdgeConstraints(*eit);
        }
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::simplifyVertexArcConstraints(
        Halfedge from, Halfedge to)
{
    assert(hasToVertexValue());
    assert(fromVertex(from) == fromVertex(to));

    Node& n0 = halfedgeNode(from, FROM_VERTEX_VALUE);
    Node& n1 = halfedgeOppositeNode(to, FROM_VERTEX_VALUE);

    bool n0c = n0.isValid() && isConstraint(n0);
    bool n1c = n1.isValid() && isConstraint(n1);

    if((!n0c && !n1c) ||
       (n0c && !n1c) ||
       (n0c && n1c && nodeValue(n0) == nodeValue(n1)))
    {
        Node& n1o = halfedgeNode(to, FROM_VERTEX_VALUE);
        if(n1o == n1)
        {
            n1o = n0;
        }
        n1 = n0;
    }
    else if(!n0c && n1c)
    {
        Node toReplace = n0;
        Halfedge h = from;
        while(true) {
            Node& n = halfedgeNode(h, FROM_VERTEX_VALUE);
            if(n == toReplace)
                n = n1;
            else
                break;

            Node& no = halfedgeOppositeNode(h, FROM_VERTEX_VALUE);
            if(no == toReplace)
                n = n1;
            else
                break;

            h = nextHalfedge(h);
        }
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::simplifyEdgeConstraints(Edge e)
{
    if(hasEdgeValue())
    {
        Node& n0 = edgeValueNode(halfedge(e, 0));
        Node& n1 = edgeValueNode(halfedge(e, 1));
        simplifyOppositeNodes(n0, n1);
    }
    if(hasEdgeGradient())
    {
        Node& n0 = edgeGradientNode(halfedge(e, 0));
        Node& n1 = edgeGradientNode(halfedge(e, 1));
        simplifyOppositeNodes(n0, n1);
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::simplifyOppositeNodes(Node& n0, Node& n1) const
{
    bool n0c = n0.isValid() && isConstraint(n0);
    bool n1c = n1.isValid() && isConstraint(n1);

    // if not a discontinuity, merge nodes.
    if(n0c && n1c && nodeValue(n0) == nodeValue(n1))
    {
        n0 = Node(std::min(n1.idx(), n0.idx()));
        n1 = n0;
    }
    else if(!n0c && !n1c && n0 == n1)
    {
        // It is useless to use an unknown node here
        // FIXME: Assume that these unknown node are only used
        // around this vertex.
        n0 = Node();
        n1 = Node();
    }
    else if(n0c && !n1.isValid())
    {
        n1 = n0;
    }
    else if(n1c && !n0.isValid())
    {
        n0 = n1;
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::finalize()
{
    assert(hasToVertexValue());

    std::vector<Halfedge> consEdges;
    consEdges.reserve(12);

    for(VertexIterator vit = verticesBegin();
        vit != verticesEnd(); ++vit)
    {
        consEdges.clear();
        findConstrainedEdgesSimplify(*vit, consEdges);

        if(consEdges.empty())
            consEdges.push_back(halfedge(*vit));

        Halfedge prev = consEdges.back();
        std::vector<Halfedge>::iterator cit = consEdges.begin();
        for(; cit != consEdges.end(); ++cit)
        {
            if(!isBoundary(prev))
                finalizeVertexArc(prev, *cit);

            prev = *cit;
        }
    }

    if(hasEdgeValue() || hasEdgeGradient())
    {
        for(EdgeIterator eit = edgesBegin();
            eit != edgesEnd(); ++eit)
        {
            finalizeEdge(*eit);
        }
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::finalizeVertexArc(Halfedge from, Halfedge to)
{
    Node n0 = halfedgeNode(from, FROM_VERTEX_VALUE);
    Node n1 = halfedgeOppositeNode(to, FROM_VERTEX_VALUE);

    bool n0c = n0.isValid() && isConstraint(n0);
    bool n1c = n1.isValid() && isConstraint(n1);

    Node n = Node();
    if(!n0c && !n1c)
    {
        // Free nodes, choose one valid or create a new one.
        if     (n0.isValid()) n = n0;
        else if(n1.isValid()) n = n1;
        else                  n = addNode();
    }
    else if(n0c != n1c)
        n = n0c? n0: n1;  // One constraint, choose it
    else if(n0 == n1 || nodeValue(n0) == nodeValue(n1))
        n = n0;  // Same constraints, choose one arbitrarily

    // The remaining option is a singularity, that require special
    // processing.
    if(isValid(n))
        setVertexNode(n, from, to);
    else
        setSingularity(n0, n1, from, to);
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::finalizeEdge(Edge e)
{
    Halfedge h0 = halfedge(e, 0);
    Halfedge h1 = halfedge(e, 1);
    if(hasEdgeValue())
    {
        Node& n0 = edgeValueNode(h0);
        Node& n1 = edgeValueNode(h1);
        bool n0v = n0.isValid();
        bool n1v = n1.isValid();

        // if both are invalid, create a single node. Else, create
        // nodes independently, thus producing a discontinuity.
        if(!n0v && !isBoundary(h0)) n0 = addNode();
        if(!n1v && !isBoundary(h1)) n1 = n0v? addNode(): n0;
    }
    if(hasEdgeGradient())
    {
        Node& n0 = edgeGradientNode(h0);
        Node& n1 = edgeGradientNode(h1);
        bool n0v = n0.isValid();
        bool n1v = n1.isValid();

        // if both are invalid, create a single node. Else, create
        // nodes independently, thus producing a discontinuity.
        if(!n0v && !isBoundary(h0)) n0 = addNode();
        if(!n1v && !isBoundary(h1)) n1 = n0v? addNode(): n0;
    }
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::compactNodes()
{
    std::vector<int> buf(nNodes(), 0);

    // Find used node ids
    HalfedgeIterator hBegin = halfedgesBegin(),
                     hEnd   = halfedgesEnd();
    for(HalfedgeIterator hit = hBegin; hit != hEnd; ++hit)
    {
        if(!isBoundary(*hit))
        {
            if(hasToVertexValue() && toVertexValueNode(*hit).isValid())
                buf[toVertexValueNode(*hit).idx()]     = 1;
            if(hasFromVertexValue() && fromVertexValueNode(*hit).isValid())
                buf[fromVertexValueNode(*hit).idx()] = 1;
            if(hasEdgeValue() && edgeValueNode(*hit).isValid())
                buf[edgeValueNode(*hit).idx()]       = 1;
//            if(hasVertexGradient())
//                marked[vertexGradientNode(*hit)]  = 1;
            if(hasEdgeGradient() && edgeGradientNode(*hit).isValid())
                buf[edgeGradientNode(*hit).idx()]    = 1;
        }
    }

    // Compute remapping
    int size=0;
    for(int i = 0; i < buf.size(); ++i)
    {
        if(buf[i])
        {
            buf[size] = i;
            ++size;
        }
    }

    // Update node vector and fill remapping vector
    std::vector<int> map(nNodes(), -1);
    NodeVector reord(size);
    for(int i = 0; i < size; ++i)
    {
        reord[i] = nodeValue(Node(buf[i]));
        map[buf[i]] = i;
    }
    m_nodes.swap(reord);

    // Remap nodes in mesh
    for(HalfedgeIterator hit = hBegin; hit != hEnd; ++hit)
    {
        if(!isBoundary(*hit))
        {
            if(hasToVertexValue() && toVertexValueNode(*hit).isValid())
                toVertexValueNode(*hit)     = Node(map[toVertexValueNode(*hit).idx()]);
            if(hasFromVertexValue() && fromVertexValueNode(*hit).isValid())
                fromVertexValueNode(*hit) = Node(map[fromVertexValueNode(*hit).idx()]);
            if(hasEdgeValue() && edgeValueNode(*hit).isValid())
                (edgeValueNode(*hit))     = Node(map[edgeValueNode(*hit).idx()]);
//            if(hasVertexGradient())
//                vertexGradientNode(*hit)  = map[vertexGradientNode(*hit)];
            if(hasEdgeGradient() && edgeGradientNode(*hit).isValid())
                edgeGradientNode(*hit)    = Node(map[edgeGradientNode(*hit).idx()]);
        }
    }
}


template < typename _Scalar, int _Dim, int _Chan >
bool
VGMesh<_Scalar, _Dim, _Chan>::isSingular(Halfedge h) const
{
    return hasFromVertexValue() &&
            toVertexValueNode(h) != fromVertexValueNode(nextHalfedge(h));
}


template < typename _Scalar, int _Dim, int _Chan >
unsigned
VGMesh<_Scalar, _Dim, _Chan>::nSingulars(Face f) const
{
    unsigned nSingulars = 0;
    HalfedgeAroundFaceCirculator
            hit  = halfedges(f),
            hend = hit;
    do
    {
        nSingulars += isSingular(*hit);
    }
    while(++hit != hend);

    return nSingulars;
}


template < typename _Scalar, int _Dim, int _Chan >
unsigned
VGMesh<_Scalar, _Dim, _Chan>::nSingularFaces() const
{
    unsigned n = 0;
    for(FaceIterator fit = facesBegin();
        fit != facesEnd(); ++fit)
    {
        if(nSingulars(*fit))
            ++n;
    }
    return n;
}


template < typename _Scalar, int _Dim, int _Chan >
void
VGMesh<_Scalar, _Dim, _Chan>::
    findConstrainedEdgesSimplify(Vertex vx,
                                 std::vector<Halfedge>& consEdges)
{
    HalfedgeAroundVertexCirculator hit = halfedges(vx),
                                   hEnd = hit;
    do
    {
        Node& np = halfedgeOppositeNode(*hit, FROM_VERTEX_VALUE);
        Node& n0 = halfedgeNode(*hit, FROM_VERTEX_VALUE);
        bool boundary = isBoundary(edge(*hit));
        if(np.isValid() || n0.isValid() || boundary)
        {
            simplifyOppositeNodes(np, n0);

            if(np.isValid() || n0.isValid() || boundary)
            {
                consEdges.push_back(*hit);
            }
        }
        ++hit;
    }
    while(hit != hEnd);
}


}  // namespace Vitelotte