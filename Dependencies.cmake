include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(myproject_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET boost::boost)
    cpmaddpackage(
      NAME
      Boost 
      VERSION
      1.86.0
      GITHUB_REPOSITORY
      "boostorg/boost"
      GIT_TAG
      "boost-1.86.0"
    )
  endif()

  if(NOT TARGET protobuf)
    cpmaddpackage(
      NAME
      protobuf
      VERSION
      25.1
      GITHUB_REPOSITORY
      "protocolbuffers/protobuf"
      GIT_TAG
      "v25.1"
      OPTIONS
      "protobuf_BUILD_TEST OFF"
    )
  endif()

endfunction()
