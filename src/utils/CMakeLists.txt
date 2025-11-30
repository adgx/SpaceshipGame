#stb_image
add_library(STB STATIC stb_image_impl.cpp)
target_include_directories(STB PRIVATE ${CMAKE_SOURCE_DIR}/include/utils)
target_link_libraries(STB PUBLIC glm)
set_target_properties(STB PROPERTIES FOLDER "stb_image")
