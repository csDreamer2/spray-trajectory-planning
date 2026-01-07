add_library(RobotKinematics
    RobotKinematics.cpp
)
target_include_directories(RobotKinematics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(RobotKinematics PUBLIC Qt6::Core Qt6::Gui)