/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/. 
*/




template < class DataPoint, class _WFunctor, typename T>
void 
OrientedSphereFit<DataPoint, _WFunctor, T>::init(const VectorType& evalPos){
  
  // Setup primitive
  Base::resetPrimitive();
  Base::basisCenter() = evalPos;
    
  // Setup fitting internal values
  _sumP     = VectorType::Zero();
  _sumN     = VectorType::Zero();
  _sumDotPN = Scalar(0.0);
  _sumDotPP = Scalar(0.0);
  _sumW     = Scalar(0.0);
}

template < class DataPoint, class _WFunctor, typename T>
void 
OrientedSphereFit<DataPoint, _WFunctor, T>::addNeighbor(const DataPoint& nei){
    
  // centered basis
  VectorType q = nei.pos() - Base::basisCenter();
  
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
    Base::_uq = Scalar(0.);
  }else{
    invSumW = Scalar(1.)/_sumW;

    Base::_uq = Scalar(.5)* (_sumDotPN - invSumW*_sumP.dot(_sumN))
      / (_sumDotPP - invSumW*_sumP.dot(_sumP));
  }    

  Base::_ul = (_sumN-_sumP*(Scalar(2.)*Base::_uq))*invSumW;
  Base::_uc = -invSumW*(Base::_ul.dot(_sumP) + _sumDotPP*Base::_uq);
    
  // 2. Deal with plane case:
  if (fabs(Base::_uq)<Scalar(1e-9)){
    Scalar s = Scalar(1.) / Base::_ul.norm();
    Base::_ul = s*Base::_ul;
    Base::_uc = s*Base::_uc;
    Base::_uq = Scalar(0.);
  }

  Base::_isNormalized = false;
}




namespace internal{

  template < class DataPoint, class _WFunctor, typename T, int Type>
  void 
  OrientedSphereDer<DataPoint, _WFunctor, T, Type>::init(const VectorType& evalPos){
    Base::init(evalPos);

    for (unsigned int d = 0; d < derDimension(); d++){

      _dSumN[d]     = VectorType::Zero();
      _dSumP[d]     = VectorType::Zero();
        
      _dSumDotPN[d] = Scalar(0.0);
      _dSumDotPP[d] = Scalar(0.0);
      _dSumW[d]     = Scalar(0.0);
        
      _dUc[d] = Scalar(0.0);
      _dUq[d] = Scalar(0.0);
      _dUl[d] = VectorType::Zero();
    }
  }


  template < class DataPoint, class _WFunctor, typename T, int Type>
  void 
  OrientedSphereDer<DataPoint, _WFunctor, T, Type>::addNeighbor(const DataPoint  &nei){
    Base::addNeighbor(nei);

    int spaceId = (Type & FitScaleDer) ? 1 : 0;

    ScalarArray w;

    // centered basis
    VectorType q = nei.pos()-Base::basisCenter();

    // compute weight
    if (Type & FitScaleDer)
      w[0] = Base::_w.scaledw(q, nei);

    if (Type & FitSpaceDer){
      VectorType vw = Base::_w.spacedw(q, nei);
      for(unsigned int i = 0; i < DataPoint::Dim; i++)
	      w [spaceId+i] = vw[i];
    }

    // increment
    for (unsigned int d = 0; d < derDimension(); d++){
      _dSumW[d]     += w[d];
      _dSumP[d]     += w[d] * q;
      _dSumN[d]     += w[d] * nei.normal();
      _dSumDotPN[d] += w[d] * nei.normal().dot(q);
      _dSumDotPP[d] += w[d] * q.squaredNorm();
    }
  }


  template < class DataPoint, class _WFunctor, typename T, int Type>
  void 
  OrientedSphereDer<DataPoint, _WFunctor, T, Type>::finalize(){
    MULTIARCH_STD_MATH(sqrt);

    Base::finalize();
    
    if (Base::_sumW != Scalar(0.)){

      Scalar invSumW = Scalar(1.)/Base::_sumW;

      Scalar nume  = Base::_sumDotPN - invSumW*Base::_sumP.dot(Base::_sumN);
      Scalar deno  = Base::_sumDotPP - invSumW*Base::_sumP.dot(Base::_sumP);

      // increment
      for (unsigned int d = 0; d < derDimension(); d++){
	Scalar dNume = _dSumDotPN[d] - invSumW*invSumW*(
							Base::_sumW*(_dSumP[d].dot(Base::_sumN)+Base::_sumP.dot(_dSumN[d])) - _dSumW[d]*Base::_sumP.dot(Base::_sumN));
	Scalar dDeno = _dSumDotPP[d] - invSumW*invSumW*( Scalar(2.)*Base::_sumW*_dSumP[d].dot(Base::_sumP)
							 - _dSumW[d]*Base::_sumP.dot(Base::_sumP));

	_dUq[d] = Scalar(.5) * (deno * dNume - dDeno * nume)/(deno*deno);
	_dUl[d] = invSumW*((_dSumN[d] - Scalar(2.)*(_dSumP[d]*Base::_uq+Base::_sumP*_dUq[d])) - _dSumW[d]*Base::_ul);
	_dUc[d] = -invSumW*( _dUl[d].dot(Base::_sumP) + _dUq[d]*Base::_sumDotPP + Base::_ul.dot(_dSumP[d])
			     + Base::_uq*_dSumDotPP[d] + _dSumW[d]*Base::_uc);
      }
    }

  }

  
  template < class DataPoint, class _WFunctor, typename T, int Type>
  bool
  OrientedSphereDer<DataPoint, _WFunctor, T, Type>::applyPrattNorm() {
    if(Base::isNormalized())
      return false; //need original parameters without Pratt Normalization

    MULTIARCH_STD_MATH(sqrt);
    Scalar pn2    = Base::prattNorm2();
    Scalar pn     = sqrt(pn2);
    
    for (unsigned int d = 0; d < derDimension(); d++){
      Scalar dpn2   = dprattNorm2(d);
      Scalar factor = Scalar(0.5) * dpn2 / pn;	
      
      _dUc[d] = ( _dUc[d] * pn - Base::_uc * factor ) / pn2;
      _dUl[d] = ( _dUl[d] * pn - Base::_ul * factor ) / pn2;
      _dUq[d] = ( _dUq[d] * pn - Base::_uq * factor ) / pn2;
    }
    
    Base::applyPrattNorm();
    return true;
  }
  
}// namespace internal
