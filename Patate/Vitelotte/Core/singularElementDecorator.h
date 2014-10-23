#ifndef _SINGULAR_ELEMENT_DECORATOR_H_
#define _SINGULAR_ELEMENT_DECORATOR_H_


#include <vector>

#include <Eigen/Core>
#include <Eigen/Sparse>


namespace Vitelotte
{


template < class _Element >
class SingularElementDecorator
{
public:
    typedef _Element Element;

    typedef typename Element::Scalar Scalar;
    typedef typename Element::Mesh Mesh;

    typedef typename Element::Vector Vector;
    typedef typename Element::Triplet Triplet;
    typedef typename Element::StiffnessMatrix StiffnessMatrx;

    typedef typename Element::TripletVector TripletVector;
    typedef typename Element::TripletVectorIterator TripletVectorIterator;

    typedef typename Mesh::Face Face;


public:
    inline explicit SingularElementDecorator(const Element& element=Element())
        : m_element(element) {}

    unsigned nCoefficients(const Mesh& mesh, Face element) const;
    void addCoefficients(TripletVectorIterator& it,
                         const Mesh& mesh, Face element) const;

    Element& element() { return m_element; }
    const Element& element() const { return m_element; }

private:
    Element m_element;
};


#include "singularElementDecorator.hpp"

} // namespace Vitelotte

#endif
