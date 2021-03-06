

#
# A) Package-specific configuration options
#

TRIBITS_CONFIGURE_FILE(${PACKAGE_NAME}_config.hpp)

#
# B) Define the header and source files (and directories)
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.hpp
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# FAD
APPEND_SET(HEADERS
  Phalanx_any.hpp
  Phalanx_DAG_Node.hpp
  Phalanx_DAG_Manager_Def.hpp
  Phalanx_DAG_Manager.hpp
  Phalanx_DataLayout_MDALayout_Def.hpp
  Phalanx_DataLayout_MDALayout.hpp
  Phalanx_DataLayout.hpp
  Phalanx_DimTag.hpp
  Phalanx_DoxygenDocumentation.hpp
  Phalanx_EvaluationContainer_Base_Def.hpp
  Phalanx_EvaluationContainer_Base.hpp
  Phalanx_EvaluationContainer_Def.hpp
  Phalanx_EvaluationContainer.hpp
  Phalanx_EvaluationContainer_TemplateBuilder.hpp
  Phalanx_EvaluationContainer_TemplateManager.hpp
  Phalanx_Evaluator_Base.hpp
  Phalanx_Evaluator_Derived.hpp
  Phalanx_Evaluator_Factory_Def.hpp
  Phalanx_Evaluator_Factory.hpp
  Phalanx_Evaluator_Factory_UFO.hpp
  Phalanx_Evaluator.hpp
  Phalanx_Evaluator_Macros.hpp
  Phalanx_Evaluator_TaskBase.hpp
  Phalanx_Evaluator_TemplateBuilder.hpp
  Phalanx_Evaluator_TemplateManager.hpp
  Phalanx_Evaluator_Utilities.hpp
  Phalanx_Evaluator_WithBaseImpl_Def.hpp
  Phalanx_Evaluator_WithBaseImpl.hpp
  Phalanx_Evaluator_AliasField.hpp
  Phalanx_Evaluator_UnmanagedFieldDummy.hpp
  Phalanx_Exceptions.hpp
  Phalanx_FieldManager_Def.hpp
  Phalanx_FieldManager.hpp
  Phalanx_FieldTag.hpp
  Phalanx_FieldTag_STL_Functors.hpp
  Phalanx_FieldTag_Tag_Def.hpp
  Phalanx_FieldTag_Tag.hpp
  Phalanx_KokkosDeviceTypes.hpp
  Phalanx_KokkosViewFactory.hpp
  Phalanx_KokkosViewFactoryFunctor.hpp
  Phalanx_MDField_Def.hpp
  Phalanx_MDField.hpp
  Phalanx_MDField_DynRank_Def.hpp
  Phalanx_MDField_DynRank.hpp
  Phalanx_MDField_TypeTraits.hpp
  Phalanx_MDFieldToKokkos.hpp
  Phalanx_Print_Utilities.hpp
  Phalanx_TemplateIterator.hpp
  Phalanx_TemplateManager_Def.hpp
  Phalanx_TemplateManager.hpp
  Phalanx_Traits.hpp
  Phalanx_TypeStrings.hpp
  Phalanx_MDField_Utilities.hpp
  Phalanx_MDField_Utilities_Def.hpp
  ) 

APPEND_SET(SOURCES
  Phalanx_DataLayout.cpp
  Phalanx_DimTag.cpp
  Phalanx_Exceptions.cpp
  Phalanx_FieldTag.cpp
  )
#
# C) Define the targets for package's library(s)
#

IF(PHX_KOKKOS_DEVICE_TYPE_CUDA)
  IF( TPL_ENABLE_MPI )
      CUDA_INCLUDE_DIRECTORIES( ${MPI_BASE_DIR}/include )
  ENDIF()

  CUDA_INCLUDE_DIRECTORIES( ${KokkosCore_INCLUDE_DIRS}
                            ${Phalanx_INCLUDE_DIRS})
ENDIF()

TRIBITS_ADD_LIBRARY(
  phalanx
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  DEPLIBS ${Phalanx_CUDA_LIBRARY}
  )

