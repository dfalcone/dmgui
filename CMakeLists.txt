
cmake_minimum_required (VERSION 3.10)
set(Proj DmGui)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND sourceFiles
  "include/dmgui/dmgui.h"
  "include/dmgui/dmgui_config.h"
  "include/dmgui/dmgui_font.h"
  "source/dmgui.cpp"
  "source/dmgui_font.cpp"
  "source/stb_truetype.h"
)

set(RenderBackend "gl3" CACHE STRING "")
set(InputBackend "sdl2" CACHE STRING "")

list(APPEND sourceFiles
  "backends/dmgui_render_${RenderBackend}.h"
  "backends/dmgui_render_${RenderBackend}.cpp"
  "backends/dmgui_input_${InputBackend}.h"
  "backends/dmgui_input_${InputBackend}.cpp"
)

add_library (${Proj} STATIC ${sourceFiles})
target_include_directories (${Proj} PUBLIC "include")

target_compile_definitions(${Proj} PUBLIC
  DMGUI_RENDER_${RenderBackend}
  DMGUI_RENDER_H="${CMAKE_CURRENT_SOURCE_DIR}/backends/dmgui_render_${RenderBackend}.h"
  DMGUI_INPUT_${InputBackend}
  DMGUI_INPUT_H="${CMAKE_CURRENT_SOURCE_DIR}/backends/dmgui_input_${InputBackend}.h"
)

set_property(TARGET ${Proj} PROPERTY CXX_STANDARD 17)
