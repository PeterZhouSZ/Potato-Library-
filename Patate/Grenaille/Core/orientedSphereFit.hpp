

template < class DataPoint, class _WFunctor, typename T>
void 
OrientedSphereFit<DataPoint, _WFunctor, T>::init(const VectorType& evalPos){
  _p    = evalPos;
    
  // initial values
  _sumP     = VectorType::Zero();
  _sumN     = VectorType::Zero();
  _sumDotPN = Scalar(0.0);
  _sumDotPP = Scalar(0.0);
  _sumW     = Scalar(0.0);
    
    
  _uc = Scalar(0.0);
  _ul = VectorType::Zero();
  _uq = Scalar(0.0);
}

template < class DataPoint, class _WFunctor, typename T>
void 
OrientedSphereFit<DataPoint, _WFunctor, T>::addNeighbor(const DataPoint& nei){
    
  // centered basis
  VectorType q = nei.pos()-_p;
  
  // compute weight
  Scalar w = _w.w(q, nei);  
  
  if (w > Scalar(0.)){
    
    // increment matrix
    _sumP     += q * w;
    _sumN     += nei.normal() * w;
    _sumDotPN += w * nei.normal().dot(q);
    _sumDotPP += w * q.squaredNorm();
    _sumW     += w;
  }
}


template < class DataPoint, class _WFunctor, typename T>
void
OrientedSphereFit<DataPoint, _WFunctor, T>::finalize (){
  MULTIARCH_STD_MATH(sqrt);

  // 1. finalize sphere fitting
  Scalar invSumW;
    
  if(_sumW == Scalar(0.)){ // handle planar configurations
    invSumW = 10000000;
    _uq = Scalar(0.);
  }else{
    invSumW = Scalar(1.)/_sumW;

    _uq = Scalar(.5)* (_sumDotPN - invSumW*_sumP.dot(_sumN))
      / (_sumDotPP - invSumW*_sumP.dot(_sumP));
  }
    

  _ul = (_sumN-_sumP*(Scalar(2.)*_uq))*invSumW;
  _uc = -invSumW*(_ul.dot(_sumP) + _sumDotPP*_uq);
    
  // 2. Deal with plane case:
  if (fabs(_uq)<Scalar(1e-9)){
    Scalar s = Scalar(1.) / _ul.norm();
    _ul = s*_ul;
    _uc = s*_uc;
    _uq = Scalar(0.);
  }
}


/*!
  \todo Check Cuda bug with std::min
*/
template < class DataPoint, class _WFunctor, typename T>
typename DataPoint::VectorType
OrientedSphereFit<DataPoint, _WFunctor, T>::project( VectorType q ){
  MULTIARCH_STD_MATH(min);

  // centered basis
  q = q-_p;

  //if(_isPlane)
  //{
  VectorType grad;
  VectorType dir  = _ul+Scalar(2.)*_uq*q;
  Scalar ilg      = Scalar(1.)/dir.norm();
  dir             = dir*ilg;
  Scalar ad       = _uc + _ul.dot(q) +
    _uq * q.squaredNorm();
  Scalar delta    = -ad*min(ilg,Scalar(1.));
  VectorType proj = q + dir*delta;

  for (int i=0 ; i<16 ; ++i)
    {
      grad = _ul+Scalar(2.)*_uq*proj;
      ilg = Scalar(1.)/grad.squaredNorm();
      delta = -(_uc + _ul.dot(proj) +
		_uq * proj.squaredNorm())*min(ilg,1.);
      proj += dir*delta;
    }
  return proj + _p;
  //}
  //return other - _ul * dot(other,_ul) + _uc;
  //return normalize(other-_center) * _r + _center;
}

