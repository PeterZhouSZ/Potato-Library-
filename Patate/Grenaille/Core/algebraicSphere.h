/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/. 
*/


#ifndef _GRENAILLE_ALGEBRAIC_SPHERE_
#define _GRENAILLE_ALGEBRAIC_SPHERE_

#include "internal.h"

namespace Grenaille
{

/*!
    \brief Algebraic Sphere fitting procedure on oriented point sets
    
    Method published in \cite Guennebaud:2007:APSS
    
    An algebraic hyper-sphere is defined as the \f$0\f$-isosurface of the scalar field
    
    \f$ s_\mathbf{u}(\mathbf{x}) = \left[ 1 \; \mathbf{x}^T \; \mathbf{x}^T\mathbf{x}\right]^T \cdot \mathbf{u} \f$    
    
    with \f$ \mathbf{u} \left[ u_c \; \mathbf{u_l} \; u_q\right]^T \f$ is the 
    vector of the constant, linear and quadratic parameters.
    
    \note If internally the scalar fields are stored in a local frame defined
    by the evaluation position, the public methods involving a query (such as
    project, potential, gradient) have to be defined in global 
    coordinates (e.g. you don't need to convert your query in the current locale
    frame).
    
    This fitting procedure provides: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE \endverbatim

    \todo Deal with planar case
   */
  template < class DataPoint, class _WFunctor, typename T = void  >
  class AlgebraicSphere {
  protected:
    enum
      {
        PROVIDES_ALGEBRAIC_SPHERE /*!< \brief Provides Algebraic Sphere */
      };

  public:
    /*! \brief Scalar type inherited from DataPoint*/
    typedef typename DataPoint::Scalar     Scalar;     
    /*! \brief Vector type inherited from DataPoint*/
    typedef typename DataPoint::VectorType VectorType;
    /*! \brief Vector type inherited from DataPoint*/
    typedef typename DataPoint::MatrixType MatrixType;
    /*! \brief Weight Function*/
    typedef _WFunctor                      WFunctor;  
    
  private:    
    //! \brief Evaluation position (needed for centered basis)
    VectorType _p; 
    
  protected:
    //! Is the implicit scalar field normalized using Pratt
    bool _isNormalized;

    // results
  public:
    Scalar _uc,       /*!< \brief Constant parameter of the Algebraic hyper-sphere */
           _uq;       /*!< \brief Quadratic parameter of the Algebraic hyper-sphere */
    VectorType _ul;   /*!< \brief Linear parameter of the Algebraic hyper-sphere */
    
  public:
    /*! \brief Default constructor */
    MULTIARCH inline AlgebraicSphere(){
      _p = VectorType::Zero();
      resetPrimitive();
    }    
    
    /*! \brief Set the scalar field values to 0 and reset the isNormalized() status */
    MULTIARCH inline void resetPrimitive() {
      _uc = Scalar(0.0);
      _ul = VectorType::Zero();
      _uq = Scalar(0.0);
      _isNormalized = false;
    }    

    /*! \brief Reading access to the basis center (evaluation position) */
    MULTIARCH inline const VectorType& basisCenter () const { return _p; }
    /*! \brief Writing access to the (evaluation position) */
    MULTIARCH inline       VectorType& basisCenter ()       { return _p; }

    /*! \brief compute the Pratt norm of the implicit scalar field. */
    MULTIARCH inline Scalar prattNorm() const {
      MULTIARCH_STD_MATH(sqrt);
      return sqrt(prattNorm2());
    }
    
    /*! \brief compute the squared Pratt norm of the implicit scalar field. */
    MULTIARCH inline Scalar prattNorm2() const {
      return _ul.squaredNorm() - Scalar(4.) * _uc*_uq;
    }

    /*!
       \brief Normalize the scalar field by the Pratt norm
       \return false when the normalization fails (sphere is already normalized)
     */
    MULTIARCH inline bool applyPrattNorm() {
      if (! _isNormalized){
        Scalar pn = prattNorm();
        _uc /= pn;
        _ul *= Scalar(1.)/pn;
        _uq /= pn;

        _isNormalized = true;
      }
      return true;
    }
    
    //! \brief State indicating when the sphere has been normalized 
    MULTIARCH inline bool isNormalized() const { return _isNormalized; }
    
    //! \brief Value of the scalar field at the location \f$ \mathbf{q} \f$
    MULTIARCH inline Scalar potential (const VectorType& q) const;
    
    //! \brief Project a point on the sphere
    MULTIARCH inline VectorType project (const VectorType& q) const;
    
    //! \brief Approximation of the scalar field gradient at \f$ \mathbf{q} \f$
    MULTIARCH inline VectorType primitiveGradient (const VectorType& q) const;
    
    //! \brief Rough Approximation of the hessian matrix at \f$ \mathbf{q} \f$
    MULTIARCH inline MatrixType primitiveHessian (const VectorType &q) const;
  }; //class OrientedSphereFit


#include "algebraicSphere.hpp"

}
#endif  // _GRENAILLE_ALGEBRAIC_SPHERE_
