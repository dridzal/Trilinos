
TRIBITS_ADD_EXECUTABLE_AND_TEST(
  iallreduce
  SOURCES
    iallreduce
    ${TEUCHOS_STD_UNIT_TEST_MAIN}
  NUM_MPI_PROCS 1-4
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  extractMpiCommFromTeuchos
  SOURCES
    extractMpiCommFromTeuchos
    ${TEUCHOS_STD_UNIT_TEST_MAIN}
  NUM_MPI_PROCS 1-4
  COMM mpi
  )

# This test is best done with at least 2 MPI processes.
TRIBITS_ADD_EXECUTABLE_AND_TEST(
  isInterComm
  SOURCES
    isInterComm
    ${TEUCHOS_STD_UNIT_TEST_MAIN}
  NUM_MPI_PROCS 2-4
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  idot
  SOURCES
    idot
    ${TEUCHOS_STD_UNIT_TEST_MAIN}
  NUM_MPI_PROCS 1-4
  COMM serial mpi
  )
