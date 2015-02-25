#include "vgMeshWithCurves.h"


const VGMeshWithCurves::NodeGradient VGMeshWithCurves::UnconstrainedGradient =
        VGMeshWithCurves::NodeGradient::Constant(
            std::numeric_limits<VGMeshWithCurves::Scalar>::quiet_NaN());


VGMeshWithCurves::VGMeshWithCurves()
{
    m_halfedgeCurveConn = addHalfedgeProperty<HalfedgeCurveConnectivity>("h:curveConnectivity");
}


VGMeshWithCurves::Curve VGMeshWithCurves::addCurve(unsigned flags)
{
    Curve c(nCurves());
    CurveInfo ci;
    ci.flags = flags;
    m_curves.push_back(ci);
    return c;
}


void VGMeshWithCurves::addHalfedgeToCurve(Curve c, Halfedge h, float from, float to)
{
    assert(isValid(c));
    assert(isValid(h));

    CurveInfo& ci = m_curves[c.idx()];
    HalfedgeCurveConnectivity& hcc = m_halfedgeCurveConn[h];
    HalfedgeCurveConnectivity& ohcc = m_halfedgeCurveConn[oppositeHalfedge(h)];

    assert(!hcc.curve.isValid());
    assert(!ohcc.curve.isValid());

    hcc.curve = c;
    hcc.pos = to;
    ohcc.curve = c;
    ohcc.pos = from;
    if(ci.firstHalfedge.isValid())
    {
        assert(ci.lastHalfedge.isValid());
        HalfedgeCurveConnectivity& phcc = m_halfedgeCurveConn[ci.lastHalfedge];

        phcc.next = h;
        ohcc.next = ci.lastHalfedge;
        ci.lastHalfedge = h;
    }
    else
    {
        ci.firstHalfedge = h;
        ci.lastHalfedge = h;
    }
}


void VGMeshWithCurves::setFlags(Curve c, unsigned flags)
{
    assert(isValid(c));

    CurveInfo& ci = m_curves[c.idx()];
    if(flags & VALUE_TEAR && !(ci.flags & VALUE_TEAR))
        ci.gradient[VALUE_RIGHT] = ci.gradient[VALUE_LEFT];
    if(flags & GRADIENT_TEAR && !(ci.flags & GRADIENT_TEAR))
        ci.gradient[GRADIENT_RIGHT] = ci.gradient[GRADIENT_LEFT];
    ci.flags = flags;
}


void VGMeshWithCurves::setFlagsRaw(Curve c, unsigned flags)
{
    assert(isValid(c));

    CurveInfo& ci = m_curves[c.idx()];
    ci.flags = flags;
}


const VGMeshWithCurves::ValueGradient&
VGMeshWithCurves::valueGradient(Curve c, unsigned which) const
{
    return const_cast<VGMeshWithCurves*>(this)->valueGradient(c, which);
}


VGMeshWithCurves::ValueGradient&
VGMeshWithCurves::valueGradient(Curve c, unsigned which)
{
    assert(isValid(c));
    assert(which < 4);

    switch(which)
    {
    case VALUE_RIGHT:
        if(!valueTear(c))
            which = VALUE_LEFT;
        break;
    case GRADIENT_RIGHT:
        if(!gradientTear(c))
            which = GRADIENT_LEFT;
        break;
    }

    return m_curves[c.idx()].gradient[which];
}


const VGMeshWithCurves::ValueGradient&
VGMeshWithCurves::valueGradientRaw(Curve c, unsigned which) const
{
    return const_cast<VGMeshWithCurves*>(this)->valueGradientRaw(c, which);
}


VGMeshWithCurves::ValueGradient&
VGMeshWithCurves::valueGradientRaw(Curve c, unsigned which)
{
    assert(isValid(c));
    assert(which < 4);

    return m_curves[c.idx()].gradient[which];
}


VGMeshWithCurves::PointConstraint VGMeshWithCurves::addPointConstraint()
{
    PointConstraint pc(nPointConstraints());
    PointConstraintInfo pci;
    pci.value = UnconstrainedNode;
    pci.gradient = UnconstrainedGradient;
    m_pointConstraints.push_back(pci);
    return pc;
}


void VGMeshWithCurves::clear()
{
    Base::clear();
    m_pointConstraints.clear();
    m_curves.clear();
}


void VGMeshWithCurves::setNodesFromCurves()
{
    m_nodes.clear();
    for(HalfedgeIterator hit = halfedgesBegin();
        hit != halfedgesEnd(); ++hit)
    {
        if(hasFromVertexValue())    fromVertexValueNode(*hit)   = Node();
        if(hasToVertexValue())      toVertexValueNode(*hit)     = Node();
        if(hasEdgeValue())          edgeValueNode(*hit)         = Node();
        if(hasEdgeGradient())       edgeGradientNode(*hit)      = Node();
    }

    for(unsigned pci = 0; pci < nPointConstraints(); ++pci)
    {
        PointConstraint pc(pci);
        Vertex vx = vertex(pc);
        assert(isValid(vx));

        Node vn;
        if(isValueConstraint(pc))
            vn = addNode(value(pc));

        HalfedgeAroundVertexCirculator hit = halfedges(vx);
        HalfedgeAroundVertexCirculator hend = hit;
        do {
            Halfedge opp = oppositeHalfedge(*hit);
            if(isValueConstraint(pc))
            {
                if(!isBoundary(*hit))
                    halfedgeNode(*hit, FROM_VERTEX_VALUE) = vn;
                if(!isBoundary(opp))
                    halfedgeOppositeNode(*hit, FROM_VERTEX_VALUE) = vn;
            }
            ++hit;
        } while(hit != hend);

        if(isGradientConstraint(pc))
        {
            std::cout << "Set gradient constraint: " << position(vx).transpose() << "\n"
                      << gradient(pc) << "\n";
            setGradientConstraint(vertex(pc), gradient(pc));
        }
    }

    for(unsigned ci = 0; ci < nCurves(); ++ci)
    {
        Curve c(ci);

        Halfedge lh = firstHalfedge(c);
        if(!lh.isValid())
            continue;

        Node fromNode[2];
        addGradientNodes(fromNode, c, VALUE, fromCurvePos(lh));
        do {
            Node toNode[2];
            addGradientNodes(toNode, c, VALUE, toCurvePos(lh));

            Halfedge rh = oppositeHalfedge(lh);
            float midPos = (fromCurvePos(lh) + toCurvePos(lh)) / 2.f;

            bool lhnb = !isBoundary(lh);
            bool rhnb = !isBoundary(rh);

            if(hasFromVertexValue())
            {
                if(lhnb) fromVertexValueNode(lh) = fromNode[LEFT];
                if(rhnb) fromVertexValueNode(rh) =   toNode[RIGHT];
            }
            if(hasToVertexValue())
            {
                if(lhnb) toVertexValueNode(lh) =   toNode[LEFT];
                if(rhnb) toVertexValueNode(rh) = fromNode[RIGHT];
            }

            if(hasEdgeValue())
            {
                Node midNode[2];
                addGradientNodes(midNode, c, VALUE, midPos);
                if(lhnb) edgeValueNode(lh) = midNode[LEFT];
                if(rhnb) edgeValueNode(rh) = midNode[RIGHT];
            }

            if(hasEdgeGradient())
            {
                Node gNode[2];
                addGradientNodes(gNode, c, GRADIENT, midPos);
                if(lhnb) edgeGradientNode(lh) = gNode[LEFT];
                if(rhnb) edgeGradientNode(rh) = gNode[RIGHT];

                if(halfedgeOrientation(lh) && lhnb)
                    nodeValue(gNode[LEFT]) *= -1;
                if(gNode[LEFT] != gNode[RIGHT] && halfedgeOrientation(lh) && rhnb)
                    nodeValue(gNode[RIGHT]) *= -1;

            }

            fromNode[0] = toNode[0];
            fromNode[1] = toNode[1];
            lh = nextCurveHalfedge(lh);
        } while(lh.isValid());
    }

//    if(flags_ & FLAT_BOUNDARY)
//    {
//        Halfedge h;
//        for(EdgeIterator eit = edgesBegin();
//            eit != edgesEnd(); ++eit)
//        {
//            if(!isBoundary(*eit))
//                continue;
//            for(int i = 0; i < 2; ++i)
//            {
//                h = halfedge(*eit, i);
//                if(!isBoundary(h))
//                    edgeGradientNode(h) = addNode(NodeValue::Constant(0));
//            }
//        }
//    }
}


VGMeshWithCurves::NodeValue VGMeshWithCurves::evalValueGradient(
        Curve c, unsigned which, float pos) const
{
    const ValueGradient& g = valueGradient(c, which);

    assert(!g.empty());

    if(pos <= g.begin()->first)
        return g.begin()->second;
    if(pos >= g.rbegin()->first)
        return g.rbegin()->second;

    ValueGradient::const_iterator next = g.upper_bound(pos);
    ValueGradient::const_iterator prev = next;
    --next;

    float alpha = (pos - prev->first) / (next->first - prev->first);
    return (1 - alpha) * prev->second + alpha * next->second;
}


void VGMeshWithCurves::addGradientNodes(
        Node nodes[2], Curve c, unsigned gType, float pos)
{
    bool tear = (gType == VALUE)? valueTear(c): gradientTear(c);

    nodes[LEFT] = valueGradient(c, gType | LEFT).empty()?
                addNode():
                addNode(evalValueGradient(c, gType | LEFT, pos));
    nodes[RIGHT] =
            (!tear)?
                nodes[LEFT]:
                valueGradient(c, gType | RIGHT).empty()?
                            addNode():
                            addNode(evalValueGradient(c, gType | RIGHT, pos));
}
