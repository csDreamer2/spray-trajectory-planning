add_library(DataSTEP
    STEPModelTree.cpp
    STEPModelTreeWorker.cpp
)
target_include_directories(DataSTEP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(DataSTEP PUBLIC ${VTK_INCLUDE_DIRS})
target_link_libraries(DataSTEP PUBLIC
    Qt6::Core
    Qt6::Gui
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
    ${VTK_LIBRARIES}
)