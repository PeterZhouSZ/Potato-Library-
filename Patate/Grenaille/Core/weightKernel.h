/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/. 
*/


#ifndef _GRENAILLE_WEIGHT_KERNEL_
#define _GRENAILLE_WEIGHT_KERNEL_

/*!
  \file weightKernel.h Define 1D weight kernel functors
 */


namespace Grenaille{

  template <typename _Scalar>
  class ConstantWeightKernel{
  public:
    typedef _Scalar Scalar;
    
    // Init 
    MULTIARCH inline ConstantWeightKernel(const Scalar& value = Scalar(1.)) : _y(value){}
    MULTIARCH inline void setValue(const Scalar& value){ _y = value; } 

    // Functor
    MULTIARCH inline Scalar f  (const Scalar&) const { return _y; }
    MULTIARCH inline Scalar df (const Scalar&) const { return Scalar(0.); }
    MULTIARCH inline Scalar ddf(const Scalar&) const { return Scalar(0.); }

  private: 
    Scalar _y;
  };// class ConstantWeightKernel

  
  /*!
    \todo Add way to specify a degree parameter (in this class or another one)
   */
  template <typename _Scalar>
  class SmoothWeightKernel{
  public:
    typedef _Scalar Scalar;

    // Functor
    /*! Define the smooth weighting function \f$ w(x) = (x^2-1)^2 \f$ */
    MULTIARCH inline Scalar f  (const Scalar& x) const { Scalar v = x*x - Scalar(1.); return v*v; }
    /*! Define the smooth first order weighting function \f$ \nabla w(x) = 4x(x^2-1) \f$ */
    MULTIARCH inline Scalar df (const Scalar& x) const { return Scalar(4.)*x*(x*x-Scalar(1.)); }
    /*! Define the smooth second order weighting function \f$ \nabla^2 w(x) = 12x^2-4 \f$ */
    MULTIARCH inline Scalar ddf(const Scalar& x) const { return Scalar(12.)*x*x - Scalar(4.); }
  };//class SmoothWeightKernel

}// namespace Grenaille


#endif //_GRENAILLE_WEIGHT_KERNEL_
