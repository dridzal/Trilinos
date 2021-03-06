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


/*! \file test_04.cpp
 *  \brief Test barrier functions and factory
 */


#include "ROL_StdVector.hpp"
#include "ROL_BarrierFunctions.hpp"

#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_ParameterList.hpp"

int main( int argc, char *argv[] ) {

  typedef double RealT;

  typedef std::vector<RealT>         vector;
  typedef ROL::Vector<RealT>         V;
  typedef ROL::StdVector<RealT>      SV;

  typedef typename vector::size_type uint;

  using Teuchos::RCP;  using Teuchos::rcp;

  RealT xmin = -1.0;
  RealT xmax =  2.0;

  uint N = 30;

  RCP<vector> x_rcp = rcp( new vector(N,0.0) );
  RCP<vector> y_rcp = rcp( new vector(N,0.0) );
  RCP<vector> z_rcp = rcp( new vector(N,0.0) );

  RCP<vector> u_rcp = rcp( new vector(N,1.0) );
  RCP<vector> l_rcp = rcp( new vector(N,0.0) );

  SV x(x_rcp);
  SV y(y_rcp);
  SV z(z_rcp);

  SV u(u_rcp);
  SV l(l_rcp);

  RCP<V> x_minus_l = x.clone();
  RCP<V> u_minus_x = x.clone();

  

/*
  Teuchos::ParameterList logList;
  Teuchos::ParameterList quadList;

  logList.sublist("Barrier Function").set("Type","Logarithmic");
  quadList.sublist("Barrier Function").set("Type","Quadratic");

  ROL::Elementwise::BarrierFunctionFactory<RealT> logFactory( logList );
  ROL::Elementwise::BarrierFunctionFactory<RealT> quadFactory( quadList );

  RCP<ROL::Elementwise::BarrierFunction<RealT> > logFunction = logFactory.getBarrierFunction();
  RCP<ROL::Elementwise::BarrierFunction<RealT> > quadFunction = quadFactory.getBarrierFunction();
*/
  
  RCP<ROL::Elementwise::BinaryFunction<RealT> > lesser = rcp( new ROL::Elementwise::Lesser<RealT> );
  RCP<ROL::Elementwise::BinaryFunction<RealT> > greater = rcp( new ROL::Elementwise::Greater<RealT> );
   

  for( uint i=0; i<N; ++i ) {
    RealT t = static_cast<RealT>(i)/(N-1);
    (*x_rcp)[i] = xmin*(1-t) + xmax*t;
  }


  typedef ROL::Elementwise::Axpy<RealT>              Axpy;
  typedef ROL::Elementwise::Scale<RealT>             Scale;
  typedef ROL::Elementwise::Logarithm<RealT>         Logarithm;
  
  



  x_minus_l->set(l);
  x_minus_l->applyBinary(Axpy(-1.0),x);  
  x_minus_l->applyUnary(Logarithm());
  x_minus_l->applyUnary(Scale(-1.0));

  u_minus_x->set(x);
  u_minus_x->applyBinary(Axpy(-1.0),u);
  u_minus_x->applyUnary(Logarithm());
  u_minus_x->applyUnary(Scale(-1.0));


  y.set(x);
  y.applyBinary(*lesser,u);
  y.applyBinary(*greater,l);
  
  RCP<vector> xml = Teuchos::rcp_static_cast<SV>(x_minus_l)->getVector();
  RCP<vector> umx = Teuchos::rcp_static_cast<SV>(u_minus_x)->getVector();

  for(uint i=0; i<N; ++i ) {
    std::cout << std::setw(16) << (*xml)[i] 
              << std::setw(16) << (*umx)[i] << std::endl;
  } 

 


  return 0;
}


