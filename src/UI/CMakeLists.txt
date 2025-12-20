set(UI_SOURCES
    MainWindow.cpp
    MainWindow.h
    VTKWidget.cpp
    VTKWidget.h
    Simple3DWidget.cpp
    Simple3DWidget.h
    ParameterPanel.cpp
    ParameterPanel.h
    StatusPanel.cpp
    StatusPanel.h
    SafetyPanel.cpp
    SafetyPanel.h
    PointCloudLoader.cpp
    PointCloudLoader.h
)

add_library(UI STATIC ${UI_SOURCES})

target_link_libraries(UI
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    VTK::CommonCore
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::GUISupportQt
    VTK::InteractionStyle
    VTK::IOPLY
)

target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})