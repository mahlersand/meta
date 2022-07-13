cmake_minimum_required(VERSION 3.14)

find_package(doctest)

project(meta
	LANGUAGES CXX
)

add_library(meta
	include/meta/dummy.cpp
	include/meta/connection
	include/meta/connection.h
	include/meta/connection_old.h
	include/meta/object
	include/meta/object.h
	include/meta/util.h
)

target_compile_features(meta
	PUBLIC cxx_std_20
)

target_include_directories(meta
	PUBLIC include
)

if(doctest_FOUND)
	add_executable(tests
		tests/tests.cpp
	)

    target_link_libraries(tests
		PUBLIC doctest::doctest
		PUBLIC meta
	)
endif()
