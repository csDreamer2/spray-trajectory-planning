# STEP模型树测试程序

# 最小测试程序
set(MINIMAL_STEP_TEST_SOURCES
    minimal_step_test.cpp
)

# 简单测试程序（无GUI）
set(SIMPLE_STEP_TEST_SOURCES
    simple_step_test.cpp
)

# GUI测试程序
set(TEST_STEP_MODEL_TREE_SOURCES
    test_step_model_tree.cpp
)

# 主程序集成测试
set(MAIN_INTEGRATION_TEST_SOURCES
    main_integration_test.cpp
)

# 创建最小测试可执行文件
add_executable(minimal_step_test ${MINIMAL_STEP_TEST_SOURCES})

# 创建简单测试可执行文件
add_executable(simple_step_test ${SIMPLE_STEP_TEST_SOURCES})

# 创建GUI测试可执行文件
add_executable(test_step_model_tree ${TEST_STEP_MODEL_TREE_SOURCES})

# 创建主程序集成测试可执行文件
add_executable(main_integration_test ${MAIN_INTEGRATION_TEST_SOURCES})

# 链接最小测试的库
target_link_libraries(minimal_step_test PRIVATE
    Qt6::Core
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

# 链接简单测试的库
target_link_libraries(simple_step_test PRIVATE
    Qt6::Core
    Data
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

# 链接GUI测试的库
target_link_libraries(test_step_model_tree PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

# 链接主程序集成测试的库
target_link_libraries(main_integration_test PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

# 设置Qt的MOC、UIC、RCC
set_target_properties(minimal_step_test PROPERTIES
    AUTOMOC ON
)

set_target_properties(simple_step_test PROPERTIES
    AUTOMOC ON
)

set_target_properties(test_step_model_tree PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

set_target_properties(main_integration_test PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 设置输出目录
set_target_properties(minimal_step_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

set_target_properties(simple_step_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

set_target_properties(test_step_model_tree PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

set_target_properties(main_integration_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# 包含目录
target_include_directories(minimal_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
)

target_include_directories(simple_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
)

target_include_directories(test_step_model_tree PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

target_include_directories(main_integration_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

# Windows特定设置
if(WIN32)
    # 最小测试保留控制台
    set_target_properties(minimal_step_test PROPERTIES
        WIN32_EXECUTABLE OFF
    )
    
    # 简单测试保留控制台
    set_target_properties(simple_step_test PROPERTIES
        WIN32_EXECUTABLE OFF
    )
    
    # GUI测试程序
    set_target_properties(test_step_model_tree PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
    
    # 主程序集成测试
    set_target_properties(main_integration_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "STEP模型树测试程序配置完成")
message(STATUS "  - minimal_step_test: 最小OpenCASCADE测试")
message(STATUS "  - simple_step_test: 简单控制台测试")
message(STATUS "  - test_step_model_tree: GUI测试程序")
message(STATUS "  - main_integration_test: 主程序集成测试")

# STEP崩溃调试测试
add_executable(step_crash_debug step_crash_debug.cpp)

target_link_libraries(step_crash_debug PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(step_crash_debug PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(step_crash_debug PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

if(WIN32)
    set_target_properties(step_crash_debug PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - step_crash_debug: STEP崩溃调试测试")
# STEP模型树单独测试
add_executable(step_tree_only_test step_tree_only_test.cpp)

target_link_libraries(step_tree_only_test PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(step_tree_only_test PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(step_tree_only_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

if(WIN32)
    set_target_properties(step_tree_only_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - step_tree_only_test: STEP模型树单独测试")
# 安全STEP测试
add_executable(safe_step_test safe_step_test.cpp)

target_link_libraries(safe_step_test PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(safe_step_test PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(safe_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

if(WIN32)
    set_target_properties(safe_step_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - safe_step_test: 安全STEP测试（带保护措施）")
# 最小OpenCASCADE测试
add_executable(minimal_occt_test minimal_occt_test.cpp)

target_link_libraries(minimal_occt_test PRIVATE
    Qt6::Core
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(minimal_occt_test PROPERTIES
    AUTOMOC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

if(WIN32)
    set_target_properties(minimal_occt_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - minimal_occt_test: 最小OpenCASCADE STEP测试")
# 基础STEP测试
add_executable(basic_step_test basic_step_test.cpp)

target_link_libraries(basic_step_test PRIVATE
    Qt6::Core
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(basic_step_test PROPERTIES
    AUTOMOC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

if(WIN32)
    set_target_properties(basic_step_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - basic_step_test: 基础STEP读取测试")
# 最终STEP修复测试
add_executable(final_step_test final_step_test.cpp)

target_link_libraries(final_step_test PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    # STEP CAF Control 需要的库
    TKSTEPBase TKSTEP209 TKSTEPAttr TKXDESTEP TKDCAF
)

set_target_properties(final_step_test PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(final_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

if(WIN32)
    set_target_properties(final_step_test PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "  - final_step_test: 最终STEP修复测试")