find_package(Boost REQUIRED unit_test_framework timer system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

get_cmake_property(VARNAMES VARIABLES)

qpmad_add_test("test_givens" "givens.cpp")
qpmad_add_test("test_cholesky" "cholesky.cpp")
qpmad_add_test("test_inverse" "inverse.cpp")
qpmad_add_test("test_solver" "solver.cpp")
