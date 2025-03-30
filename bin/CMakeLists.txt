add_executable(crypt main.cpp
)

target_link_libraries(crypt PUBLIC crypt_lib)
target_include_directories(crypt PUBLIC ${PROJECT_SOURCE_DIR})
