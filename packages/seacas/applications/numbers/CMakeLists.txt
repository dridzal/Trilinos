INCLUDE(FortranSettings)

TRIBITS_SUBPACKAGE(Numbers)

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

SET(CMAKE_Fortran_FLAGS_RELEASE "-O0")

SET(HEADERS "")
APPEND_GLOB(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.f)
TRIBITS_ADD_EXECUTABLE(
  numbers
  NOEXEPREFIX
  NOEXESUFFIX
  HEADERS ${HEADERS}
  LINKER_LANGUAGE Fortran
  SOURCES ${SOURCES}
  COMM serial mpi
  )
install_executable(numbers)

TRIBITS_SUBPACKAGE_POSTPROCESS()

