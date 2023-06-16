cmake_minimum_required(VERSION 3.23)

find_package(doctest)

project(meta
	LANGUAGES CXX
)

add_library(meta INTERFACE
	include/meta/util.h
	include/meta/connection include/meta/connection.h
	include/meta/object 	include/meta/object.h
)

target_compile_features(meta
	INTERFACE cxx_std_20
)

target_include_directories(meta
	INTERFACE include
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
