add_library(DataModels
    BaseModel.cpp
    WorkpieceData.cpp
    TrajectoryData.cpp
    DataModels.cpp
)
target_include_directories(DataModels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(DataModels PUBLIC Qt6::Core Qt6::Gui)