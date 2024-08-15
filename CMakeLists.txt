cmake_minimum_required(VERSION 3.16)

project(installer_fomod)

find_package(mo2-cmake CONFIG REQUIRED)

mo2_configure_extension()

add_subdirectory(src)
