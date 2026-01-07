add_library(RobotControl
    RobotController.cpp
    MotoTcpClient.cpp
    ProgramGenerator.cpp
)
target_include_directories(RobotControl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(RobotControl PUBLIC
    Qt6::Core
    Qt6::Network
    RobotKinematics
)