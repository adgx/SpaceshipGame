add_executable(ShaderTest
    main.cpp)

target_include_directories(ShaderTest PRIVATE ${CMAKE_SOURCE_DIR}/include/
                            PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(ShaderTest PRIVATE ShaderProgram
    PRIVATE glad_gl_core_33
    PRIVATE OpenGL::GL
    PRIVATE LogManager
    PRIVATE WindowManager)
    
set_target_properties(ShaderTest PROPERTIES FOLDER "Tests")