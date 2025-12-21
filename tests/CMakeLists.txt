# STEP模型树测试程序 - 清理版本

# 核心测试程序（保留）
message(STATUS "配置STEP模型树测试程序...")

# 1. 安全STEP测试（参考版本 - 正常工作）
add_executable(safe_step_test safe_step_test.cpp)
target_link_libraries(safe_step_test PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Gui Data UI
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)
set_target_properties(safe_step_test PROPERTIES
    AUTOMOC ON AUTOUIC ON AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    WIN32_EXECUTABLE OFF
)
target_include_directories(safe_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/Data ${CMAKE_SOURCE_DIR}/src/UI
)

# 2. STEP树单独测试（独立版本 - 正常工作）
add_executable(step_tree_only_test step_tree_only_test.cpp)
target_link_libraries(step_tree_only_test PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Gui Data UI
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)
set_target_properties(step_tree_only_test PROPERTIES
    AUTOMOC ON AUTOUIC ON AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    WIN32_EXECUTABLE OFF
)
target_include_directories(step_tree_only_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/Data ${CMAKE_SOURCE_DIR}/src/UI
)

# 3. 修复版安全STEP树状界面测试（最终解决方案）
add_executable(safe_tree_gui_fixed safe_tree_gui_fixed.cpp)
target_link_libraries(safe_tree_gui_fixed PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Gui Data UI
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)
set_target_properties(safe_tree_gui_fixed PROPERTIES
    AUTOMOC ON AUTOUIC ON AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    WIN32_EXECUTABLE OFF
)
target_include_directories(safe_tree_gui_fixed PRIVATE
    ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/Data ${CMAKE_SOURCE_DIR}/src/UI
)

message(STATUS "STEP模型树测试程序配置完成:")
message(STATUS "  ✅ safe_step_test - 安全STEP测试（参考版本）")
message(STATUS "  ✅ step_tree_only_test - STEP树单独测试（独立版本）")
message(STATUS "  ✅ safe_tree_gui_fixed - 修复版STEP树状界面测试（最终解决方案）")