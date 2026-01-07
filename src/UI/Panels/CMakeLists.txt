add_library(UIPanels
    ParameterPanel.cpp
    StatusPanel.cpp
    SafetyPanel.cpp
    WorkpieceManagerPanel.cpp
)
target_include_directories(UIPanels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(UIPanels PUBLIC Qt6::Widgets)