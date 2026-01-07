add_library(UIVisualization
    VTKWidget.cpp
    Simple3DWidget.cpp
)
target_include_directories(UIVisualization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(UIVisualization PUBLIC ${VTK_INCLUDE_DIRS})
target_link_libraries(UIVisualization PUBLIC
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${VTK_LIBRARIES}
)