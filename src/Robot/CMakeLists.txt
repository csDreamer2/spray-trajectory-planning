add_subdirectory(Kinematics)
add_subdirectory(Control)
add_subdirectory(UI)

add_library(Robot INTERFACE)
target_link_libraries(Robot INTERFACE
    RobotKinematics
    RobotControl
    RobotUI
)