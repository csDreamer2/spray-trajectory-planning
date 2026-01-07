add_library(RobotUI
    RobotControlPanel.cpp
)
target_include_directories(RobotUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(RobotUI PUBLIC
    Qt6::Widgets
    RobotControl
)