add_subdirectory(Models)
add_subdirectory(Database)
add_subdirectory(STEP)
add_subdirectory(PointCloud)
add_subdirectory(Trajectory)

add_library(Data INTERFACE)
target_link_libraries(Data INTERFACE
    DataModels
    DataDatabase
    DataSTEP
    DataPointCloud
    DataTrajectory
)