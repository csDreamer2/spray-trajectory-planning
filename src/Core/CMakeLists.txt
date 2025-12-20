set(CORE_SOURCES
    Application.cpp
    Application.h
    ConfigManager.cpp
    ConfigManager.h
    Logger.cpp
    Logger.h
)

add_library(Core STATIC ${CORE_SOURCES})

target_link_libraries(Core
    Qt6::Core
    Qt6::Network
)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})