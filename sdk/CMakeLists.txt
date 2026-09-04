cmake_minimum_required(VERSION 3.22.1)
project(nacl_native C CXX)

include_directories(include)

find_library(LOG_LIB log)

# Non-main source files for shared library
file(GLOB ALL_SRCS "src/*.c" "src/*.cpp")
list(FILTER ALL_SRCS EXCLUDE REGEX "src/(mock_client_main\\.c|sensors_daemon\\.c|service_daemon\\.c|shm_client\\.c|shm_daemon\\.c)$")

add_library(nacl_core SHARED ${ALL_SRCS})
if(LOG_LIB)
    target_link_libraries(nacl_core PRIVATE ${LOG_LIB})
endif()

# Executable targets ending with _svc as expected by workflow packaging
add_executable(sensors_svc src/sensors_daemon.c)
target_link_libraries(sensors_svc PRIVATE nacl_core)

add_executable(service_svc src/service_daemon.c)
target_link_libraries(service_svc PRIVATE nacl_core)

add_executable(shm_svc src/shm_daemon.c)
target_link_libraries(shm_svc PRIVATE nacl_core)
