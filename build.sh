#!/bin/bash

current_baseline_version="2"

build_dir="$PWD/x64"
engine_dir="$PWD/engine"
dependencies_dir="$PWD/dependencies"
demo_dir="$PWD/demo"

include_dirs=""
include_dirs="$include_dirs -I$engine_dir"
include_dirs="$include_dirs -I$dependencies_dir/dds/include"

engine_sources=""
engine_sources="$engine_sources $engine_dir/engine_bitmap.cpp"
engine_sources="$engine_sources $engine_dir/engine_camera.cpp"
engine_sources="$engine_sources $engine_dir/engine_container.cpp"
engine_sources="$engine_sources $engine_dir/engine_ebo.cpp"
engine_sources="$engine_sources $engine_dir/engine_fbo.cpp"
engine_sources="$engine_sources $engine_dir/engine_light.cpp"
engine_sources="$engine_sources $engine_dir/engine_list.cpp"
engine_sources="$engine_sources $engine_dir/engine_log.cpp"
engine_sources="$engine_sources $engine_dir/engine_managed.cpp"
engine_sources="$engine_sources $engine_dir/engine_material.cpp"
engine_sources="$engine_sources $engine_dir/engine_mesh.cpp"
engine_sources="$engine_sources $engine_dir/engine_node.cpp"
engine_sources="$engine_sources $engine_dir/engine_object.cpp"
engine_sources="$engine_sources $engine_dir/engine_ovo.cpp"
engine_sources="$engine_sources $engine_dir/engine_pipeline_default.cpp"
engine_sources="$engine_sources $engine_dir/engine_pipeline_fullscreen2d.cpp"
#engine_sources="$engine_sources $engine_dir/engine_pipeline_fullscreenLighting.cpp"
#engine_sources="$engine_sources $engine_dir/engine_pipeline_gbuffer.cpp"
#engine_sources="$engine_sources $engine_dir/engine_pipeline_raytracing.cpp"
engine_sources="$engine_sources $engine_dir/engine_pipeline_shadowmapping.cpp"
engine_sources="$engine_sources $engine_dir/engine_pipeline.cpp"
engine_sources="$engine_sources $engine_dir/engine_program.cpp"
engine_sources="$engine_sources $engine_dir/engine_serializer.cpp"
engine_sources="$engine_sources $engine_dir/engine_shader.cpp"
#engine_sources="$engine_sources $engine_dir/engine_ssbo.cpp"
engine_sources="$engine_sources $engine_dir/engine_texture.cpp"
engine_sources="$engine_sources $engine_dir/engine_vao.cpp"
engine_sources="$engine_sources $engine_dir/engine_vbo.cpp"
engine_sources="$engine_sources $engine_dir/engine.cpp"

demo_sources=""
demo_sources="$demo_sources $demo_dir/main.cpp"

common_compiler_flags="-std=c++17 -D_DEBUG"
common_compiler_flags="$common_compiler_flags -g -O0" # debug version
#common_compiler_flags="$common_compiler_flags -g -O3" # release version

g++ $common_compiler_flags -o $build_dir/libengine.so $engine_sources -fPIC -shared -g $include_dirs
g++ $common_compiler_flags -o $build_dir/demo $demo_sources -g $include_dirs -L$build_dir -lengine -lglfw -lGLEW -Wl,-rpath,$build_dir
