add_subdirectory(Panels)
add_subdirectory(Visualization)
add_subdirectory(ModelTree)
add_subdirectory(Loaders)

add_library(UI
    MainWindow.cpp
)
target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(UI PUBLIC
    Qt6::Widgets
    Qt6::Core
    UIPanels
    UIVisualization
    UIModelTree
    UILoaders
    Data
    Robot
    Simulation
)