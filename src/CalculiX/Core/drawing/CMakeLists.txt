target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawSnapshotCapture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawSnapshotCapture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCaptureRequest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCaptureRequest.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCaptureInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCaptureInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawCapture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawCapture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawEntityGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawEntityGeometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DrawLabelSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DrawLabelSettings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DrawLabelData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DrawLabelData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawLabels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawLabels.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawingAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawingAdapter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawSavedState.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawSavedState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawTopology.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawScope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawScope.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawMeshGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawMeshGeometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawScene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawScene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawTypes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TopologyAnimation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TopologyAnimation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreDrawData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreDraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDraw.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawBCData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawBCData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawLoadData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawLoadData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawConstraintData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawConstraintData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawOrientationData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDrawOrientationData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDrawInterface.hpp
)

option(CUBITSIM_DRAWING_LABEL_TESTS "Build CalculiX drawing label regression tests" OFF)
if (CUBITSIM_DRAWING_LABEL_TESTS)
  enable_testing()
  add_subdirectory(tests "${CMAKE_CURRENT_BINARY_DIR}/label-tests")
endif()
