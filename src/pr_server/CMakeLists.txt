add_subdirectory(proto_gen)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(pr_server main.cpp)

target_link_libraries(
  pr_server
  PRIVATE myproject::myproject_options
          myproject::myproject_warnings
          bigdata
        )

target_link_system_libraries(
  pr_server
  PRIVATE
          fmt::fmt
          spdlog::spdlog
          Boost::asio
          Boost::beast
          Boost::json
          protobuf::libprotobuf
          )

target_include_directories(pr_server PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

target_compile_options(pr_server PRIVATE -fsanitize=address)
target_link_options(pr_server PRIVATE -fsanitize=address)
