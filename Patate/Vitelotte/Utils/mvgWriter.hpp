/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mvgWriter.h"


namespace Vitelotte {

template < typename _Mesh >
void
MVGWriter<_Mesh>::write(std::ostream& _out, const Mesh& mesh) const
{
    typedef typename Mesh::VertexIterator VertexIterator;
    typedef typename Mesh::FaceIterator FaceIterator;
    typedef typename Mesh::HalfedgeAroundFaceCirculator
            HalfedgeAroundFaceCirculator;

    assert(m_version == VERSION_1_0);

    // Ensure that the stream we read encode numbers using the C locale
    _out.imbue(std::locale::classic());

    int iOffset = 0;

    _out << "mvg 1.0\n";
    _out << "dim " << Mesh::Dim << "\n";
    _out << "parameters " << Mesh::Chan << "\n";

    if(mesh.getAttributes() == Mesh::LINEAR_FLAGS)
        _out << "linear\n";
    else if(mesh.getAttributes() == Mesh::QUADRATIC_FLAGS)
        _out << "quadratic\n";
    else if(mesh.getAttributes() == Mesh::MORLEY_FLAGS)
        _out << "morley\n";
    else if(mesh.getAttributes() == Mesh::FV_FLAGS)
        _out << "fv\n";
    else
        _out << "mesh " << mesh.getAttributes() << "\n";

    _out << "vertices " << mesh.nVertices() << "\n";
    _out << "nodes " << mesh.nNodes() << "\n";
    _out << "faces " << mesh.nFaces() << "\n";

    for(VertexIterator vit = mesh.verticesBegin();
        vit != mesh.verticesEnd(); ++vit)
    {
        _out << "v " << mesh.position(*vit).transpose() << "\n";
    }

    for(unsigned i = 0; i < mesh.nNodes(); ++i)
    {
        if(mesh.isConstraint(Node(i)))
            _out << "n " << mesh.nodeValue(Node(i)).transpose() << "\n";
        else
            _out << "n void\n";
    }

    for(FaceIterator fit = mesh.facesBegin();
         fit != mesh.facesEnd(); ++fit)
    {
        _out << (mesh.nSingulars(*fit)? "fs": "f");

        HalfedgeAroundFaceCirculator
                hit  = mesh.halfedges(*fit),
                hend = hit;
        do
        {
            _out << " " << mesh.toVertex(*hit).idx() + iOffset;

            if(mesh.hasToVertexValue())
            {
                Node vn = mesh.toVertexValueNode(*hit);
                _out << "/";
                printNode(_out, vn);

                // VertexFromValue only makes sense if vertexValue in enable.
                if(mesh.hasFromVertexValue())
                {
                    Node fn = mesh.fromVertexValueNode(mesh.nextHalfedge(*hit));
                    if(vn != fn)
                    {
                        _out << "/";
                        printNode(_out, fn);
                    }
                }
            }
        }
        while(++hit != hend);

        if(mesh.hasEdgeValue() || mesh.hasEdgeGradient())
        {
            _out << " -";

            --hit; hend = hit;
            do
            {
                char sep = ' ';
                if(mesh.hasEdgeValue())
                {
                    _out << sep;
                    sep = '/';
                    printNode(_out, mesh.edgeValueNode(*hit));
                }
                if(mesh.hasEdgeGradient())
                {
                    _out << sep;
                    sep = '/';
                    printNode(_out, mesh.edgeGradientNode(*hit));
                }
            }
            while(++hit != hend);
        }

        _out << "\n";
    }
}

template < typename _Mesh >
void
MVGWriter<_Mesh>::printNode(std::ostream& _out, Node n) const
{
    if(n.isValid())
        _out << n.idx();
    else
        _out << "x";
}


template < typename Mesh >
void writeMvg(std::ostream& out, const Mesh& mesh,
              typename MVGWriter<Mesh>::Version version)
{
    MVGWriter<Mesh> writer(version);
    writer.write(out, mesh);
}

template < typename Mesh >
void writeMvgToFile(const std::string& filename, const Mesh& mesh,
                    typename MVGWriter<Mesh>::Version version)
{
    std::ofstream out(filename.c_str());
    writeMvg(out, mesh, version);
}


}  // namespace Vitelotte
