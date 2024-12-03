set(GENERATED_CODE_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRCS ${GENERATED_CODE_DIR}/bigdata.pb.cc)
set(PROTO_HDRS ${GENERATED_CODE_DIR}/bigdata.pb.h)
set(PROTOC ${protobuf_BINARY_DIR}/protoc)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})


add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${PROTOC} 
    --proto_path ${PROTO_DIR} bigdata.proto 
    --cpp_out ${GENERATED_CODE_DIR}
    DEPENDS ${PROTOC} ${PROTO_DIR}/bigdata.proto
    )

#include_directories("${protobuf_SOURCE_DIR}/src/google/protobuf")
add_library(bigdata ${PROTO_HDRS} ${PROTO_SRCS})
target_include_directories(bigdata PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bigdata PRIVATE protobuf::libprotobuf)
