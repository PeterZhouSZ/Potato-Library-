#include "vgMeshWithCurvesWriter.h"


VGMeshWithCurveWriter::VGMeshWithCurveWriter(Version version)
    : Base(version) {
}


void VGMeshWithCurveWriter::write(std::ostream& _out, const Mesh& mesh) {
    typedef Mesh::Halfedge Halfedge;
    typedef Mesh::PointConstraint PointConstraint;
    typedef Mesh::Curve Curve;

    Base::write(_out, mesh);

    for(unsigned pci = 0; pci < mesh.nPointConstraints(); ++pci)
    {
        PointConstraint pc(pci);

        _out << "pc " << (mesh.isValueConstraint(pc)? "o": "x") << " "
             << (mesh.isGradientConstraint(pc)? "o": "x") << ";";
        if(mesh.isValueConstraint(pc))
        {
            _out << " " << mesh.value(pc).transpose().format(m_format) << ";";
        }
        if(mesh.isGradientConstraint(pc))
        {
            _out << " " << mesh.gradient(pc).transpose().format(m_format) << ";";
        }
        _out << " " << vertexIndex(mesh.vertex(pc)) << "\n";
    }

    for(unsigned ci = 0; ci < mesh.nCurves(); ++ci)
    {
        Curve curve(ci);

        _out << "dc"
             << " " << (mesh.valueFunction(curve, Mesh::VALUE_LEFT).empty()? "x": "o");
        if(mesh.valueTear(curve))
        {
            _out << "/" << (mesh.valueFunction(curve, Mesh::VALUE_RIGHT).empty()? "x": "o");
        }
        _out << " " << (mesh.valueFunction(curve, Mesh::GRADIENT_LEFT).empty()? "x": "o");
        if(mesh.gradientTear(curve))
        {
            _out << "/" << (mesh.valueFunction(curve, Mesh::GRADIENT_RIGHT).empty()? "x": "o");
        }
        _out << ";";

        if(!mesh.valueFunction(curve, Mesh::VALUE_LEFT).empty())
        {
            writeValueFunction(_out, mesh.valueFunction(curve, Mesh::VALUE_LEFT));
        }
        if(mesh.valueTear(curve) && !mesh.valueFunction(curve, Mesh::VALUE_RIGHT).empty())
        {
            writeValueFunction(_out, mesh.valueFunction(curve, Mesh::VALUE_RIGHT));
        }

        if(!mesh.valueFunction(curve, Mesh::GRADIENT_LEFT).empty())
        {
            writeValueFunction(_out, mesh.valueFunction(curve, Mesh::GRADIENT_LEFT));
        }
        if(mesh.gradientTear(curve) && !mesh.valueFunction(curve, Mesh::GRADIENT_RIGHT).empty())
        {
            writeValueFunction(_out, mesh.valueFunction(curve, Mesh::GRADIENT_RIGHT));
        }

        Halfedge h = mesh.firstHalfedge(curve);
        _out << " " << vertexIndex(mesh.fromVertex(h)) << ":" << mesh.fromCurvePos(h);
        while(h.isValid())
        {
            _out << " " << vertexIndex(mesh.toVertex(h)) << ":" << mesh.toCurvePos(h);
            h = mesh.nextCurveHalfedge(h);
        }
        _out << "\n";
    }
}


void VGMeshWithCurveWriter::writeValueFunction(std::ostream& out, const ValueFunction& vg) const
{
    Eigen::IOFormat fmt = m_format;
    fmt.coeffSeparator = ",";
    for(ValueFunction::ConstIterator stop = vg.begin();
        stop != vg.end(); ++stop)
    {
        out << " " << stop->first << ":" << stop->second.transpose().format(fmt);
    }
    out << ";";
}
