set(SIMULATION_SOURCES
    SimulationEngine.cpp
    SimulationEngine.h
    CollisionDetector.cpp
    CollisionDetector.h
    QualityPredictor.cpp
    QualityPredictor.h
)

add_library(Simulation STATIC ${SIMULATION_SOURCES})

target_link_libraries(Simulation
    Qt6::Core
    Qt6::OpenGL
)

target_include_directories(Simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})