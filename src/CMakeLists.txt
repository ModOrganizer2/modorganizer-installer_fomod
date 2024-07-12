cmake_minimum_required(VERSION 3.16)

find_package(mo2-cmake CONFIG REQUIRED)
find_package(mo2-uibase CONFIG REQUIRED)

add_library(installer_fomod SHARED)
mo2_configure_plugin(installer_fomod WARNINGS OFF)
target_link_libraries(installer_fomod PRIVATE mo2::uibase)
mo2_install_plugin(installer_fomod)
