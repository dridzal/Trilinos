
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(SOURCES "")

FILE(GLOB SOURCES *.cpp)

SET(LIBRARIES shyluchol)

IF(Kokkos_ENABLE_OpenMP OR Kokkos_ENABLE_Serial)
  TRIBITS_ADD_EXECUTABLE(
    Tacho_ExampleCholSupernodes
    NOEXEPREFIX
    SOURCES Tacho_ExampleCholSupernodes.cpp
    COMM serial mpi
  )
  IF(TPL_ENABLE_MKL)
    TRIBITS_ADD_EXECUTABLE(
      Tacho_ExampleCholPardiso
      NOEXEPREFIX
      SOURCES Tacho_ExampleCholPardiso.cpp
      COMM serial mpi
    )
  ENDIF()
ENDIF()

TRIBITS_COPY_FILES_TO_BINARY_DIR(ShyLUTacho_SimpleSparseTest_File
        SOURCE_FILES test.mtx
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
