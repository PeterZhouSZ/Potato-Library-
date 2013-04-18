/* ---------------------------------------------------------------------------
**
** Examples/Grenaille/basic_cpu.h
** <very brief file description>
**
** Author: Nicolas Mellado
** Compile command: 
** $ g++ basic_cpu.h_cpu.cpp -I ../.. -I /path/to/Eigen/
** -------------------------------------------------------------------------*/

#include <cmath>
#include <algorithm>
#include <iostream>

#include "Patate/grenaille.h"
#include "Eigen"

#include <vector>
using namespace std;
using namespace Grenaille;

// Define our working data structure
class MyPoint{
public:
  enum {Dim = 3};
  typedef double Scalar;
  typedef Eigen::Matrix<Scalar, Dim, 1> VectorType;

  inline MyPoint(const VectorType &pos    = VectorType::Zero(), 
		 const VectorType& normal = VectorType::Zero())
    : _pos(pos), _normal(normal) {}
    
  inline const VectorType& pos()    const { return _pos; }  
  inline const VectorType& normal() const { return _normal; }

  inline VectorType& pos()    { return _pos; }  
  inline VectorType& normal() { return _normal; }

  static inline MyPoint Random() { 
    return MyPoint (VectorType::Random(), VectorType::Random());
  };

private:
  VectorType _pos, _normal;
};

typedef MyPoint::Scalar Scalar;
typedef MyPoint::VectorType VectorType;

// Define related structure
typedef DistWeightFunc<MyPoint,SmoothWeightKernel<Scalar> > WeightFunc; 
typedef Basket<MyPoint,WeightFunc,OrientedSphereFit, GLSParam, OrientedSphereScaleSpaceDer, GLSDer, GLSGeomVar> Fit;



int main() {
  // set evaluation point and scale
  Scalar tmax = 10.0;
  VectorType p = VectorType::Random();
  
  // init input data
  int n = 1000;
  vector<MyPoint> vecs (n);

  fill(vecs.begin(), vecs.end(), MyPoint::Random());
  
  VectorType normal; normal << 0.0 , 0.0, 1.0;
  
  // init Fit procedure
  Fit fit;
  fit.setWeightFunc(WeightFunc(tmax));
  fit.init(p);
  
  for(vector<MyPoint>::iterator it = vecs.begin(); it != vecs.end(); it++)
    fit.addNeighbor(*it);
  
  fit.finalize();

  cout << "Pratt normalization" << (fit.applyPrattNorm() ? " is now done." : " has already been applied.") << endl;
  
  cout << "Fitted Sphere: " << endl
       << "\t Tau  : " << fit.tau() << endl
       << "\t Eta  : " << fit.eta().transpose() << endl
       << "\t Kappa: " << fit.kappa() << endl;
    
  cout << "The initial point " << p.transpose() << endl
       << "Is projected at   " << fit.project(p).transpose() << endl;

  Fit::ScalarArray dtau = fit.dkappa_normalized();

  cout << "dkappa: " << dtau[0] 
       << " , "    << dtau[1]
       << " , "    << dtau[2]
       << endl;

  cout << "geomVar: " << fit.geomVar() << endl;
  
  
}
