# 测试程序 CMakeLists.txt
# 这些是独立的测试和调试程序，不包含在主程序构建中

cmake_minimum_required(VERSION 3.16)

# 如果作为独立项目构建
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(SpraySystemTestPrograms)
    
    # 包含主项目的配置
    include(../../config/paths.cmake)
    
    # 设置C++标准
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # 查找依赖
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
    find_package(VTK REQUIRED)
    find_package(OpenCASCADE REQUIRED)
endif()

# 1. OpenCASCADE基础测试程序
add_executable(TestOpenCASCADE test_opencascade.cpp)
target_link_libraries(TestOpenCASCADE 
    ${OpenCASCADE_LIBRARIES}
)
target_include_directories(TestOpenCASCADE PRIVATE 
    ${OpenCASCADE_INCLUDE_DIR}
)

# 2. 异步STEP加载测试程序
add_executable(TestAsyncSTEP test_async_step.cpp)
target_link_libraries(TestAsyncSTEP 
    Qt6::Core
    Qt6::Widgets
    ${OpenCASCADE_LIBRARIES}
)
target_include_directories(TestAsyncSTEP PRIVATE 
    ${OpenCASCADE_INCLUDE_DIR}
)

# 3. VTK PLY测试程序
add_executable(TestVTKPLY vtk_ply_test.cpp)
target_link_libraries(TestVTKPLY 
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
)
target_include_directories(TestVTKPLY PRIVATE 
    ${VTK_INCLUDE_DIRS}
)

# 4. VTK主测试程序（需要UI组件）
if(TARGET UI)  # 如果UI库存在
    add_executable(TestVTKMain vtk_test_main.cpp)
    target_link_libraries(TestVTKMain 
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGLWidgets
        UI  # 链接UI库
        ${VTK_LIBRARIES}
    )
    target_include_directories(TestVTKMain PRIVATE 
        ${VTK_INCLUDE_DIRS}
        ../../src  # 访问UI组件
    )
endif()

# 5. 调试异步主程序（需要UI组件）
if(TARGET UI)  # 如果UI库存在
    add_executable(DebugAsyncMain debug_async_main.cpp)
    target_link_libraries(DebugAsyncMain 
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGLWidgets
        UI  # 链接UI库
        ${VTK_LIBRARIES}
        ${OpenCASCADE_LIBRARIES}
    )
    target_include_directories(DebugAsyncMain PRIVATE 
        ${VTK_INCLUDE_DIRS}
        ${OpenCASCADE_INCLUDE_DIR}
        ../../src  # 访问UI组件
    )
endif()

# 设置输出目录
set_target_properties(
    TestOpenCASCADE TestAsyncSTEP TestVTKPLY
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
)

if(TARGET TestVTKMain)
    set_target_properties(TestVTKMain PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
    )
endif()

if(TARGET DebugAsyncMain)
    set_target_properties(DebugAsyncMain PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
    )
endif()

# 复制Qt DLL到测试程序目录（Windows）
if(WIN32)
    add_custom_command(TARGET TestAsyncSTEP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE_DIR:TestAsyncSTEP>
    )
    
    add_custom_command(TARGET TestAsyncSTEP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE_DIR:TestAsyncSTEP>
    )
endif()

message(STATUS "✅ 测试程序配置完成")
message(STATUS "   - TestOpenCASCADE: OpenCASCADE基础功能测试")
message(STATUS "   - TestAsyncSTEP: 异步STEP加载测试")
message(STATUS "   - TestVTKPLY: VTK点云加载测试")
if(TARGET TestVTKMain)
    message(STATUS "   - TestVTKMain: VTK主测试程序")
endif()
if(TARGET DebugAsyncMain)
    message(STATUS "   - DebugAsyncMain: 调试异步主程序")
endif()