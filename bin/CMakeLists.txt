add_executable(crypt main.cpp)

target_link_libraries(crypt PRIVATE crypt_lib)
target_include_directories(crypt PUBLIC ${PROJECT_SOURCE_DIR}/lib)
