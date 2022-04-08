cmake_minimum_required(VERSION 3.5)

project(meta LANGUAGES CXX)

add_executable(meta
	main.cpp
	include/meta/connection
	include/meta/connection.h
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
