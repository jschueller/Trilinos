// @HEADER
//
// ***********************************************************************
//
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//                  Copyright 2012 Sandia Corporation
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
// Questions? Contact Karen Devine      (kddevin@sandia.gov)
//                    Erik Boman        (egboman@sandia.gov)
//                    Siva Rajamanickam (srajama@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

/*! \file Zoltan2_MappingProblem.hpp
    \brief Defines the MappingProblem class.
*/

#ifndef _ZOLTAN2_MAPPINGPROBLEM_HPP_
#define _ZOLTAN2_MAPPINGPROBLEM_HPP_

#include <Zoltan2_Standards.hpp>

#include <Zoltan2_Problem.hpp>
#include <Zoltan2_MappingAlgorithm.hpp>
#include <Zoltan2_MappingSolution.hpp>
#include <string>
#include <bitset>

namespace Zoltan2{

////////////////////////////////////////////////////////////////////////

/*! \brief MappingProblem enables mapping of a partition (either computed or input) to MPI ranks.
 *
 *  The MappingProblem is the core of the Zoltan2 mappin API.
 *  Based on the the user's input and parameters, the MappingProblem
 *  sets up a computational Model, and a Solution object.  When the user
 *  calls the solve() method, the MappingProblem runs the algorithm,
 *  after which the Solution object may be obtained by the user.
 *
 *  The template parameter is the InputAdapter containing the data that
 *  is to be partitioned.
 */

template<typename Adapter>
class MappingProblem : public Problem<Adapter>
{
public:

  typedef typename Adapter::scalar_t scalar_t;
  typedef typename Adapter::gno_t gno_t;
  typedef typename Adapter::lno_t lno_t;
  typedef typename Adapter::user_t user_t;
  typedef typename Adapter::part_t part_t;
  typedef typename Adapter::base_adapter_t base_adapter_t;

  typedef typename PartitioningSolution<Adapter> partsoln_t;
  typedef typename MachineRepresentation<scalar_t, part_t> machine_t;

  /*! \brief Destructor
   */
  virtual ~MappingProblem() {};

#ifdef HAVE_ZOLTAN2_MPI
  /*! \brief Constructor that takes an MPI communicator
   */
  MappingProblem(Adapter *A_, Teuchos::ParameterList *p_, 
                 MPI_Comm comm_,
                 partsoln_t *partSoln_ = NULL, machine_t *machine_ = NULL)
    Problem<Adapter>(A_, p_, comm_) 
  {
    HELLO;
    createMappingProblem(partSoln_, machine_);
  };
#endif

  /*! \brief Constructor that takes an MPI communicator
   */
  MappingProblem(Adapter *A_, Teuchos::ParameterList *p_,
                 Teuchos::Comm<int> comm_,
                 partsoln_t *partSoln_ = NULL, machine_t *machine_ = NULL)
    Problem<Adapter>(A_, p_, comm_) 
  {
    HELLO;
    createMappingProblem(partSoln_, machine_);
  };

  /*! \brief Constructor that uses a default communicator
   */
  MappingProblem(Adapter *A_, Teuchos::ParameterList *p_,
                 partsoln_t *partSoln_ = NULL, machine_t *machine_ = NULL)
    Problem<Adapter>(A_, p_) 
  {
    HELLO;
    createMappingProblem(partSoln_, machine_);
  };

  //!  \brief Direct the problem to create a solution.
  //
  //    \param updateInputData   If true this indicates that either
  //          this is the first attempt at solution, or that we
  //          are computing a new solution and the input data has
  //          changed since the previous solution was computed.
  //          If false, this indicates that we are computing a
  //          new solution using the same input data was used for
  //          the previous solution, even though the parameters
  //          may have been changed.
  //
  //  For the sake of performance, we ask the caller to set \c updateInputData
  //  to false if he/she is computing a new solution using the same input data,
  //  but different problem parameters, than that which was used to compute
  //  the most recent solution.
  
  void solve(bool updateInputData=true); 

  //!  \brief Get the solution to the problem.
  //
  //   \return  a reference to the solution to the most recent solve().

  MappingSolution<Adapter> *getSolution() {
    // Get the raw ptr from the rcp
    return soln.getRawPtr();
  };

private:
  void createMappingProblem(partsoln_t *partSoln_, machine_t *machine_);

  Teuchos::RCP<MappingSolution<Adapter> > soln;

  Teuchos::RCP<partsoln_t> partSoln;
  Teuchos::RCP<machine_t> machine;
};


////////////////////////////////////////////////////////////////////////
template <typename Adapter>
void MappingProblem<Adapter>::solve(bool newData)
{
  HELLO;

  // Create a mapping solution.
  try
  {
    this->soln = rcp(new MappingSolution<Adapter>());
  }
  Z2_FORWARD_EXCEPTIONS;

  // Determine which algorithm to use based on defaults and parameters.
  std::string algName(defString);
  pe = pl.getEntryPtr("mapping_algorithm");
  if (pe) algName = pe->getValue<std::string>(&algName);

  try {
    if (algName == "geometric") {
      CoordinateTaskMapper<> alg(this->comm_, );
      alg.map(this->soln);
    }
  }
  Z2_FORWARD_EXCEPTIONS;
}

////////////////////////////////////////////////////////////////////////
//! createMappingProblem 
//  Method with common functionality for creating a MappingProblem.
//  Individual constructors do appropriate conversions of input, etc.
//  This method does everything that all constructors must do.

template <typename Adapter>
void MappingProblem<Adapter>::createMappingProblem(
  partsoln_t *partSoln_,
  machine_t *machine_)
{
  HELLO;
  if (partSoln_) partSoln = Teuchos::rcp(partSoln_, false);

  // Save pointer to user's machine.  If no machine provided, create one.
  if (machine_) 
    machine = Teuchos::rcp(machine_, false);
  else 
    try {
      machine = Teuchos::rcp(new machine_t(this->comm_));
    }
    Z2_FORWARD_EXCEPTIONS;
}
} //namespace Zoltan2

#endif

#ifdef KDDKDD
Case 1
MappingProblem(
  InputAdapter
  partitioningSolution
  MachineRepresentation=NULL
// KDD Don't know how to properly template MachineRepresentation.  Proper types
// KDD probably depend on how it is to be used.  I imagine MJ needs 
// KDD pcoord_t to be scalar_t, right?  But how does user know that at the
// KDD time he calls this constructor?
)
{
  // Create MachineRepresentation if not provided
  // User would have called partitioning problem and provides a solution
  // Mapping vertices are the parts from the partitioning solution
  // Create MappingSolution that can return getRankForPart(part)
}


Case 2
MappingProblem(
  InputAdapter
  MachineRepresentation=NULL
)
{
  // Create MachineRepresentation if not provided
  // Compute mapping vertices based on InputAdapter's partition
  // Assuming input adapter's partition should be used.
  // KDD would use with Exodus/Nemesis input files or PamGen meshes

}


Case 3
MappingProblem(
  InputAdapter
  MachineRepresentation=NULL
)
{
  // Create MachineRepresentation if not provided
  // Call a partitioning algorithm to get mapping vertices that are the parts
  // Mapping vertices are computed from this internal partitioning solution
  // Maybe relevant models can be shared.
  // Similar to what's in PartitioningProblem now and to what LibTopoMap does

}


Case 4
MappingProblem(
  InputAdapter
  MachineRepresentation=NULL
)
{
  // Create MachineRepresentation if not provided
  // Call mapping with mapping vertices == IDs from the input adapter.
  // Similar in spirit to previous case, but runs more slowly since current
  // task mapping is done in serial.
  // Mehmet has done experiments with Scotch, comparing case 3 with 4.
  // Case 3 is faster; case 4 has higher quality.


}


In general, the input Adapters' applyPartitioningSolution method should take an 
optional MappingSolution.

Should MappingSolution provide a re-numbered communicator reflecting the new mapping?

#endif