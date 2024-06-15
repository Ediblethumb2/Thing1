

#set(CMAKE_CXX_COMPILER "C:/Program Files/JetBrains/CLion 2024.1.1/bin/mingw/bin/g++.exe")
cmake_minimum_required(VERSION 3.27)
project(LOGL)

find_package(OpenGL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# Set C++ standard to 23
include(FetchContent)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # release 3.4
)
FetchContent_MakeAvailable(glfw)
include(FetchContent)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v5.4.1
)

FetchContent_MakeAvailable(assimp)


set(CMAKE_CXX_STANDARD 23)
#SET( CMAKE_CXX_FLAGS  "-static-libstdc++")
# Include directories for GLFW
#include_directories("C:/Program Files/JetBrains/CLion 2024.1.1/bin/mingw/include")

# Add executable and link libraries
add_executable(LOGL main.cpp glad.h
        glad.cpp
        stb_images.h
        imgui_impl_glfw.cpp
        imgui.cpp
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_truetype.h
        imgui_tables.cpp
        imgui_draw.cpp
        imgui_impl_opengl3.cpp
        imgui_stdlib.cpp
        tinyobjloader.h
        GLFW/glfw3native.h
        glm/detail/_features.hpp
        glm/detail/_noise.hpp
        glm/detail/_swizzle.hpp
        glm/detail/_swizzle_func.hpp
        glm/detail/compute_vector_decl.hpp
        glm/detail/func_common_simd.inl
        glm/detail/func_exponential_simd.inl
        glm/detail/func_geometric_simd.inl
        glm/detail/func_integer_simd.inl
        glm/detail/func_matrix_simd.inl
        glm/detail/func_packing_simd.inl
        glm/detail/func_trigonometric_simd.inl
        glm/detail/func_vector_relational_simd.inl
        glm/detail/glm.cpp
        glm/detail/type_mat4x4_simd.inl
        glm/detail/type_quat_simd.inl
        glm/detail/type_vec4_simd.inl
        glm/ext.hpp

        KHR/khrplatform.h
        stb_image_write.h
        json.hpp
        tiny_gltf.h
        tiny_gltf.h
        fastgltf/base64.hpp
        Mesh.h
    )
target_include_directories(LOGL PRIVATE glm)

target_link_libraries(LOGL PRIVATE glfw)
target_link_libraries(LOGL PRIVATE assimp)
#target_link_libraries(LOGL PUBLIC OpenGL::GL glfw3)

# Copy DLL files
#file(COPY "C:/Program Files/JetBrains/CLion 2024.1.1/bin/mingw/lib/glfw3.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(COPY "C:/Program Files/JetBrains/CLion 2024.1.1/bin/mingw/lib/Assimp/assimp-vc143-mtd.lib" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_EXE_LINKER_FLAGS "-static")