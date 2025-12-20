set(ROBOT_SOURCES
    RobotController.cpp
    RobotController.h
    MotoTcpClient.cpp
    MotoTcpClient.h
    ProgramGenerator.cpp
    ProgramGenerator.h
)

add_library(Robot STATIC ${ROBOT_SOURCES})

target_link_libraries(Robot
    Qt6::Core
    Qt6::Network
)

target_include_directories(Robot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})