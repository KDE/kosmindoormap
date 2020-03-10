add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(osmtypetest.cpp LINK_LIBRARIES Qt5::Test KOSM)
ecm_add_test(indexeddatatabletest LINK_LIBRARIES Qt5::Test)

