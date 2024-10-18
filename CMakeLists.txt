cmake_minimum_required(VERSION 3.6)

project(Blockchain)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
set(OPENSSL_PATH "${CMAKE_SOURCE_DIR}/openssl-3/x64")
else()
set(OPENSSL_PATH "${CMAKE_SOURCE_DIR}/openssl-3/x86")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /utf-8")

add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${PROJECT_BINARY_DIR}/compile_commands.json
        ${PROJECT_SOURCE_DIR}
    )


  add_executable(${PROJECT_NAME} main.cpp CryptoUtils.cpp Block.cpp Clients.cpp Chain.cpp Network.cpp)

find_package(OpenSSL REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC "${OPENSSL_PATH}/include/")

target_link_directories(${PROJECT_NAME} PRIVATE "${OPENSSL_PATH}/lib/")

if (build_type STREQUAL debug)
message("Debug build")
target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto OpenSSL::SSL)
# target_link_libraries(${PROJECT_NAME} libssl libcrypto)
elseif (build_type STREQUAL release)
message("Release build")
target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto OpenSSL::SSL)
# target_link_libraries(${PROJECT_NAME} libssl libcrypto)
endif()
