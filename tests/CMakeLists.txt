# STEP模型树测试程序

# 简单测试程序（无GUI）
set(SIMPLE_STEP_TEST_SOURCES
    simple_step_test.cpp
)

# GUI测试程序
set(TEST_STEP_MODEL_TREE_SOURCES
    test_step_model_tree.cpp
)

# 创建简单测试可执行文件
add_executable(simple_step_test ${SIMPLE_STEP_TEST_SOURCES})

# 创建GUI测试可执行文件
add_executable(test_step_model_tree ${TEST_STEP_MODEL_TREE_SOURCES})

# 链接简单测试的库
target_link_libraries(simple_step_test PRIVATE
    Qt6::Core
    Data
    ${OpenCASCADE_LIBRARIES}
)

# 链接GUI测试的库
target_link_libraries(test_step_model_tree PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Data
    UI
    ${OpenCASCADE_LIBRARIES}
)

# 设置Qt的MOC、UIC、RCC
set_target_properties(simple_step_test PROPERTIES
    AUTOMOC ON
)

set_target_properties(test_step_model_tree PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 设置输出目录
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

# 包含目录
target_include_directories(simple_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
)

target_include_directories(test_step_model_tree PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Data
    ${CMAKE_SOURCE_DIR}/src/UI
)

# Windows特定设置
if(WIN32)
    # 简单测试保留控制台
    set_target_properties(simple_step_test PROPERTIES
        WIN32_EXECUTABLE OFF
    )
    
    # GUI测试程序
    set_target_properties(test_step_model_tree PROPERTIES
        WIN32_EXECUTABLE OFF  # 保留控制台用于调试
    )
endif()

message(STATUS "STEP模型树测试程序配置完成")
message(STATUS "  - simple_step_test: 简单控制台测试")
message(STATUS "  - test_step_model_tree: GUI测试程序")