include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(myproject_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET fmtlib::fmtlib)
    cpmaddpackage("gh:fmtlib/fmt#9.1.0")
  endif()

  if(NOT TARGET spdlog::spdlog)
    cpmaddpackage(
      NAME
      spdlog
      VERSION
      1.11.0
      GITHUB_REPOSITORY
      "gabime/spdlog"
      OPTIONS
      "SPDLOG_FMT_EXTERNAL ON")
  endif()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage("gh:catchorg/Catch2@3.3.2")
  endif()

  if(NOT TARGET CLI11::CLI11)
    cpmaddpackage("gh:CLIUtils/CLI11@2.3.2")
  endif()

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
    )
  endif()

endfunction()
