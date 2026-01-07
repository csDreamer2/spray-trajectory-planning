add_library(UIModelTree
    STEPModelTreeWidget.cpp
    ModelTreeDockWidget.cpp
)
target_include_directories(UIModelTree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(UIModelTree PUBLIC ${VTK_INCLUDE_DIRS})
target_link_libraries(UIModelTree PUBLIC
    Qt6::Widgets
    DataSTEP
    ${VTK_LIBRARIES}
)