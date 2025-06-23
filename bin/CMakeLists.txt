add_executable(cote main.cpp
)

target_link_libraries(cote PUBLIC cote_lib)
target_include_directories(cote PUBLIC ${PROJECT_SOURCE_DIR})
