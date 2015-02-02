#ifndef _EXAMPLES_VITELOTTE_COMMON_VG_MESH_WITH_CURVES_
#define _EXAMPLES_VITELOTTE_COMMON_VG_MESH_WITH_CURVES_


#include <vector>

#include <Patate/vitelotte.h>


class VGMeshWithCurves : public Vitelotte::VGMesh<float>
{
public:
    typedef Vitelotte::VGMesh<float> Base;

    typedef Base::Vector Vector;
    typedef Base::NodeValue NodeValue;

    typedef std::map<float, NodeValue> ValueGradient;

    struct Curve : public BaseHandle
    {
        explicit Curve(int _idx = -1) : BaseHandle(_idx) {}
        std::ostream& operator<<(std::ostream& os) const { return os << 'c' << idx(); }
    };

    struct PointConstraint : public BaseHandle
    {
        explicit PointConstraint(int _idx = -1) : BaseHandle(_idx) {}
        std::ostream& operator<<(std::ostream& os) const { return os << "pc" << idx(); }
    };

    struct HalfedgeCurveConnectivity
    {
        Curve curve;
        Halfedge next;
        float pos;
    };

    struct CurveInfo
    {
        Halfedge firstHalfedge;
        Halfedge lastHalfedge;
        unsigned flags;
        ValueGradient gradient[4];
    };

    struct PointConstraintInfo
    {
        Vertex vertex;
        NodeValue value;
        NodeValue xGradient;
        NodeValue yGradient;
    };

    enum
    {
        VALUE_TEAR      = 0x01,
        GRADIENT_TEAR   = 0x02
    };

    enum
    {
        LEFT        = 0x00,
        RIGHT       = 0x01,
        VALUE       = 0x00,
        GRADIENT    = 0x02,

        VALUE_LEFT      = VALUE     | LEFT,
        VALUE_RIGHT     = VALUE     | RIGHT,
        GRADIENT_LEFT   = GRADIENT  | LEFT,
        GRADIENT_RIGHT  = GRADIENT  | RIGHT
    };

public:
    VGMeshWithCurves();

    inline Curve  curve(Halfedge h) const { return m_halfedgeCurveConn[h].curve; }
    inline Curve& curve(Halfedge h)       { return m_halfedgeCurveConn[h].curve; }

    inline float  fromCurvePos(Halfedge h) const
        { return m_halfedgeCurveConn[oppositeHalfedge(h)].pos; }
    inline float& fromCurvePos(Halfedge h)
        { return m_halfedgeCurveConn[oppositeHalfedge(h)].pos; }
    inline float  toCurvePos(Halfedge h) const { return m_halfedgeCurveConn[h].pos; }
    inline float& toCurvePos(Halfedge h)       { return m_halfedgeCurveConn[h].pos; }

    inline Halfedge  nextCurveHalfedge(Halfedge h) const { return m_halfedgeCurveConn[h].next; }
    inline Halfedge& nextCurveHalfedge(Halfedge h)       { return m_halfedgeCurveConn[h].next; }
    inline Halfedge  prevCurveHalfedge(Halfedge h) const
        { return m_halfedgeCurveConn[oppositeHalfedge(h)].next; }
    inline Halfedge& prevCurveHalfedge(Halfedge h)
        { return m_halfedgeCurveConn[oppositeHalfedge(h)].next; }

    inline unsigned nCurves() const { return m_curves.size(); }
    Curve addCurve(unsigned flags);

    using Base::isValid;
    inline bool isValid(PointConstraint pc) const
        { return pc.isValid() && unsigned(pc.idx()) < nPointConstraints(); }
    inline bool isValid(Curve c) const { return c.isValid() && unsigned(c.idx()) < nCurves(); }

    void addHalfedgeToCurve(Curve c, Halfedge h, float from, float to);

    inline Halfedge  firstHalfedge(Curve c) const { return m_curves.at(c.idx()).firstHalfedge; }
    inline Halfedge& firstHalfedge(Curve c)       { return m_curves.at(c.idx()).firstHalfedge; }
    inline Halfedge   lastHalfedge(Curve c) const { return m_curves.at(c.idx()). lastHalfedge; }
    inline Halfedge&  lastHalfedge(Curve c)       { return m_curves.at(c.idx()). lastHalfedge; }

    inline bool valueTear(Curve c)    const { return m_curves.at(c.idx()).flags & VALUE_TEAR;    }
    inline bool gradientTear(Curve c) const { return m_curves.at(c.idx()).flags & GRADIENT_TEAR; }

    inline unsigned flags(Curve c) const { return m_curves.at(c.idx()).flags; }
    void setFlags(Curve c, unsigned flags);

    const ValueGradient& valueGradient(Curve c, unsigned which) const;
          ValueGradient& valueGradient(Curve c, unsigned which);


    inline PointConstraint  pointConstraint(Vertex v) const
      { return m_pointConstraintConn[v]; }
    inline PointConstraint& pointConstraint(Vertex v)
      { return m_pointConstraintConn[v]; }

    using Base::vertex;
    inline Vertex  vertex(PointConstraint pc) const { return m_pointConstraints[pc.idx()].vertex; }
    inline Vertex& vertex(PointConstraint pc)       { return m_pointConstraints[pc.idx()].vertex; }

    inline bool isValueConstraint(PointConstraint pc) const
        { return !isnan(m_pointConstraints[pc.idx()].value(0)); }
    inline bool isGradientConstraint(PointConstraint pc) const
        { return !isnan(m_pointConstraints[pc.idx()].xGradient(0)); }

    inline const NodeValue& value(PointConstraint pc) const
        { return m_pointConstraints[pc.idx()].value; }
    inline       NodeValue& value(PointConstraint pc)
        { return m_pointConstraints[pc.idx()].value; }

    inline const NodeValue& xGradient(PointConstraint pc) const
        { return m_pointConstraints[pc.idx()].xGradient; }
    inline       NodeValue& xGradient(PointConstraint pc)
        { return m_pointConstraints[pc.idx()].xGradient; }
    inline const NodeValue& yGradient(PointConstraint pc) const
        { return m_pointConstraints[pc.idx()].yGradient; }
    inline       NodeValue& yGradient(PointConstraint pc)
        { return m_pointConstraints[pc.idx()].yGradient; }

    inline unsigned nPointConstraints() const { return m_pointConstraints.size(); }
    PointConstraint addPointConstraint();


    void clear();
    void setNodesFromCurves();

    NodeValue evalValueGradient(Curve c, unsigned which, float pos) const;


private:
    typedef Node NodePair[2];
    void addGradientNodes(Node nodes[2], Curve c, unsigned gType, float pos);

private:
    Base::VertexProperty<PointConstraint> m_pointConstraintConn;
    Base::HalfedgeProperty<HalfedgeCurveConnectivity> m_halfedgeCurveConn;

    std::vector<PointConstraintInfo> m_pointConstraints;
    std::vector<CurveInfo> m_curves;

};


#endif
