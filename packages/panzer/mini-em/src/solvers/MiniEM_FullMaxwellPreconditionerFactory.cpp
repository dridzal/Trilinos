#include "MiniEM_FullMaxwellPreconditionerFactory.hpp"

#include "Teko_BlockLowerTriInverseOp.hpp"
#include "Teko_BlockUpperTriInverseOp.hpp"

#include "Teko_SolveInverseFactory.hpp"

#include "Thyra_DiagonalLinearOpBase.hpp"
#include "Thyra_DefaultProductVectorSpace.hpp"
#include "Thyra_DefaultProductMultiVector.hpp"

#include "Teuchos_as.hpp"
#include "Teuchos_Time.hpp"

#include "Teko_TpetraHelpers.hpp"
#include "Teko_TpetraOperatorWrapper.hpp"

#include "Thyra_TpetraLinearOp.hpp"
#include "MatrixMarket_Tpetra.hpp"
#include "Panzer_NodeType.hpp"
#include "EpetraExt_RowMatrixOut.h"
#include "EpetraExt_BlockMapOut.h"
#include "Epetra_CombineMode.h"
#include "Epetra_Comm.h"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Panzer_LOCPair_GlobalEvaluationData.hpp"
#include "Panzer_LinearObjContainer.hpp"
#include "Panzer_ThyraObjContainer.hpp"

#include "Thyra_DefaultDiagonalLinearOp.hpp"

using Teuchos::RCP;
using Teuchos::rcp_dynamic_cast;

namespace mini_em {

  template<class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node=Tpetra::Map<>::node_type>
  RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> >  get_Tpetra_CrsMatrix(const Thyra::LinearOpBase<double> & op) {
    const RCP<const Thyra::TpetraLinearOp<Scalar,LocalOrdinal,GlobalOrdinal,Node> > tOp = rcp_dynamic_cast<const Thyra::TpetraLinearOp<Scalar,LocalOrdinal,GlobalOrdinal,Node> >(Teuchos::rcpFromRef(op),true);
    RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > crsOp = rcp_dynamic_cast<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> >(tOp->getConstTpetraOperator(),true);
    return crsOp;
  }


  RCP<const Epetra_CrsMatrix> get_Epetra_CrsMatrix(const Thyra::LinearOpBase<double> & op) {
    const RCP<const Thyra::EpetraLinearOp> eOp = rcp_dynamic_cast<const Thyra::EpetraLinearOp>(Teuchos::rcpFromRef(op),true);
    RCP<const Epetra_CrsMatrix> crsOp = rcp_dynamic_cast<const Epetra_CrsMatrix>(eOp->epetra_op(),true);
    return crsOp;
  }


  RCP<const Epetra_CrsMatrix> get_Epetra_CrsMatrix(const Thyra::DiagonalLinearOpBase<double> & op, const Epetra_Comm& comm) {
    RCP<const Epetra_Map> map = Thyra::get_Epetra_Map(*op.range(),Teuchos::rcpFromRef(comm));
    int nodeNumElements = map->NumMyElements();
    RCP<Epetra_CrsMatrix> crsMatrix = Teuchos::rcp(new Epetra_CrsMatrix(Copy,*map,*map,1,true));

    RCP<const Thyra::VectorBase<double> > diag = op.getDiag();
    RTOpPack::SubVectorView<double> view;
    diag->acquireDetachedView(Thyra::Range1D(),&view);

    for (int i = 0; i < nodeNumElements; i++) {
      int err = crsMatrix->InsertMyValues(i, 1, &(view[i]), &i);
      TEUCHOS_ASSERT(err==0);
    }

    diag->releaseDetachedView(&view);
    crsMatrix->FillComplete();
    return crsMatrix;
  }


void writeOut(const std::string & s,const Thyra::LinearOpBase<double> & op)
{
  using Teuchos::RCP;
  using NT = panzer::TpetraNodeType;
  const RCP<const Thyra::TpetraLinearOp<double,int,panzer::GlobalOrdinal,NT> > tOp = rcp_dynamic_cast<const Thyra::TpetraLinearOp<double,int,panzer::GlobalOrdinal,NT> >(Teuchos::rcpFromRef(op));
  const RCP<const Thyra::EpetraLinearOp> eOp = rcp_dynamic_cast<const Thyra::EpetraLinearOp>(Teuchos::rcpFromRef(op));
  if(tOp != Teuchos::null) {
    *Teko::getOutputStream() << "Dumping matrix \'" << s << "\'" << std::endl;
    const RCP<const Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> > crsOp = rcp_dynamic_cast<const Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >(tOp->getConstTpetraOperator(),true);
    Tpetra::MatrixMarket::Writer<Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >::writeMapFile(("rowmap_"+s).c_str(),*(crsOp->getRowMap()));
    Tpetra::MatrixMarket::Writer<Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >::writeMapFile(("colmap_"+s).c_str(),*(crsOp->getColMap()));
    Tpetra::MatrixMarket::Writer<Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >::writeMapFile(("domainmap_"+s).c_str(),*(crsOp->getDomainMap()));
    Tpetra::MatrixMarket::Writer<Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >::writeMapFile(("rangemap_"+s).c_str(),*(crsOp->getRangeMap()));
    Tpetra::MatrixMarket::Writer<Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >::writeSparseFile(s.c_str(),crsOp);
  } else if (eOp != Teuchos::null) {
    *Teko::getOutputStream() << "Dumping matrix \'" << s << "\'" << std::endl;
    const RCP<const Epetra_CrsMatrix> crsOp = rcp_dynamic_cast<const Epetra_CrsMatrix>(eOp->epetra_op(),true);
    EpetraExt::BlockMapToMatrixMarketFile(("rowmap_"+s).c_str(), crsOp->RowMap());
    EpetraExt::BlockMapToMatrixMarketFile(("colmap_"+s).c_str(), crsOp->ColMap());
    EpetraExt::BlockMapToMatrixMarketFile(("domainmap_"+s).c_str(), crsOp->DomainMap());
    EpetraExt::BlockMapToMatrixMarketFile(("rangemap_"+s).c_str(), crsOp->RangeMap());
    EpetraExt::RowMatrixToMatrixMarketFile(s.c_str(), *crsOp);
  } else
    TEUCHOS_ASSERT(false);
}


void describeMatrix(const std::string & s,const Thyra::LinearOpBase<double> & op,Teuchos::RCP<Teuchos::FancyOStream> out)
{
  using Teuchos::RCP;
  using NT = Tpetra::Map<>::node_type;
  if (out!=Teuchos::null) {
    const RCP<const Thyra::TpetraLinearOp<double,int,panzer::GlobalOrdinal,NT> > tOp = rcp_dynamic_cast<const Thyra::TpetraLinearOp<double,int,panzer::GlobalOrdinal,NT> >(Teuchos::rcpFromRef(op));
    const RCP<const Thyra::EpetraLinearOp > eOp = rcp_dynamic_cast<const Thyra::EpetraLinearOp>(Teuchos::rcpFromRef(op));
    if(tOp != Teuchos::null) {
      const RCP<const Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> > crsOp = rcp_dynamic_cast<const Tpetra::CrsMatrix<double,int,panzer::GlobalOrdinal,NT> >(tOp->getConstTpetraOperator(),true);
      *out << "\nDebug: " << s << std::endl;
      crsOp->describe(*out,Teuchos::VERB_MEDIUM);
    } else if (eOp != Teuchos::null) {
      const RCP<const Epetra_CrsMatrix> crsOp = rcp_dynamic_cast<const Epetra_CrsMatrix>(eOp->epetra_op(),true);
      *out << "\nDebug: " << s << std::endl;
      // crsOp->describe(*out,Teuchos::VERB_MEDIUM);
    } else
      TEUCHOS_ASSERT(false);
  }
}


///////////////////////////////////////
// FullMaxwellPreconditionerFactory  //
///////////////////////////////////////

Teko::LinearOp FullMaxwellPreconditionerFactory::buildPreconditionerOperator(Teko::BlockedLinearOp & blo, Teko::BlockPreconditionerState & /* state */) const
{
   typedef double Scalar;
   typedef int LocalOrdinal;
   typedef panzer::GlobalOrdinal GlobalOrdinal;
   typedef panzer::TpetraNodeType Node;

   Teuchos::TimeMonitor tM(*Teuchos::TimeMonitor::getNewTimer(std::string("MaxwellPreconditioner::build")));

   // Output stream for debug information
   Teuchos::RCP<Teuchos::FancyOStream> debug = Teuchos::null;
   if (doDebug)
     debug = Teko::getOutputStream();

   // Check that system is right size
   int rows = Teko::blockRowCount(blo);
   int cols = Teko::blockColCount(blo);
   TEUCHOS_ASSERT(rows==cols);
   TEUCHOS_ASSERT(rows==2);

   // Notation:
   // 0 - Hgrad
   // 1 - Hcurl
   // 2 - HDiv

   // M_k(a) - mass matrix on space k with weight a
   // D_k - derivative from space k to k+1

   // The block matrix is
   //
   // | Q_B  K   |
   // | Kt   Q_E |
   //
   // where
   // Q_B = 1/dt * M_2(1)
   // K   = M_2(1) * D_1
   // Kt  = - D_1^T * M_2(1/mu)
   // Q_E = 1/dt * M_1(eps)

   // S_E = Q_E - Kt * Q_B^-1 * K
   //     = 1/dt * M_1(eps) + dt * D_1^T * M_2(1/mu) * D_1
   //
   // -> curl-curl term scales like dt / mu
   //
   // for refmaxwell: Q_rho = M_0(mu / dt) so that the addon is:
   // M_1(1) * D_0 * M_0(mu / dt)^-1 * D_0^T * M_1(1)

   // Extract the blocks
   Teko::LinearOp Q_B   = Teko::getBlock(0,0,blo);
   Teko::LinearOp K     = Teko::getBlock(0,1,blo);
   Teko::LinearOp Kt    = Teko::getBlock(1,0,blo);
   Teko::LinearOp Q_E   = Teko::getBlock(1,1,blo);

   // nodal mass matrix
   Teko::LinearOp Q_rho = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Mass Matrix AUXILIARY_NODE"));

   // Set up the Schur complement
   Teko::LinearOp S_E;
   {
     Teuchos::TimeMonitor tm(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Schur complement"));
     Teko::LinearOp CurlCurl = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Curl Curl AUXILIARY_EDGE"));
     S_E = Teko::explicitAdd(Q_E, CurlCurl);
   }

   // Check whether we are using Tpetra or Epetra
   Teuchos::RCP<const Thyra::EpetraLinearOp> EOp = rcp_dynamic_cast<const Thyra::EpetraLinearOp>(Q_rho);
   bool useTpetra = (EOp == Teuchos::null);

   /////////////////////////////////////////////////
   // Debug and matrix dumps                      //
   /////////////////////////////////////////////////

   if (dump) {
     writeOut("Q_B.mm",*Q_B);
     writeOut("K.mm",*K);
     writeOut("Kt.mm",*Kt);
     writeOut("Q_E.mm",*Q_E);
     writeOut("Q_rho.mm",*Q_rho);
     writeOut("S_E.mm",*S_E);
   }
   describeMatrix("Q_B",*Q_B,debug);
   describeMatrix("K",*K,debug);
   describeMatrix("Kt",*Kt,debug);
   describeMatrix("Q_E",*Q_E,debug);
   describeMatrix("Q_rho",*Q_rho,debug);
   describeMatrix("S_E",*S_E,debug);



   /////////////////////////////////////////////////
   // Set up inverses for sub-blocks              //
   /////////////////////////////////////////////////

   // Inverse of B mass matrix
   Teko::LinearOp invQ_B;
   {
     Teuchos::TimeMonitor tm(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Inverse Q_B"));
     // Are we building a solver or a preconditioner?
     if (useAsPreconditioner) {
       invQ_B = Teko::buildInverse(*invLib.getInverseFactory("Q_B Preconditioner"),Q_B);
     } else {
       Teko::LinearOp invDiagQ_B = Teko::buildInverse(*invLib.getInverseFactory("Q_B Preconditioner"),Q_B);
       describeMatrix("invDiagQ_B",*invDiagQ_B,debug);
       invQ_B = Teko::buildInverse(*invLib.getInverseFactory("Q_B Solve"),Q_B, invDiagQ_B);
     }
   }

   // Solver for S_E
   Teko::LinearOp invS_E;
   {
     Teuchos::TimeMonitor tm1(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Solver S_E"));
     if(!use_refmaxwell) // Augmentation based solver
       {
         // Get auxiliary operators for gradient and nodal mass matrix
         Teko::LinearOp G;
         if (use_discrete_gradient_) {
           Teko::LinearOp T = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Discrete Gradient"));
           if (dump)
             writeOut("DiscreteGradient.mm",*T);
           describeMatrix("DiscreteGradient",*T,debug);
           G = Teko::explicitMultiply(Q_E,Teko::scale(-1.0,T));
         } else {
           G = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Weak Gradient"));
         }
         if (dump)
           writeOut("WeakGradient.mm",*G);
         describeMatrix("WeakGradient",*G,debug);

         Teko::LinearOp Gt = Teko::explicitTranspose(G);
         // Compute grad-div term
         Teko::LinearOp invDiagQ_rho = Teko::getInvDiagonalOp(Q_rho,Teko::AbsRowSum);
         Teko::LinearOp GGt = Teko::explicitMultiply(G,invDiagQ_rho,Gt);

         // Rescale such that grad-div is large enough to fix curl-curl null-space while not dominating
         double scaling = Teko::infNorm(Q_E)/Teko::infNorm(GGt);

         // Augmented Schur complement and its inverse
         if (doDebug)
           *debug << "Adding up T_E" << std::endl;
         Teko::LinearOp T_E = Teko::explicitAdd(S_E, Thyra::scale(scaling,GGt));
         if (debug != Teuchos::null)
           *debug << "Added up T_E" << std::endl;
         *Teko::getOutputStream() << "Building T_E inverse operator" << std::endl;
         Teko::LinearOp invT_E = Teko::buildInverse(*invLib.getInverseFactory("T_E Solve"),T_E);

         // Correction term
         Teko::LinearOp Z_E = Thyra::add(Q_E, Thyra::scale(scaling,GGt));

         // if (dump)
         //   writeOut("Z_E.mm",*Z_E);
         describeMatrix("Z_E",*Z_E,debug);

         // Mass inverse - diagonal approximation
         Teko::LinearOp invQ_E = Teko::getInvDiagonalOp(Q_E,Teko::AbsRowSum);

         // Approximate inverse of S_E
         invS_E = Teko::multiply(invQ_E,Z_E,invT_E);
       }
     else if (S_E_prec_type_ == "MueLuRefMaxwell-Tpetra" || S_E_prec_type_ == "MueLuRefMaxwell" || S_E_prec_type_ == "ML") {// refMaxwell

       // Teko::LinearOp T = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Discrete Gradient"));
       // Teko::LinearOp KT = Teko::explicitMultiply(K,T);
       // TEUCHOS_ASSERT(Teko::infNorm(KT) < 1.0e-14 * Teko::infNorm(T) * Teko::infNorm(K));

       Teuchos::RCP<Teko::InverseFactory> S_E_prec_factory;
       Teuchos::ParameterList S_E_prec_pl;
       S_E_prec_factory = invLib.getInverseFactory("S_E Preconditioner");
       S_E_prec_pl = *S_E_prec_factory->getParameterList();

       // Get coordinates
       {
#ifndef PANZER_HIDE_DEPRECATED_CODE
         if (useTpetra && ((S_E_prec_type_ == "MueLuRefMaxwell") || (S_E_prec_type_ == "MueLuRefMaxwell-Tpetra"))) {
#else
         if (useTpetra && (S_E_prec_type_ == "MueLuRefMaxwell")) {
#endif
           Teuchos::RCP<Tpetra::MultiVector<Scalar, LocalOrdinal, GlobalOrdinal, Node> > Coordinates = S_E_prec_pl.get<Teuchos::RCP<Tpetra::MultiVector<Scalar, LocalOrdinal, GlobalOrdinal, Node> > >("Coordinates");
           S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("Coordinates",Coordinates);
#ifndef PANZER_HIDE_DEPRECATED_CODE
         } else if (!useTpetra && ((S_E_prec_type_ == "MueLuRefMaxwell") || (S_E_prec_type_ == "MueLuRefMaxwell-Tpetra"))) {
#else
         } else if (!useTpetra && (S_E_prec_type_ == "MueLuRefMaxwell")) {
#endif
           Teuchos::RCP<Epetra_MultiVector> Coordinates = S_E_prec_pl.get<Teuchos::RCP<Epetra_MultiVector> >("Coordinates");
           S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("Coordinates",Coordinates);
         } else if (S_E_prec_type_ == "ML") {
           double* x_coordinates = S_E_prec_pl.sublist("ML Settings").get<double*>("x-coordinates");
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 11list").set("x-coordinates",x_coordinates);
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 22list").set("x-coordinates",x_coordinates);
           double* y_coordinates = S_E_prec_pl.sublist("ML Settings").get<double*>("y-coordinates");
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 11list").set("y-coordinates",y_coordinates);
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 22list").set("y-coordinates",y_coordinates);
           double* z_coordinates = S_E_prec_pl.sublist("ML Settings").get<double*>("z-coordinates");
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 11list").set("z-coordinates",z_coordinates);
           S_E_prec_pl.sublist("ML Settings").sublist("refmaxwell: 22list").set("z-coordinates",z_coordinates);
         } else
           TEUCHOS_ASSERT(false);
       }

       RCP<const Thyra::DiagonalLinearOpBase<Scalar> > invDiagQ_rho;
       {
         Teuchos::TimeMonitor tm(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Lumped diagonal Q_rho"));
         
         // Get inverse of lumped Q_rho
         RCP<Thyra::VectorBase<Scalar> > ones = Thyra::createMember(Q_rho->domain());
         RCP<Thyra::VectorBase<Scalar> > diagonal = Thyra::createMember(Q_rho->range());
         Thyra::assign(ones.ptr(),1.0);
         // compute lumped diagonal
         Thyra::apply(*Q_rho,Thyra::NOTRANS,*ones,diagonal.ptr());
         Thyra::reciprocal(*diagonal,diagonal.ptr());
         invDiagQ_rho = rcp(new Thyra::DefaultDiagonalLinearOp<Scalar>(diagonal));
       }

       {
         Teko::InverseLibrary myInvLib = invLib;
         if (S_E_prec_type_ == "ML") {
           RCP<const Epetra_CrsMatrix> eInvDiagQ_rho = get_Epetra_CrsMatrix(*invDiagQ_rho,get_Epetra_CrsMatrix(*Q_B)->RowMap().Comm());
           S_E_prec_pl.sublist("ML Settings").set("M0inv",eInvDiagQ_rho);

           S_E_prec_pl.set("Type",S_E_prec_type_);
           myInvLib.addInverse("S_E Preconditioner",S_E_prec_pl);
         } else {
           S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("M0inv",invDiagQ_rho);

           S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("Type",S_E_prec_type_);
           myInvLib.addInverse("S_E Preconditioner",S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_));
         }
         S_E_prec_factory = myInvLib.getInverseFactory("S_E Preconditioner");
       }

       // Are we building a solver or a preconditioner?
       {
         Teuchos::TimeMonitor tm(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Build S_E preconditioner"));
         
         if (useAsPreconditioner)
           invS_E = Teko::buildInverse(*S_E_prec_factory,S_E);
         else {
           Teko::LinearOp S_E_prec = Teko::buildInverse(*S_E_prec_factory,S_E);
           invS_E = Teko::buildInverse(*invLib.getInverseFactory("S_E Solve"),S_E,S_E_prec);
         }
       } 
     } else {
       if (useAsPreconditioner)
         invS_E = Teko::buildInverse(*invLib.getInverseFactory("S_E Preconditioner"),S_E);
       else {
         Teko::LinearOp S_E_prec = Teko::buildInverse(*invLib.getInverseFactory("S_E Preconditioner"),S_E);
         invS_E = Teko::buildInverse(*invLib.getInverseFactory("S_E Solve"),S_E,S_E_prec);
       }
     }
   }


   /////////////////////////////////////////////////
   // Build block  inverse matrices               //
   /////////////////////////////////////////////////

   {
     Teuchos::TimeMonitor tm(*Teuchos::TimeMonitor::getNewTimer("MaxwellPreconditioner: Block preconditioner"));

     // Inverse blocks
     std::vector<Teko::LinearOp> diag(2);
     diag[0] = invQ_B;
     diag[1] = invS_E;

     // Upper tri blocks
     Teko::BlockedLinearOp U = Teko::createBlockedOp();
     Teko::beginBlockFill(U,rows,rows);
     Teko::setBlock(0,0,U,Q_B);
     Teko::setBlock(1,1,U,S_E);
     Teko::setBlock(0,1,U,K);
     Teko::endBlockFill(U);

     Teko::LinearOp invU = Teko::createBlockUpperTriInverseOp(U,diag);

     if (!useAsPreconditioner) {
       Teko::BlockedLinearOp invL = Teko::createBlockedOp();
       Teko::LinearOp id_B = Teko::identity(Teko::rangeSpace(Q_B));
       Teko::LinearOp id_E = Teko::identity(Teko::rangeSpace(Q_E));
       Teko::beginBlockFill(invL,rows,rows);
       Teko::setBlock(0,0,invL,id_B);
       Teko::setBlock(1,0,invL,Teko::multiply(Thyra::scale(-1.0, Kt), invQ_B));
       Teko::setBlock(1,1,invL,id_E);
       Teko::endBlockFill(invL);

       return Teko::multiply(invU, Teko::toLinearOp(invL));
     } else
       return invU;
   }
}

//! Initialize from a parameter list
void FullMaxwellPreconditionerFactory::initializeFromParameterList(const Teuchos::ParameterList & pl)
{
   /////////////////////
   // Solver options  //
   // //////////////////

   params = pl;

   use_refmaxwell         = params.get("Use refMaxwell",false);
   use_discrete_gradient_ = params.get("Use discrete gradient",false);
   dump                   = params.get("Dump",false);
   doDebug                = params.get("Debug",false);
   useAsPreconditioner    = params.get("Use as preconditioner",false);

   if(pl.isSublist("S_E Preconditioner") && pl.sublist("S_E Preconditioner").isParameter("Type"))
     S_E_prec_type_ = pl.sublist("S_E Preconditioner").get<std::string>("Type");
   else
     S_E_prec_type_ = "";

   // Output stream for debug information
   Teuchos::RCP<Teuchos::FancyOStream> debug = Teuchos::null;
   if (doDebug)
     debug = Teko::getOutputStream();

   //////////////////////////////////
   // Set up sub-solve factories   //
   //////////////////////////////////

   // New inverse lib to add inverse factories to
   invLib = *getInverseLibrary();

   // Q_B solve
   if (pl.isParameter("Q_B Solve")) {
     Teuchos::ParameterList cg_pl = pl.sublist("Q_B Solve");
     invLib.addInverse("Q_B Solve",cg_pl);
   }

   // Q_B preconditioner
   Teuchos::ParameterList Q_B_prec_pl = pl.sublist("Q_B Preconditioner");
   invLib.addStratPrecond("Q_B Preconditioner",
                          Q_B_prec_pl.get<std::string>("Prec Type"),
                          Q_B_prec_pl.sublist("Prec Types"));

   if (!use_refmaxwell){
     // T_E solve
     Teuchos::ParameterList T_E_pl = pl.sublist("T_E Solve");
     invLib.addInverse("T_E Solve",T_E_pl);

   } else if (S_E_prec_type_ == "MueLuRefMaxwell-Tpetra" || S_E_prec_type_ == "MueLuRefMaxwell" || S_E_prec_type_ == "ML") { // RefMaxwell based solve

     dt = params.get<double>("dt");

     // S_E solve
     Teuchos::ParameterList ml_pl = pl.sublist("S_E Solve");
     invLib.addInverse("S_E Solve",ml_pl);

     // S_E preconditioner
     Teuchos::ParameterList S_E_prec_pl = pl.sublist("S_E Preconditioner");

     // add discrete gradient and edge mass matrix
     Teko::LinearOp Q_E_aux = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Mass Matrix AUXILIARY_EDGE"));
     Teko::LinearOp T = getRequestHandler()->request<Teko::LinearOp>(Teko::RequestMesg("Discrete Gradient"));
     if (S_E_prec_type_ == "ML") {
       RCP<const Epetra_CrsMatrix> eT = get_Epetra_CrsMatrix(*T);
       RCP<const Epetra_CrsMatrix> eQ_E_aux = get_Epetra_CrsMatrix(*Q_E_aux);
       S_E_prec_pl.sublist("ML Settings").set("D0",eT);
       S_E_prec_pl.sublist("ML Settings").set("M1",eQ_E_aux);
       S_E_prec_pl.sublist("ML Settings").set("Ms",eQ_E_aux);
     } else {
       S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("D0",T);
       S_E_prec_pl.sublist("Preconditioner Types").sublist(S_E_prec_type_).set("M1",Q_E_aux);
     }

     if (dump) {
       writeOut("DiscreteGradient.mm",*T);
     }
     describeMatrix("DiscreteGradient",*T,debug);

     invLib.addInverse("S_E Preconditioner",S_E_prec_pl);
   } else {
     // S_E solve
     if (pl.isParameter("S_E Solve")) {
       Teuchos::ParameterList cg_pl = pl.sublist("S_E Solve");
       invLib.addInverse("S_E Solve",cg_pl);
     }

     // S_E preconditioner
     Teuchos::ParameterList S_E_prec_pl = pl.sublist("S_E Preconditioner");
     invLib.addStratPrecond("S_E Preconditioner",
                            S_E_prec_pl.get<std::string>("Prec Type"),
                            S_E_prec_pl.sublist("Prec Types"));
   }


   use_discrete_curl_ = params.get("Use discrete curl",true);
}

} // namespace mini_em
