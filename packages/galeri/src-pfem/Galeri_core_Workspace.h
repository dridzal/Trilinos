// @HEADER
// ************************************************************************
//
//            Galeri: Finite Element and Matrix Generation Package
//                 Copyright (2006) ETHZ/Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// Questions about Galeri? Contact Marzio Sala (marzio.sala _AT_ gmail.com)
//
// ************************************************************************
// @HEADER

/*! \file Galeri_core_Workspace
 *
 * \author Marzio Sala, ETHZ
 *
 * \date Last modified on Aug-06
 */

#ifndef GALERI_CORE_WORKSPACE_H
#define GALERI_CORE_WORKSPACE_H

#include "Epetra_BlockMap.h"
#include "Epetra_Map.h"
#include "Epetra_MultiVector.h"

#define GALERI_MAX(x,y) (( (x) > (y) ) ? x : y)
#define GALERI_MIN(x,y) (( (x) < (y) ) ? x : y) 
#define GALERI_SGN(x) (((x) < 0.0) ? -1.0 : 1.0) 

namespace Galeri {
namespace core {

/*!
 * \class Workspace
 *
 * \brief Function class containing a few static methods and constants, to be
 * used as workspace tools.
 */
class Workspace 
{
  public:
    //! Default constructor.
    Workspace() {}

    //! Default destructor.
    ~Workspace() {}

    //! Sets the number of dimension (1, 2, or 3).
    static void setNumDimensions(const int numDimensions)
    {
      numDimensions_ = numDimensions;
    }

    //! Returns the number of dimensions used (1, 2, or 3).
    static int getNumDimensions()
    {
      return(numDimensions_);
    }

    /*! \brief Creates a multivector that can hold a component of the specified 
     * multivector.
     *
     * From the input Epetra_MultiVector, defined on an Epetra_BlockMap,
     * defines a new Epetra_Map that can host one component of the
     * multivector, and allocates an Epetra_MultiVector based on it. This is
     * useful in vector problems, when the input vector contains several PDE
     * componets, and one of them has to be extracted.
     */
    static 
    Epetra_MultiVector* createMultiVectorComponent(const Epetra_MultiVector& input)
    {
      const Epetra_Comm& comm = input.Comm();
      const Epetra_BlockMap& inputMap = input.Map();
      const int* myGlobalElements = inputMap.MyGlobalElements();
      const int numMyElements = inputMap.NumMyElements();
      const int indexBase = inputMap.IndexBase();

      Epetra_Map extractMap(-1, numMyElements, myGlobalElements, indexBase, comm);
      Epetra_MultiVector* output = new Epetra_MultiVector(extractMap, input.NumVectors());

      return(output);
    }

    /*! \brief Extracts the component of the specified \c equation from the \c
     * input Epetra_MultiVector, and stores it in \c output.
     */
    static 
    void extractMultiVectorComponent(const Epetra_MultiVector& input,
                                     const int equation,
                                     Epetra_MultiVector& output)
    {
      const Epetra_BlockMap& inputMap = input.Map();
      const int numMyElements = inputMap.NumMyElements();

      for (int i = 0; i < numMyElements; ++i)
      {
        int j = inputMap.FirstPointInElement(i) + equation;
        for (int k = 0; k < input.NumVectors(); ++k)
          output[k][i] = input[k][j];
      }
    }

    //! Input default value for "min".
    static const int MIN;
    //! Input default value for "max".
    static const int MAX;

  private:
    //! Number of dimensions in the computations.
    static int numDimensions_;
}; // class Workspace

} // namespace core
} // namespace Galeri

#endif
