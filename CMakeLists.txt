cmake_minimum_required(VERSION 3.12)

project(hiring_test DESCRIPTION "Hiring Test")
set(CMAKE_CXX_STANDARD 20)

# Enable multi-threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(hiring_test main.cpp)
target_include_directories(hiring_test SYSTEM PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(hiring_test SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/DirectXMath/Inc)
target_link_libraries(hiring_test PUBLIC Threads::Threads)
target_link_libraries(hiring_test PRIVATE OpenGL::GL GLUT::GLUT)