add_library(UILoaders
    PointCloudLoader.cpp
)
target_include_directories(UILoaders PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(UILoaders PUBLIC ${VTK_INCLUDE_DIRS})
target_link_libraries(UILoaders PUBLIC
    Qt6::Widgets
    DataPointCloud
    ${VTK_LIBRARIES}
)