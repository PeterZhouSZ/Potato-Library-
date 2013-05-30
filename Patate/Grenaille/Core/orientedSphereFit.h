/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/. 
*/


#ifndef _GRENAILLE_ORIENTED_SPHERE_FIT_
#define _GRENAILLE_ORIENTED_SPHERE_FIT_

#include "internal.h"

namespace Grenaille
{
  /*!
    \brief Algebraic Sphere Fitting on oriented point sets
    

    Provide: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE \endverbatim

    \todo Deal with planar case
   */
  template < class DataPoint, class _WFunctor, typename T = void >
  class OrientedSphereFit {
  protected:
    enum
      {
        PROVIDES_ALGEBRAIC_SPHERE
      };

  public:
    typedef typename DataPoint::Scalar     Scalar;
    typedef typename DataPoint::VectorType VectorType;
    typedef _WFunctor             WFunctor;
    
  protected:
    // Evaluation position (needed for centered basis)
    VectorType _p;
    
    // computation data
    VectorType _sumN, _sumP;
    Scalar _sumDotPN, _sumDotPP, _sumW;
    
    WFunctor _w;

    // Is the implicit scalar field normalized using Pratt
    bool _isNormalized;

    // results
  public:
    Scalar _uc, _uq;
    VectorType _ul;
    
  public:
    MULTIARCH inline OrientedSphereFit(){ }

    // getters
    MULTIARCH inline const VectorType& evalPos () const { return _p; }
    MULTIARCH inline       VectorType& evalPos ()       { return _p; }
    
    // init
    MULTIARCH inline void setWeightFunc (const WFunctor& w) { _w  = w; }
    MULTIARCH inline void init (const VectorType& evalPos);

    // processing
    MULTIARCH inline void addNeighbor(const DataPoint &nei);
    MULTIARCH inline void finalize   ();
    
    //! compute the Pratt norm of the implicit scalar field.
    MULTIARCH inline Scalar prattNorm() const {
      MULTIARCH_STD_MATH(sqrt);
      return sqrt(prattNorm2());
    }
    
    //! compute the squared Pratt norm of the implicit scalar field.
    MULTIARCH inline Scalar prattNorm2() const {
      return _ul.squaredNorm() - Scalar(4.) * _uc*_uq;
    }

    //! Normalize the scalar field by the Pratt norm
    /*!
      Return false when an error occured during the normalization.
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
    
    //! State that indicates if the sphere has been normalized 
    MULTIARCH inline bool isNormalized() const { return _isNormalized; }
    
    //! Project a point on the sphere
    MULTIARCH inline VectorType project (VectorType q);

    
    //    MULTIARCH VectorType gradient(VectorType q, bool normalize = true);

  }; //class OrientedSphereFit


  namespace internal{

    enum {
      FitScaleDer = 0x01,
      FitSpaceDer = 0x02
    };

    /*!
      Internal generic class describing the Fit derivation
     */
    template < class DataPoint, class _WFunctor, typename T, int Type>
    class OrientedSphereDer : public T{
    private:
      typedef T Base;

    protected:
      enum
	{
	  Check = Base::PROVIDES_ALGEBRAIC_SPHERE,
	  PROVIDES_ALGEBRAIC_SPHERE_DERIVATIVE
	};

    public:
      typedef typename Base::Scalar     Scalar;
      typedef typename Base::VectorType VectorType;
      typedef typename Base::WFunctor   WFunctor;

#define GLS_DER_NB_DERIVATIVES(TYPE,DIM) ((TYPE & FitScaleDer) ? 1 : 0 ) + ((TYPE & FitSpaceDer) ? DIM : 0)
      typedef FixedSizeArray <VectorType, GLS_DER_NB_DERIVATIVES(Type,DataPoint::Dim)> VectorArray;
      typedef FixedSizeArray <Scalar,     GLS_DER_NB_DERIVATIVES(Type,DataPoint::Dim)> ScalarArray;
      
    private:
      // computation data
      VectorArray _dSumN, _dSumP;
      ScalarArray _dSumDotPN, _dSumDotPP, _dSumW;
      
    public:
      // results
      VectorArray _dUl;
      ScalarArray _dUc, _dUq;

      
      // processing
      MULTIARCH void init       (const VectorType &evalPos);
      MULTIARCH void addNeighbor(const DataPoint  &nei);
      MULTIARCH void finalize   ();
    
      //! compute the squared Pratt norm derivative in dimension d
      MULTIARCH inline Scalar dprattNorm2(unsigned int d) const {
        return Scalar(2.) * _dUl[d].dot(Base::_ul) - Scalar(4.)*_dUc[d]*Base::_uq
	  - Scalar(4.)* Base::_uc*_dUq[d];}

      //! compute the Pratt norm derivative in dimension d
      MULTIARCH inline Scalar dprattNorm(unsigned int d) const {
	MULTIARCH_STD_MATH(sqrt);
        return sqrt(dprattNorm2(d));
      }

      MULTIARCH inline bool isScaleDer() const {return Type & FitScaleDer;}
      MULTIARCH inline bool isSpaceDer() const {return Type & FitSpaceDer;}
      MULTIARCH inline unsigned int derDimension() const { return VectorArray::size();}


      //! Normalize the scalar field by the Pratt norm
      /*!
	\warning Requieres that isNormalized() return false
	\return false when the original sphere has already been normalized.
       */
      MULTIARCH inline bool applyPrattNorm();

    }; //class OrientedSphereFitDer

  }// namespace internal  

  /*!
    \brief Differentiation in scale of the OrientedSphereFit
    
    Requierement: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE \endverbatim
    Provide: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE_SCALE_DERIVATIVE \endverbatim
  */
  template < class DataPoint, class _WFunctor, typename T>
  class OrientedSphereScaleDer:public internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitScaleDer>
  {
  protected:
    typedef internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitScaleDer> Base;
    enum { PROVIDES_ALGEBRAIC_SPHERE_SCALE_DERIVATIVE };
  };


  /*!
    \brief Spatial differentiation of the OrientedSphereFit
    
    Requierement: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE \endverbatim
    Provide: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE_SPACE_DERIVATIVE \endverbatim
  */
  template < class DataPoint, class _WFunctor, typename T>
  class OrientedSphereSpaceDer:public internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitSpaceDer>
  {
  protected:
    typedef internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitSpaceDer> Base;
    enum {  PROVIDES_ALGEBRAIC_SPHERE_SPACE_DERIVATIVE };
  };


  /*!
    \brief Differentiation both in scale and space of the OrientedSphereFit
    
    Requierement: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE \endverbatim
    Provide: 
    \verbatim PROVIDES_ALGEBRAIC_SPHERE_SCALE_DERIVATIVE 
    PROVIDES_ALGEBRAIC_SPHERE_SPACE_DERIVATIVE
    \endverbatim
  */
  template < class DataPoint, class _WFunctor, typename T>
  class OrientedSphereScaleSpaceDer:public internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitSpaceDer | internal::FitScaleDer>
  {
  protected:
    typedef internal::OrientedSphereDer<DataPoint, _WFunctor, T, internal::FitSpaceDer | internal::FitScaleDer> Base;
    enum
      {
        PROVIDES_ALGEBRAIC_SPHERE_SCALE_DERIVATIVE,
        PROVIDES_ALGEBRAIC_SPHERE_SPACE_DERIVATIVE
      };
  };


#include "orientedSphereFit.hpp"

} //namespace Grenaille


#endif
