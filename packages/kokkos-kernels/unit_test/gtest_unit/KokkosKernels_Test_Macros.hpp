/*
//@HEADER
// ************************************************************************
//
//               KokkosKernels 0.9: Linear Algebra and Graph Kernels
//                 Copyright 2017 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// Questions? Contact Siva Rajamanickam (srajama@sandia.gov)
//
// ************************************************************************
//@HEADER
*/


#include <gtest/gtest.h>

namespace KokkosKernels{

template <typename T>
struct TypeNameTraits
{
    static const char* name()
    {
        return typeid(T).name();
    }
};
}


//MNDEVEC: CODES ARE COPY PASTED FROM TEUCHOS

 #define KK_TEST_ASSERT( v1 ) \
    EXPECT_TRUE( v1 );

/** \brief Test that a chunk of code does not throw any exceptions.
 *
 * This macro is not complicated so take a look for yourself!
 *
 * \ingroup teuchos_testing_grp
 */
#define KK_TEST_THROW( code, ExceptType, out, success  ) \
  { \
    std::ostream& l_out = (out); \
    try { \
      l_out << "Test that code {"#code";} throws " \
            << KokkosKernels::TypeNameTraits<ExceptType>::name () << ": "; \
      code; \
      (success) = false; \
      l_out << "failed (code did not throw an exception at all)\n"; \
    } \
    catch (const ExceptType& except) { \
      l_out << "passed\n";                                        \
      l_out << "\nException message for expected exception:\n\n";   \
      { \
        l_out << except.what () << "\n\n"; \
      } \
    } \
    catch (std::exception& except) { \
      l_out << "The code was supposed to throw an exception of type "   \
            << KokkosKernels::TypeNameTraits<ExceptType>::name () << ", but " \
            << "instead threw an exception of type " \
            << typeid (except).name () << ", which is a subclass of " \
            << "std::exception.  The exception's message is:\n\n"; \
      { \
        l_out << except.what () << "\n\n"; \
      } \
      l_out << "failed\n"; \
    } \
    catch (...) { \
      l_out << "The code was supposed to throw an exception of type "   \
            << KokkosKernels::TypeNameTraits<ExceptType>::name () << ", but " \
            << "instead threw an exception of some unknown type, which is " \
            << "not a subclass of std::exception.  This means we cannot " \
            << "show you the exception's message, if it even has one.\n\n"; \
      l_out << "failed\n"; \
    } \
  }

#define KK_TEST_NOTHROW( code, out, success  ) \
  { \
    std::ostream& l_out = (out); \
    try { \
      l_out << "Test that code {"#code";} does not throw : "; \
      code; \
      l_out << "passed\n"; \
    } \
    catch (std::exception& except) { \
      (success) = false; \
      l_out << "The code was not supposed to throw an exception, but " \
            << "instead threw an exception of type " \
            << typeid (except).name () << ", which is a subclass of " \
            << "std::exception.  The exception's message is:\n\n"; \
      { \
        l_out << except.what () << "\n\n"; \
      } \
      l_out << "failed\n"; \
    } \
    catch (...) { \
      (success) = false; \
      l_out << "The code was not supposed to throw an exception, but " \
            << "instead threw an exception of some unknown type, which is " \
            << "not a subclass of std::exception.  This means we cannot " \
            << "show you the exception's message, if it even has one.\n\n"; \
      l_out << "failed\n"; \
    } \
  }


#define KK_TEST_FOR_EXCEPTION(throw_exception_test, Exception, msg) \
{ \
  const bool throw_exception = (throw_exception_test); \
  if(throw_exception) { \
    std::ostringstream omsg; \
    omsg \
      << __FILE__ << ":" << __LINE__ << ":\n\n" \
      << "Throw test that evaluated to true: "#throw_exception_test \
      << "\n\n" \
      << msg; \
    const std::string &omsgstr = omsg.str(); \
    throw Exception(omsgstr); \
  } \
}

