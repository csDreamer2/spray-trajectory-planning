set(DATA_SOURCES
    BaseModel.cpp
    BaseModel.h
    BatchManager.cpp
    BatchManager.h
    WorkpieceData.cpp
    WorkpieceData.h
    TrajectoryData.cpp
    TrajectoryData.h
    DatabaseManager.cpp
    DatabaseManager.h
    DatabaseInitializer.cpp
    DatabaseInitializer.h
    # PCL相关文件
    PointCloudProcessor.cpp
    PointCloudProcessor.h
    PointCloudParser.cpp
    PointCloudParser.h
    ScanDataReceiver.cpp
    ScanDataReceiver.h
    TrajectoryPlanner.cpp
    TrajectoryPlanner.h
    DataModels.cpp
    DataModels.h
    # STEP模型树相关文件
    STEPModelTree.cpp
    STEPModelTree.h
)

add_library(Data STATIC ${DATA_SOURCES})

# 确保Qt库和PCL库被正确链接
target_link_libraries(Data PUBLIC
    Qt6::Core
    Qt6::Sql
    Qt6::Gui
    Qt6::Network
    ${PCL_LIBRARIES}
)

# 确保Qt的MOC处理
set_target_properties(Data PROPERTIES
    AUTOMOC ON
)

target_include_directories(Data PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})