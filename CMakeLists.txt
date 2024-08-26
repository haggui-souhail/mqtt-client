cmake_minimum_required(VERSION 3.10)
project(MQTT_CLIENT CXX)
set(CMAKE_CXX_STANDARD 17) # C++11

find_package(PkgConfig REQUIRED)
pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)

# Include directories
include_directories(
        ${MQTT_CLIENT_SOURCE_DIR}/include
)

# Your source files
set(SOURCES
    ${MQTT_CLIENT_SOURCE_DIR}/src/main.cpp
    ${MQTT_CLIENT_SOURCE_DIR}/src/MQTTMngr.cpp
    ${MQTT_CLIENT_SOURCE_DIR}/src/barecode.cpp
)

# Create the executable, making it depend on the custom target
add_executable(mqtt_client ${SOURCES})

# Link against the Thrift library
target_link_libraries(mqtt_client 
    mosquitto
    ulog
)
install(TARGETS mqtt_client RUNTIME DESTINATION bin)
