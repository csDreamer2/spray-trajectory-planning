set(ROBOT_SOURCES
    RobotController.cpp
    RobotController.h
    RobotKinematics.cpp
    RobotKinematics.h
    RobotControlPanel.cpp
    RobotControlPanel.h
    MotoTcpClient.cpp
    MotoTcpClient.h
    ProgramGenerator.cpp
    ProgramGenerator.h
)

add_library(Robot STATIC ${ROBOT_SOURCES})

target_link_libraries(Robot
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::Gui
)

target_include_directories(Robot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})