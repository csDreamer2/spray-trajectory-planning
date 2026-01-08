project(RobotKinematics)

# 找到Eigen3
find_package(Eigen3 REQUIRED)

# 添加源文件
set(SOURCES
    DHParameters.cpp
    RobotKinematics.cpp
)

set(HEADERS
    DHParameters.h
    RobotKinematics.h
)

# 创建库
add_library(RobotKinematics STATIC ${SOURCES} ${HEADERS})

# 链接依赖
target_link_libraries(RobotKinematics
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Eigen3::Eigen
)

# 包含目录
target_include_directories(RobotKinematics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置编译选项
if(MSVC)
    target_compile_options(RobotKinematics PRIVATE /W4)
else()
    target_compile_options(RobotKinematics PRIVATE -Wall -Wextra)
endif()

message(STATUS "RobotKinematics library configured")
