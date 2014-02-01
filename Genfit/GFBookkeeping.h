/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef GFBOOKKEEPING_H
#define GFBOOKKEEPING_H

#include"TObject.h"
#include"TMatrixT.h"
#include<stdexcept> // std::logic_error
#include<vector>
#include<cassert>
#include<iostream>
#include<utility>
#include<map>
#include"GFDetPlane.h"

namespace genf {

class GFBookkeeping : public TObject {
 private:

  //the string keys will in general be different, so this cant
  //be unified to one container
  std::map<std::string, TMatrixT<Double_t>* > fMatrices;
  std::map<std::string, GFDetPlane* > fPlanes;
  /* this is a work-around: we want to save doubles, but ROOT has problems
   * with TObjects that contain map<string,double*>. We take a 1x1 matrix
   * as a work-around to hold the double internally */
  std::map<std::string, TMatrixT<Double_t>* > fNumbers;
  std::vector< unsigned int > fFailedHits;
  int fNhits;

 public:
  void reset();
  void setNhits(int n){fNhits=n; reset();}

  void bookMatrices(std::string key);
  void bookGFDetPlanes(std::string key);
  void bookNumbers(std::string key,double val=0.);

  void setMatrix(std::string key,unsigned int index,const TMatrixT<Double_t>& mat);
  void setDetPlane(std::string key,unsigned int index,const GFDetPlane& pl);
  void setNumber(std::string key,unsigned int index, const double& num);

  bool getMatrix(std::string key, unsigned int index, TMatrixT<Double_t>& mat) ;
  bool getDetPlane(std::string key, unsigned int index, GFDetPlane& pl)  ;
  bool getNumber(std::string key, unsigned int index, double& num) ;

  std::vector< std::string > getMatrixKeys() ;
  std::vector< std::string > getGFDetPlaneKeys() ;
  std::vector< std::string > getNumberKeys() ;

  void addFailedHit(unsigned int);
  unsigned int hitFailed(unsigned int);
  unsigned int getNumFailed();

  GFBookkeeping(){fNhits=-1;}
  GFBookkeeping(const GFBookkeeping&);
  virtual ~GFBookkeeping(){clearAll();}

  void Streamer(TBuffer&); // Added this, cuz compiler complains elsewise. EC, 28-Dec-2010.
  void clearAll();
  void clearFailedHits();

  void Print() ;

 private:
  //protect from call of net yet defined assignement operator
  GFBookkeeping& operator=(const GFBookkeeping& /* rhs */){return *this;}
  
  virtual void Print(Option_t*) const
    { throw std::logic_error(std::string(__func__) + "::Print(Option_t*) not available"); }

  // public:
  //ClassDef(GFBookkeeping,2)

};

} // namespace genf
#endif
