add_library(DataPointCloud
    PointCloudParser.cpp
    PointCloudProcessor.cpp
    ScanDataReceiver.cpp
)
target_include_directories(DataPointCloud PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(DataPointCloud PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    ${PCL_LIBRARIES}
)