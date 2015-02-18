/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "singularElementDecorator.h"


namespace Vitelotte
{


template < typename _Element >
unsigned
SingularElementDecorator<_Element>::nCoefficients(
        const Mesh& mesh, Face element) const
{
    return Base::nCoefficients(mesh, element) *
            (mesh.nSingulars(element)? 2: 1);
}

template < typename _Element >
template < typename InIt >
void
SingularElementDecorator<_Element>::addCoefficients(
        InIt& it, const Mesh& mesh, Face element)
{
    typedef typename Base::Mesh Mesh;

    InIt begin = it;
    Base::addCoefficients(it, mesh, element);

    unsigned nSingular = mesh.nSingulars(element);

    if(nSingular > 1)
        Base::error(Base::STATUS_WARNING, "Element with more than one singular vertex");

    if(nSingular) {
        InIt end = it;
        int from, to;
        typename Mesh::HalfedgeAroundFaceCirculator hit = mesh.halfedges(element);
        while(true)
        {
            if(mesh.isSingular(*hit))
            {
                from = mesh.toVertexValueNode(*hit).idx();
                to = mesh.fromVertexValueNode(mesh.nextHalfedge(*hit)).idx();
                break;
            }
            ++hit;
        }

        for(InIt tit=begin; tit != end; ++tit)
        {
            int r = tit->row() == from? to: tit->row();
            int c = tit->col() == from? to: tit->col();

            *(it++) = Triplet(r, c, tit->value());
        }
    }
}


}