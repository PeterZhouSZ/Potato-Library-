###############################################################################
# Vitelotte mvgtk                                                             #
###############################################################################

include_directories(
    ${EIGEN3_INCLUDE_DIR}
    "../../.."
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(vitelotte_mvgtk_SRCS
    solveCommand.cpp
    mvgtk.cpp
    changeColorSpaceCommand.cpp
    checkCommand.cpp
    compactCommand.cpp
    convertColorSpaceCommand.cpp
    convertCommand.cpp
    curvesToNodesCommand.cpp
    finalizeCommand.cpp
    plotCommand.cpp
    simplifyCommand.cpp
    main.cpp
)
add_executable(mvgtk ${vitelotte_mvgtk_SRCS})
add_dependencies(vitelotte_examples mvgtk)

target_link_libraries(mvgtk
    vitelotte_examples_common
)

