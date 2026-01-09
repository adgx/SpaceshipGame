#stb_image
add_library(STB STATIC stb_image_impl.cpp)
target_include_directories(STB PRIVATE ${CMAKE_SOURCE_DIR}/include/utils)
target_link_libraries(STB PUBLIC glm)
set_target_properties(STB PROPERTIES FOLDER "stb_image")

#Utils
add_library(Utils STATIC utils.cpp)
target_include_directories(Utils PRIVATE ${CMAKE_SOURCE_DIR}/include/utils
                                PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
set_target_properties(Utils PROPERTIES FOLDER "Utils")
target_link_libraries(Utils PUBLIC assimp
                            PUBLIC glm
                            PRIVATE WindowManager)