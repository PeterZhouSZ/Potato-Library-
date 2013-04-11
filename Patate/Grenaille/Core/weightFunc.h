#ifndef _GRENAILLE_WEIGHT_FUNC_
#define _GRENAILLE_WEIGHT_FUNC_

namespace Grenaille{
  template <class DataPoint, typename Derived >
  class BaseWeightFunc {

  public:        
    typedef typename DataPoint::Scalar Scalar;
    typedef typename DataPoint::VectorType VectorType;
      
    MULTIARCH inline Scalar w(const VectorType& relativeQuery, 
			      const DataPoint&  attributes)
    { return _der().w(relativeQuery, attributes); }   
       
    MULTIARCH inline VectorType spacedw(const VectorType& relativeQuery, 
				    const DataPoint&  attributes)
    { return _der().spacedw(relativeQuery, attributes); }   
       
    MULTIARCH inline Scalar scaledw(const VectorType& relativeQuery, 
				    const DataPoint&  attributes)
    { return _der().scaledw(relativeQuery, attributes); }

  protected:
    MULTIARCH inline Derived& _der() { return &static_cast<Derived*>(this); }    
  };// class BaseWeightFunc


  /*!
    \warning Assumes that the evaluation scale t is strictly positive
   */
  template <class DataPoint, class WeightKernel>
  class DistWeightFunc: public BaseWeightFunc<DataPoint, DistWeightFunc<DataPoint, WeightKernel> >{
  public:
    typedef typename DataPoint::Scalar Scalar;
    typedef typename DataPoint::VectorType VectorType;
    
    MULTIARCH inline DistWeightFunc(const Scalar& t = Scalar(1.)): _t(t) {}

    /*!
      Compute a weight using the norm of the query \f$ \mathbf{q} \f$ 
      (expressed in centered basis)
     */
    MULTIARCH inline Scalar w(const VectorType& q, 
			      const DataPoint&  /*attributes*/);
    
    
    /*!
      First order derivative in space (for each dimension \f$\mathsf{x})\f$:
      
      \f$ \frac{\delta \frac{\mathbf{q}}{t}}{\delta \mathsf{x}} 
      \nabla w(\frac{\mathbf{q}}{t}) 
      = \frac{ \nabla{w(\frac{\mathbf{q}}{t})}}{t}  \f$
      
      where \f$ \mathbf{q} \f$ represent the query coordinate expressed in 
      centered basis.
    */
    MULTIARCH inline VectorType spacedw(const VectorType& q, 
			     const DataPoint&  /*attributes*/);
       
    
    /*!
      First order derivative in scale t:
      
      \f$ \frac{\delta \frac{\mathbf{q}}{t}}{\delta t} 
      \nabla w(\frac{\mathbf{q}}{t}) 
      = - \frac{\mathbf{q}}{t^2} \nabla{w(\frac{\mathbf{q}}{t})} \f$
      
      where \f$ \mathbf{q} \f$ represent the query coordinate expressed in 
      centered basis.
    */
    MULTIARCH inline Scalar scaledw(const VectorType& q, 
			     const DataPoint&  /*attributes*/);

  protected:
    Scalar       _t;
    WeightKernel _wk;
    

  };// class DistWeightFunc

#include "weightFunc.hpp"

}// namespace Grenaille


#endif // _GRENAILLE_WEIGHT_FUNC_
