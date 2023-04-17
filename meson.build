project('cubegame', 'c',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'c_args=-Wall -Wextra -Wpedantic -Wconversion -Werror -g',
    'c_std=c11',
  ],
)

cc = meson.get_compiler('c')

glfw_dep = dependency('glfw3', required : true)
cglm_dep = dependency('cglm', required : true)
glew_dep = dependency('glew', required : true)
m_dep = cc.find_library('m', required : true)

src = [
  'src/assets.c',
  'src/camera.c',
  'src/globals.c',
  'src/main.c',
  'src/mesh.c',
  'src/player.c',
  'src/shader.c',
  'src/world.c',
]

inc = include_directories('include')
sys_inc = include_directories('libs/stb', is_system : true)

executable(
  'cubegame',
  src,
  include_directories : [inc, sys_inc],
  dependencies : [
    glfw_dep,
    cglm_dep,
    glew_dep,
    m_dep,
  ],
  install : true
)

