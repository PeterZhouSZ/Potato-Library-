
template < class _Mesh, typename _Scalar >
void
QuadraticElement<_Mesh, _Scalar>::initializeMatrices()
{
    if(m_matricesInitialized)
        return;
    m_matricesInitialized = true;

    // 'Magic' initialization of base matrices for quadratic elements.
    // See Paul Tsipouras, Compact representation of triangular finite
    //	elements for Poisson's equation, International Journal for
    //	Numerical Methods in Engineering, Volume 11, Issue 3, pages 419-430,
    //	1977

    const Scalar _1_6 = Scalar(1.) / Scalar(6.);
    const Scalar _2_3 = Scalar(2.) / Scalar(3.);
    const Scalar _4_3 = Scalar(4.) / Scalar(3.);

    m_quadM1 <<
            1,  _1_6,  _1_6,     0, -_2_3, -_2_3,
         _1_6,     0, -_1_6,  _2_3,     0, -_2_3,
         _1_6, -_1_6,     0,  _2_3, -_2_3,     0,
            0,  _2_3,  _2_3,  _4_3, -_4_3, -_4_3,
        -_2_3,     0, -_2_3, -_4_3,  _4_3,  _4_3,
        -_2_3, -_2_3,     0, -_4_3,  _4_3,  _4_3;

    m_quadM2 <<
            0,  _1_6, -_1_6,     0,  _2_3, -_2_3,
         _1_6,     1,  _1_6, -_2_3,     0, -_2_3,
        -_1_6,  _1_6,     0, -_2_3,  _2_3,     0,
            0, -_2_3, -_2_3,  _4_3, -_4_3,  _4_3,
         _2_3,     0,  _2_3, -_4_3,  _4_3, -_4_3,
        -_2_3, -_2_3,     0,  _4_3, -_4_3,  _4_3;

    m_quadM3 <<
            0, -_1_6,  _1_6,     0, -_2_3,  _2_3,
        -_1_6,     0,  _1_6, -_2_3,     0,  _2_3,
         _1_6,  _1_6,     1, -_2_3, -_2_3,     0,
            0, -_2_3, -_2_3,  _4_3,  _4_3, -_4_3,
        -_2_3,     0, -_2_3,  _4_3,  _4_3, -_4_3,
         _2_3,  _2_3,     0, -_4_3, -_4_3,  _4_3;
}

template < class _Mesh, typename _Scalar >
QuadraticElement<_Mesh, _Scalar>::QuadraticElement()
{
    initializeMatrices();
}

template < class _Mesh, typename _Scalar >
unsigned
QuadraticElement<_Mesh, _Scalar>::
    nCoefficients(const Mesh& mesh, Face element) const
{
    return mesh.isSingular(element)? 47: 36;
}


template < class _Mesh, typename _Scalar >
template < typename InIt >
void
QuadraticElement<_Mesh, _Scalar>::
    addCoefficients(InIt& it, const Mesh& mesh, Face element) const
{
    assert(mesh.valence(element) == 3);

    Vector v[3];
    unsigned nodes[6];
    int singularIndex = -1;  // -1 means that there is no singularity
    unsigned singularNode = Mesh::InvalidNodeID;

    typename Mesh::HalfedgeAroundFaceCirculator hit = mesh.halfedges(element);
    --hit;
    for(int i = 0; i < 3; ++i)
    {
        v[i] = (mesh.position(mesh.toVertex(*hit)) -
                mesh.position(mesh.fromVertex(*hit))).template cast<Scalar>();
        std::cout << "v" << i << " = " << v[i].transpose() << "\n";
        nodes[3+i] = mesh.midNode(*hit);
        ++hit;
        nodes[i] = mesh.toNode(*hit);
        if(mesh.isSingular(*hit))
        {
            assert(singularIndex == -1);  // Multiple singularities not supported
            singularIndex = i;
            singularNode = mesh.fromNode(mesh.nextHalfedge(*hit));
        }
    }

    for(int i = 0; i < 6; ++i) std::cout << "n" << i << " = " << nodes[i] << "\n";
    std::cout << "si: " << singularIndex << "\n";
    std::cout << "sn: " << singularNode << "\n";

    FemScalar inv4A = 1. / (2. * det2(v[0], v[1]));

    assert(inv4A > 0);

    ElementStiffnessMatrix matrix = (m_quadM1 * v[0].squaredNorm() +
                                     m_quadM2 * v[1].squaredNorm() +
                                     m_quadM3 * v[2].squaredNorm()) * inv4A;

    std::cout << "Elenent stiffness matrix:\n" << matrix << "\n";

    for(int i = 0; i < 6; ++i)
    {
        for(int j = 0; j < 6; ++j)
        {
            // Singular elements are in facts two overlapped elements. So we
            // just multiply common triplets by 2.
            Scalar mult = (singularIndex == -1 ||
                           i == singularIndex ||
                           j == singularIndex)? Scalar(1): Scalar(2);
            *(it++) = Triplet(nodes[i], nodes[j], matrix(i, j) * mult);
        }
    }

    if(singularIndex != -1)
    {
        // Add triplets for the overlapped element that are not already here
        for(int i = 0; i < 6; ++i)
        {
            *(it++) = Triplet(nodes[i], singularNode,
                              matrix(i, singularIndex));
            if(i != singularIndex)
                *(it++) = Triplet(singularNode, nodes[i],
                                  matrix(singularIndex, i));
        }
    }
}

template < class _Mesh, typename _Scalar >
bool QuadraticElement<_Mesh, _Scalar>::m_matricesInitialized = false;

template < class _Mesh, typename _Scalar >
typename QuadraticElement<_Mesh, _Scalar>::ElementStiffnessMatrix
    QuadraticElement<_Mesh, _Scalar>::m_quadM1;

template < class _Mesh, typename _Scalar >
typename QuadraticElement<_Mesh, _Scalar>::ElementStiffnessMatrix
    QuadraticElement<_Mesh, _Scalar>::m_quadM2;

template < class _Mesh, typename _Scalar >
typename QuadraticElement<_Mesh, _Scalar>::ElementStiffnessMatrix
    QuadraticElement<_Mesh, _Scalar>::m_quadM3;
