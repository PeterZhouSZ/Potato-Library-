/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mvgReader.h"


namespace Vitelotte
{


template < typename _Mesh >
MVGReader<_Mesh>::MVGReader()
{
    m_faceIndices.reserve(3);
}


template < typename _Mesh >
bool
MVGReader<_Mesh>::read(std::istream& in, Mesh& mesh)
{
    m_mesh = &mesh;
    bool r = doRead(in);
    m_mesh = 0;

    return r;
}


template < typename _Mesh >
void
MVGReader<_Mesh>::parseHeader(std::istream& in)
{
    m_lineStream >> m_tmp;
    if(!in) error("Failed to read header");
    if(m_tmp != "mvg") error("Missing mvg header");

    m_lineStream >> m_tmp;
    if(!in) error("Failed to read header");
    if(m_tmp != "1.0") error("Unsuported version");

    std::string cmd;
    unsigned nDims = 2;
    unsigned nCoeffs = 4;
    unsigned attributes = 0;
    unsigned nVert = 1024;
    unsigned nNode = 1024;
    unsigned nFace = 1024;
    while(true)
    {
        readLine(in);
        m_lineStream >> cmd;

        if(cmd == "dim")
            m_lineStream >> nDims;
        else if(cmd == "parameters" || cmd == "coefficients")
            m_lineStream >> nCoeffs;
        else if(cmd == "linear")
            attributes = Mesh::LINEAR_FLAGS;
        else if(cmd == "quadratic")
            attributes = Mesh::QUADRATIC_FLAGS;
        else if(cmd == "morley")
            attributes = Mesh::MORLEY_FLAGS;
        else if(cmd == "fv")
            attributes = Mesh::FV_FLAGS;
        else if(cmd == "attributes")
        {
            m_lineStream >> cmd;
            if(cmd == "none")
                attributes = 0;
            else if(cmd == "linear")
                attributes = Mesh::LINEAR_FLAGS;
            else if(cmd == "quadratic")
                attributes = Mesh::QUADRATIC_FLAGS;
            else if(cmd == "morley")
                attributes = Mesh::MORLEY_FLAGS;
            else if(cmd == "fv")
                attributes = Mesh::FV_FLAGS;
        }
        else if(cmd == "mesh")
            m_lineStream >> attributes;
        else if(cmd == "vertices")
            m_lineStream >> nVert;
        else if(cmd == "nodes")
            m_lineStream >> nNode;
        else if(cmd == "faces")
            m_lineStream >> nFace;
        else
            break;

        if(!in || !m_lineStream) error("Failed to read header");
    }
    m_lineStream.seekg(0);

    if(int(Mesh::DimsAtCompileTime) != int(Dynamic) && nDims != m_mesh->nDims())
    {
        error("Invalid number of dimensions.");
        return;
    }
    if(int(Mesh::CoeffsAtCompileTime) != int(Dynamic) && nCoeffs != m_mesh->nCoeffs())
    {
        error("Invalid number of coefficients.");
        return;
    }

    m_mesh->clear();
    // TODO: set / check dims and parameters
    m_mesh->setAttributes(attributes);
    m_mesh->setNDims(nDims);
    m_mesh->setNCoeffs(nCoeffs);
    m_mesh->reserve(nVert, nVert+nFace, nFace, nNode);
}


template < typename _Mesh >
bool
MVGReader<_Mesh>::parseDefinition(const std::string& spec,
                                   std::istream& def)
{
    typedef typename Mesh::Node Node;

    int iOffset = 0;

    // vertex
    if(spec == "v")
    {
        Vector p(m_mesh->nDims());
        for(unsigned i = 0; i < m_mesh->nDims(); ++i)
            def >> p[i];
        if(!def) error("Failed to read vertex (not enought components ?)");
        def >> std::ws;
        if(!def.eof()) warning("Too much components.");
        m_mesh->addVertex(p);
    }

    // nodes
    else if(spec == "n")
    {
        Value n(m_mesh->nCoeffs());
        def >> std::ws;
        if(def.peek() == 'v')
            n = m_mesh->unconstrainedValue();
        else
        {
            for(unsigned i = 0; i < m_mesh->nCoeffs(); ++i)
                def >> n[i];
            if(!def) error("Failed to read node (wrong number of components ?)");
            def >> std::ws;
            if(!def.eof()) warning("Too much components.");
        }
        m_mesh->addNode(n);
    }

    // face
    // TODO: "fs" is deprecated. Remove it.
    else if(spec == "f" || spec == "fs")
    {
        m_fVertices.clear();

        def >> std::ws;
        int nodes[12];

        for(int i = 0; i < 3; ++i)
        {
            def >> m_tmp;
            parseIndiceList(m_tmp, m_faceIndices);
            if(m_faceIndices.size() < 1 || m_faceIndices.size() > 3)
                error("Invalid number of indices");

            m_fVertices.push_back(
                        typename Mesh::Vertex(m_faceIndices[0] - iOffset));

            if(m_faceIndices.size() > 1)
            {
                nodes[2*i + 0] = m_faceIndices[1];
                nodes[2*i + 1] = m_faceIndices.back();
            }
        }

        // mid nodes
        unsigned nEAttrs = m_mesh->hasEdgeValue() + m_mesh->hasEdgeGradient();
        if(nEAttrs)
        {
            def >> m_tmp;
            if(m_tmp != "-")
                error("Only triangles meshes are supported");

            for(int i = 0; i < 3; ++i)
            {
                def >> m_tmp;
                parseIndiceList(m_tmp, m_faceIndices);
                if(m_faceIndices.size() != nEAttrs)
                    error("Invalid number of indices");

                if(m_mesh->hasEdgeValue())
                    nodes[6+i] = m_faceIndices.front();
                if(m_mesh->hasEdgeGradient())
                    nodes[9+i] = m_faceIndices.back();
            }
        }

        typename Mesh::Face f = m_mesh->addFace(m_fVertices);

        typename Mesh::HalfedgeAroundFaceCirculator hit = m_mesh->halfedges(f);
        for(int i = 0; i < 3; ++i)
        {
            if(m_mesh->hasToVertexValue())
                m_mesh->toVertexValueNode(*hit) = Node(nodes[2*i] - iOffset);
            ++hit;
            if(m_mesh->hasFromVertexValue())
                m_mesh->fromVertexValueNode(*hit) = Node(nodes[2*i + 1] - iOffset);
            if(m_mesh->hasEdgeValue())
                m_mesh->edgeValueNode(*hit) = Node(nodes[6 + (i+2)%3] - iOffset);
            if(m_mesh->hasEdgeGradient())
                m_mesh->edgeGradientNode(*hit) = Node(nodes[9 + (i+2)%3] - iOffset);
        }
    }

    else if(spec == "vgc")
    {
        typedef typename Mesh::Gradient Gradient;
        unsigned vxIdx;
        def >> vxIdx;
        if(!def) error("Failed to read vertex gradient constraint's vertex index");
        if(vxIdx >= m_mesh->nVertices()) error("Invalid vertex index");

        Gradient grad(m_mesh->nCoeffs(), m_mesh->nDims());
        for(unsigned col = 0; col < m_mesh->nDims(); ++col)
        {
            for(unsigned row = 0; row < m_mesh->nCoeffs(); ++row)
            {
                def >> grad(row, col);
            }
        }
        if(!def) error("Error while reading gradient");

        m_mesh->setGradientConstraint(Vertex(vxIdx), grad);
    }

    // Unknown element type.
    else
    {
        warning("Unknown spec: " + spec);
        return false;
    }
    return true;
}


template < typename Mesh >
bool readMvg(std::istream& in, Mesh& mesh)
{
    MVGReader<Mesh> reader;
    return reader.read(in, mesh);
}

template < typename Mesh >
bool readMvgFromFile(const std::string& filename, Mesh& mesh)
{
    std::ifstream in(filename.c_str());
    return readMvg(in, mesh);
}


}  // namespace Vitelotte
