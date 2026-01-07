add_library(DataTrajectory
    TrajectoryPlanner.cpp
)
target_include_directories(DataTrajectory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(DataTrajectory PUBLIC DataModels)