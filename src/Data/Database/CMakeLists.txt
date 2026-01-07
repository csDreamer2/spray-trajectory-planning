add_library(DataDatabase
    DatabaseManager.cpp
    DatabaseInitializer.cpp
    BatchManager.cpp
)
target_include_directories(DataDatabase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(DataDatabase PUBLIC Qt6::Sql DataModels)