project(tinyobjloader)
cmake_minimum_required(VERSION 3.8)

add_subdirectory(lib/tiny_obj_loader)

add_executable(main main.cpp)

set(BINDIR ${CMAKE_BINARY_DIR})

set_target_properties(
    main
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BINDIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINDIR}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)


target_include_directories(
    main
    PRIVATE lib/
    PRIVATE lib/glm-0.9.8.5)

target_link_libraries(
    main
    PRIVATE tinyobjloader)