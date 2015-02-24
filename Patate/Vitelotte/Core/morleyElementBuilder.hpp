/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <Eigen/Dense>

#include "morleyElement.h"

#include "morleyElementBuilder.h"


namespace Vitelotte
{


template < class _Mesh, typename _Scalar >
MorleyElementBuilder<_Mesh, _Scalar>::MorleyElementBuilder(Scalar sigma)
    : m_sigma(sigma)
{
}

template < class _Mesh, typename _Scalar >
unsigned
MorleyElementBuilder<_Mesh, _Scalar>::
    nCoefficients(const Mesh& /*mesh*/, Face /*element*/) const
{
    return 36;
}


template < class _Mesh, typename _Scalar >
template < typename InIt >
void
MorleyElementBuilder<_Mesh, _Scalar>::
    addCoefficients(InIt& it, const Mesh& mesh, Face element)
{
    assert(mesh.valence(element) == 3);

    Vector p[3];
    Vector v[3];
    bool orient[3];
    int nodes[6];

    typename Mesh::HalfedgeAroundFaceCirculator hit = mesh.halfedges(element);
    --hit;
    for(int i = 0; i < 3; ++i)
    {
        v[i] = (mesh.position(mesh.toVertex(*hit)) -
                mesh.position(mesh.fromVertex(*hit))).template cast<Scalar>();
        orient[i] = mesh.halfedgeOrientation(*hit);
        nodes[i+3] = mesh.edgeGradientNode(*hit).idx();
        ++hit;
        nodes[i] = mesh.toVertexValueNode(*hit).idx();
        p[i] = mesh.position(mesh.toVertex(*hit)).template cast<Scalar>();
    }

    for(int i = 0; i < 6; ++i)
    {
        if(nodes[i] < 0)
        {
            error(STATUS_ERROR, "Invalid node");
            return;
        }
    }

    typedef MorleyElement<Scalar> Elem;
    Elem elem(p);

    if(elem.doubleArea() <= 0)
    {
        error(STATUS_WARNING, "Degenerated or reversed triangle");
    }

    typedef Eigen::Matrix<Scalar, 3, 1> Vector3;
    Vector6 dx2;
    Vector6 dxy;
    Vector6 dy2;
    Vector3 bc = Vector3(1, 1, 1) / 3;
    typename Elem::Hessian hessians[6];
    elem.hessian(bc, hessians);

    for(int bi = 0; bi < 6; ++bi)
    {
        dx2(bi) = hessians[bi](0, 0);
        dy2(bi) = hessians[bi](1, 1);
        dxy(bi) = hessians[bi](0, 1);
    }

    for(int i = 0; i < 6; ++i)
    {
        for(int j = 0; j < 6; ++j)
        {
            Scalar value =
                    ((dx2(i) + dy2(i)) * (dx2(j) + dy2(j)) +
                    (1-m_sigma) * ( 2*dxy(i)*dxy(j) - dx2(i)*dy2(j) - dy2(i)*dx2(j)));
            value *= elem.doubleArea() / 2;
            if((i < 3 || orient[i%3]) != (j < 3 || orient[j%3]))
            {
                value *= -1;
            }
            *(it++) = Triplet(nodes[i], nodes[j], value);
        }
    }
}


}
