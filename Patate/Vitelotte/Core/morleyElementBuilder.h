#ifndef _MORLEY_ELEMENT_BUILDER_H_
#define _MORLEY_ELEMENT_BUILDER_H_


#include <Eigen/Core>
#include <Eigen/Sparse>


#include "elementBuilderBase.h"


namespace Vitelotte
{


template < class _Mesh, typename _Scalar = typename _Mesh::Scalar >
class MorleyElementBuilder : public ElementBuilderBase
{
public:
    typedef _Scalar Scalar;
    typedef _Mesh Mesh;

    typedef Eigen::Matrix<Scalar, Mesh::Dim, 1> Vector;
    typedef Eigen::Triplet<Scalar> Triplet;

    typedef typename Mesh::Face Face;

    typedef Eigen::Matrix<Scalar, 6, 1> Vector6;

public:
    inline MorleyElementBuilder(Scalar sigma = Scalar(.5));

    unsigned nCoefficients(const Mesh& mesh, Face element) const;

    template < typename InIt >
    void addCoefficients(InIt& it, const Mesh& mesh, Face element);

private:
    Scalar m_sigma;
};


} // namespace Vitelotte

#include "morleyElementBuilder.hpp"


#endif
