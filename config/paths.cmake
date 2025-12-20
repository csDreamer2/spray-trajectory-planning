# =============================================================================
# 喷涂轨迹规划系统 - 路径配置文件
# =============================================================================
# 
# 作者: 王睿 (浙江大学)
# 版本: v1.0
# 日期: 2025-12-20
#
# 此文件包含所有第三方库的绝对路径配置
# 新开发者需要根据自己的环境修改这些路径
#
# 修改说明：
# 1. 将所有路径中的 "K:/Tools" 替换为你的工具安装目录
# 2. 确保所有路径都存在且正确
# 3. 保持路径格式一致（使用正斜杠 /）
#
# =============================================================================

# -----------------------------------------------------------------------------
# 🎯 主要工具目录配置
# -----------------------------------------------------------------------------
# 修改这个基础路径为你的工具安装目录
set(TOOLS_BASE_DIR "K:/Tools" CACHE PATH "工具基础安装目录")

# -----------------------------------------------------------------------------
# 🔧 VTK 配置 (版本 9.2)
# -----------------------------------------------------------------------------
# VTK 编译输出目录
set(VTK_BUILD_DIR "${TOOLS_BASE_DIR}/vtkQT/build" CACHE PATH "VTK编译输出目录")
set(VTK_CMAKE_DIR "${VTK_BUILD_DIR}/lib/cmake/vtk-9.2" CACHE PATH "VTK CMake配置目录")

# VTK 路径验证
if(NOT EXISTS "${VTK_BUILD_DIR}")
    message(FATAL_ERROR "❌ VTK目录不存在: ${VTK_BUILD_DIR}")
endif()

message(STATUS "✅ VTK配置:")
message(STATUS "   构建目录: ${VTK_BUILD_DIR}")
message(STATUS "   CMake目录: ${VTK_CMAKE_DIR}")

# -----------------------------------------------------------------------------
# 🔧 OpenCASCADE 配置 (版本 7.8)
# -----------------------------------------------------------------------------
# OpenCASCADE 安装目录
set(OPENCASCADE_INSTALL_DIR "${TOOLS_BASE_DIR}/OpenCasCade/install" CACHE PATH "OpenCASCADE安装目录")
set(OPENCASCADE_CMAKE_DIR "${OPENCASCADE_INSTALL_DIR}/cmake" CACHE PATH "OpenCASCADE CMake配置目录")
set(OPENCASCADE_INCLUDE_DIR "${OPENCASCADE_INSTALL_DIR}/inc" CACHE PATH "OpenCASCADE头文件目录")
set(OPENCASCADE_LIB_DIR "${OPENCASCADE_INSTALL_DIR}/win64/vc14/libd" CACHE PATH "OpenCASCADE库目录")

# OpenCASCADE 路径验证
if(NOT EXISTS "${OPENCASCADE_INSTALL_DIR}")
    message(FATAL_ERROR "❌ OpenCASCADE目录不存在: ${OPENCASCADE_INSTALL_DIR}")
endif()

if(NOT EXISTS "${OPENCASCADE_CMAKE_DIR}/OpenCASCADEConfig.cmake")
    message(FATAL_ERROR "❌ OpenCASCADE配置文件不存在: ${OPENCASCADE_CMAKE_DIR}/OpenCASCADEConfig.cmake")
endif()

message(STATUS "✅ OpenCASCADE配置:")
message(STATUS "   安装目录: ${OPENCASCADE_INSTALL_DIR}")
message(STATUS "   头文件目录: ${OPENCASCADE_INCLUDE_DIR}")
message(STATUS "   库目录: ${OPENCASCADE_LIB_DIR}")
message(STATUS "   CMake目录: ${OPENCASCADE_CMAKE_DIR}")

# -----------------------------------------------------------------------------
# 🔧 Qt 配置 (自动检测)
# -----------------------------------------------------------------------------
# Qt通常通过环境变量或CMake自动检测，但可以在这里指定特定版本
# set(Qt6_DIR "C:/Qt/6.10.0/msvc2022_64/lib/cmake/Qt6" CACHE PATH "Qt6 CMake目录")

message(STATUS "✅ Qt配置: 使用系统检测的Qt版本")

# -----------------------------------------------------------------------------
# 🔧 vcpkg 配置 (如果使用)
# -----------------------------------------------------------------------------
# 如果使用vcpkg管理依赖，在这里配置vcpkg路径
# set(VCPKG_ROOT "C:/vcpkg" CACHE PATH "vcpkg根目录")
# set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE PATH "vcpkg工具链文件")

# 检查是否正在使用vcpkg
if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    message(STATUS "✅ 使用项目内的vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# -----------------------------------------------------------------------------
# 🔧 其他工具配置
# -----------------------------------------------------------------------------
# 如果需要其他工具，在这里添加配置

# Unity (如果使用Unity集成)
# set(UNITY_INSTALL_DIR "C:/Program Files/Unity/Hub/Editor/2022.3.0f1" CACHE PATH "Unity安装目录")

# Visual Studio (如果需要特定版本)
# set(CMAKE_GENERATOR_TOOLSET "v143" CACHE STRING "Visual Studio工具集版本")

# -----------------------------------------------------------------------------
# 🔧 应用CMAKE前缀路径
# -----------------------------------------------------------------------------
# 将配置的路径添加到CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH 
    "${VTK_BUILD_DIR}"
    "${OPENCASCADE_INSTALL_DIR}"
)

# 设置特定的目录变量供CMake使用
set(VTK_DIR "${VTK_CMAKE_DIR}" CACHE PATH "VTK directory" FORCE)
set(OpenCASCADE_DIR "${OPENCASCADE_CMAKE_DIR}" CACHE PATH "OpenCASCADE directory" FORCE)

# -----------------------------------------------------------------------------
# 🔧 编译器和链接器配置
# -----------------------------------------------------------------------------
# 添加库目录
link_directories("${OPENCASCADE_LIB_DIR}")

# 添加包含目录
include_directories("${OPENCASCADE_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# 📝 配置总结
# -----------------------------------------------------------------------------
message(STATUS "=== 路径配置总结 ===")
message(STATUS "工具基础目录: ${TOOLS_BASE_DIR}")
message(STATUS "VTK目录: ${VTK_BUILD_DIR}")
message(STATUS "OpenCASCADE目录: ${OPENCASCADE_INSTALL_DIR}")
message(STATUS "==================")