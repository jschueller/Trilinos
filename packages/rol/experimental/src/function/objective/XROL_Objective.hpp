
// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#pragma once

#include "XROL.hpp"


namespace XROL {


/** \file  XROL_Objective.hpp
    \brief Defines the interfaces for objective functions 
 */

// Visitor type object that allows access to functionality not found
// in the base class
template<class PV, class  DV> struct Objective_ExtendedInterface;

// Do-nothing general type
struct ObjectiveParameters{};




/** \class XROL::Objective
    \brief Basic abstract objective function with default implementations
           based on finite differences 
*/
template<class XPrim, class XDual=XPrim> 
class Objective {

private:

  // Class that implements nonvirtual and default virtual functions
  class Impl;

  std::unique_ptr<Impl> pObjImpl_; //!< Default finite difference
                                                //!< implementations

  std::unique_ptr<ObjectiveParameters> param_; //!< Objective-specific parameters


protected:

  const decltype(auto) getParameters( void ) const {
    return std::move(param_);
  }

public: 

  using Scalar = element_t<XPrim>;
  using Real   = magnitude_t<XPrim>;

  // Default no parameter case 
  Objective() : param_(std::make_unique<ObjectiveParameters>) {
    
  }

  Objective( std::unique_ptr<ObjectiveParameters> param ) 
    : param_(std::move(param)) {}

  virtual ~Objective() {}


  virtual void setParameters( std::unique_ptr<ObjectiveParameters> param ) {
    param_ = std::move(param);
  }


  virtual void access( Objective_ExtendedInterface<XPrim,XDual>& objEI ) {
    ignore(objEI);
  };

  virtual void update( const XPrim& x ) { ignore(x); }

  virtual magnitude_t<XPrim> value( const XPrim& x, Real& tol ) = 0;
   
  virtual void gradient( XDual& g, const XPrim& x, element_t<XPrim>& tol ) { 
    pObjImpl_->gradient(*this,g,x,tol);
  }

  virtual magnitude_t<XPrim> dirDeriv( const XPrim& x, const XPrim& d, element_t<XPrim>& tol  ) { 
    return pObjImpl_->dirDeriv(*this,x,d);
  }

  virtual void hessVec( XDual& hv, const XPrim& v, const XPrim& x, element_t<XPrim>& tol ) { 
    pObjImpl_->hessVec(*this,hv,v,x,tol); 
  } 

  virtual void invHessVec( XPrim& hv, const XDual& v, const XPrim& x, element_t<XPrim>& tol ) { 
    ignore(hv,v,x);
  }

  virtual void precond( XPrim& Pv, const XDual& v, const XPrim& x, element_t<XPrim>& tol ) { 
    ignore(Pv,v,x); 
  }

  auto checkGradient( const XPrim& x, const XDual &g, const XPrim& d, std::ostream &os,
                              Teuchos::ParameterList &parlist ) {    
    return pObjImpl_->checkGradient(*this,x,g,d,os,parlist); 
  }


  auto checkHessVec( const XPrim& x, const XDual& hv, const XPrim&v, std::ostream &os,
                             Teuchos::ParameterList &parlist ) {
    return pObjImpl_->checkHessVec(*this,x,hv,v,os,parlist);
  }
  
  auto checkHessSym( const XPrim& x, const XDual& hv, const XPrim& v, const XDual& w, 
                             std::ostream & os, Teuchos::ParameterList& parlist ) {
    return pObjImpl_->checkHessSym(*this,x,hv,v,w,os,parlist);
  }             

};


} // namespace XROL

