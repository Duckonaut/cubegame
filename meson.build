project('cubegame', 'c',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'c_std=c2x',
  ],
)

cc = meson.get_compiler('c')

static_link_libs = false
if host_machine.system() == 'windows'
  static_link_libs = true
endif

if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  add_project_arguments('-Wconversion', '-Werror', '-g', language : 'c')
endif

if get_option('buildtype') == 'release'
  add_project_arguments('-DRELEASE', language : 'c')
endif

glfw_dep = dependency('glfw3', required : true, static : static_link_libs)
cglm_dep = dependency('cglm', required : true, static : static_link_libs)
glew_dep = dependency('glew', required : true, static : static_link_libs)
# cimgui is a cmake project
cmake = import('cmake')
cimgui_proj = cmake.subproject('cimgui')
cimgui_dep = cimgui_proj.dependency('cimgui')

deps = [
  glfw_dep,
  cglm_dep,
  glew_dep,
  cimgui_dep,
]
if host_machine.system() == 'linux'
  m_dep = cc.find_library('m', required : true)
  deps += m_dep
endif

asset_dir = join_paths(meson.current_source_dir(), 'assets')

finch = find_program('finch', required : true)

assets = [
  'assets/shaders/gizmo_frag.glsl',
  'assets/shaders/ui_frag.glsl',
  'assets/shaders/ui_vert.glsl',
  'assets/shaders/world_frag.glsl',
  'assets/shaders/world_vert.glsl',
  'assets/shaders/shadow_frag.glsl',
  'assets/shaders/shadow_vert.glsl',
  'assets/shaders/unlit_frag.glsl',
  'assets/textures/atlas.png',
  'assets/textures/cursor.png',
  'assets/textures/ui_atlas.png',
  'assets/textures/sun.png',
]

asset_data = custom_target(
  'asset_data.h',
  command : [finch, asset_dir, 'asset_data', '-p', 'a_'],
  output : ['asset_data.h'],
  input : assets,
)

src = [
  'src/assets.c',
  'src/camera.c',
  'src/game.c',
  'src/globals.c',
  'src/lighting.c',
  'src/log.c',
  'src/main.c',
  'src/mesh.c',
  'src/physics.c',
  'src/player.c',
  'src/saves.c',
  'src/shader.c',
  'src/ui.c',
  'src/utils.c',
  'src/world.c',
  asset_data
]

inc = include_directories('include')
sys_inc = include_directories('libs/stb', is_system : true) # stb_image.h

executable(
  'cubegame',
  src,
  include_directories : [inc, sys_inc],
  dependencies : deps,
  install : true
)
