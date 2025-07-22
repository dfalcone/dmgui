# dmgui
Dante's Deferred Mode GUI

Influenced by DearIMGui. Though this project has different goals which I was not able to achieve easily with DearIMGui.
* Target both games and tools applications
* Extensible data-driven component design
* Support for dynamic flex-like layouts (wip)
* Targeting use in C, C++, scripting languages and as data markup languages (wip)
* Data binding and animation (wip)
* User customizable styling (wip)

Deferred mode is basically 'not quite immediate mode'. In the update loop all draw functions are commands that are later processed at the end of the update. Instead of immediately going into an if block for a button, for example, a callback function (or lambda) is provided. All callbacks will execute in the order they are received.

# core dependencies
* stb_truetype.h (included in source)

# input backends
* SDL

# render backends
* OpenGL3 (Windows/Mac/Linux)
* GLES3 (Android)
* WebGL2 with Emscripten (Web)

# demo instructions
* sdl
* * download and extract the latest sdl2 release for your build platform https://github.com/libsdl-org/SDL/releases/tag/release-2.32.6
* cmake
* * Install a recent version of cmake
* * recommend using the cmake gui, otherwise cmake commandline has its own help
* * set source dir to the target demo folder (demo_sdl_gl3)
* * set build dir to a folder you wish to make the build in
* * click configure
* * if the sdl_dir path is not set, choose the cmake folder under sdl folder
* * click generate
* * click open to open the project in your default ide, or manually open it
* build
* * choose debug or release
* * build it all
* * run the demo_[input]_[render] project in debugger or run the executable directly from the build output folder
* data
* * the Ubuntu Regular ttf font is included and will automatically be copied to your build output folder by cmake
* * any shared library dependencies are copied to the build output folder by cmake

# lib instructions
* cmake
* * simply addsubdirectory for this repo root folder which contains CMakeLists.txt for linking as a static lib
* manual
* * copy all files in include and source folders to your project. ensure source files are all in the same folder.
* * copy the .h and .cpp backend files for input and rendering (or create your own) into the same folder as you put the source files.
* * set your build environment to include the include folder
* * set up your build environment with these defines:
* * * DMGUI_RENDER_[name] where the name matches the deliter in the backend header file name (DMGUI_RENDER_gl3)
* * * DMGUI_RENDER_H path to the render backend header
* * * DMGUI_INPUT_[name] where the name matches the deliter in the backend header file name (DMGUI_RENDER_sdl2)
* * * DMGUI_INPUT_H path to the input backend header
   
