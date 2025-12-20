cmake_minimum_required(VERSION 3.16)

project(SprayTrajectoryPlanning VERSION 1.0.0 LANGUAGES CXX)

# 检查并使用项目内的vcpkg
if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    message(STATUS "使用项目内的vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
elseif(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    message(STATUS "使用环境变量的vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC特定设置
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
    add_compile_options(/permissive-)
    add_compile_options(/utf-8)
endif()

# 🎯 加载路径配置文件
include(${CMAKE_SOURCE_DIR}/config/paths.cmake)

# 设置Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql OpenGL OpenGLWidgets)

# 确保Qt库能被找到
if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found!")
endif()

# 先找VTK
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    RenderingCore
    RenderingOpenGL2
    InteractionStyle
    GUISupportQt
    IOGeometry
    IOPLY
    FiltersCore
    FiltersGeometry
    RenderingAnnotation
)

if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found! Please check VTK_DIR: ${VTK_DIR}")
endif()

message(STATUS "✅ VTK found: ${VTK_VERSION}")
message(STATUS "VTK_DIR: ${VTK_DIR}")

# 设置PCL（禁用PCL查找VTK）
set(PCL_ROOT "F:/PCL 1.13.0")
# 🔧 关键：阻止PCL覆盖我们的VTK设置
set(PCL_FIND_QUIETLY ON)
find_package(PCL 1.13 REQUIRED COMPONENTS common io filters features surface segmentation)

# 手动设置PCL包含目录，排除VTK相关
set(PCL_INCLUDE_DIRS_FILTERED)
foreach(dir ${PCL_INCLUDE_DIRS})
    if(NOT dir MATCHES "VTK" AND NOT dir MATCHES "vtk")
        list(APPEND PCL_INCLUDE_DIRS_FILTERED ${dir})
    endif()
endforeach()

include_directories(${PCL_INCLUDE_DIRS_FILTERED})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# 设置自动MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 添加子目录
add_subdirectory(src/UI)
add_subdirectory(src/Core)
add_subdirectory(src/Simulation)
add_subdirectory(src/Robot)
add_subdirectory(src/Data)

# 主可执行文件
add_executable(SprayTrajectoryPlanning
    src/main.cpp
)

# 测试程序已移动到 tests/programs/ 目录
# 如需编译测试程序，请参考 tests/programs/README.md

# 链接Qt库、VTK库和PCL库
target_link_libraries(SprayTrajectoryPlanning
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${VTK_LIBRARIES}
    ${PCL_LIBRARIES}
    # OpenCASCADE核心库（Debug版本）
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKTopAlgo TKPrim
    TKSTEP TKIGES TKMesh TKXSBase TKXCAF TKLCAF TKV3d
    UI
    Core
    Simulation
    Robot
    Data
)

# 测试程序配置已移动到 tests/programs/CMakeLists.txt

# 设置输出目录
set_target_properties(SprayTrajectoryPlanning PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows特定设置
if(WIN32)
    set_target_properties(SprayTrajectoryPlanning PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 添加测试（可选）
option(BUILD_TESTS "Build test programs" OFF)
if(BUILD_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

# 安装规则
install(TARGETS SprayTrajectoryPlanning
    RUNTIME DESTINATION bin
)