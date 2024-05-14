# Learn OpenGL

This repository implements the [Learn OpenGL Tutorials](https://learnopengl.com/).

The main motivation for this repo is to learn by doing.

This repository differ's from the website author's [LearnOpenGl repository](https://github.com/JoeyDeVries/LearnOpenGL) in the following ways:
* use of some modern c++ idioms (eg: lambda's)
* simpler cmake configuration
* vcpkg compatibility
* flatter directory structure
* auto-formatting as described in `.clang-format`
* linting as described in  `.clang-tidy`

# Building

The project build with cmake using typical build commands like the following:
```
cmake -S . -B build --preset vcpkg-win -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

As a convenience, a preset called `vcpkg` is provided which obtains and builds dependencies on windows using vcpkg.
In order to use it, `VCPKG_ROOT` needs to be set.
One way to do this is to create a `CMakeUserPresets.json` file with contents analogous to this:
```
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "inherits": "vcpkg",
            "generator": "Ninja",
            "environment": {
                "VCPKG_ROOT": "C:/Users/$env{UserName}/r/vcpkg"
            }
        }
    ]
}
```
Then you can run
```
cmake -S . -B build --preset default -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

The build has been tested on the following platforms:
* Windows 10 with
[Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
and the powershell developer command prompt.
* Ubuntu 23.10 with clang/ninja
Alternate presets could be created to support other platforms, shells, and dependency-management methods.

# Running

To pickup the correct shaders, always run from the repo root, eg:

```
build/1.1.hello_window.exe
build/1.2.hello_triangle_ebo.exe
...
```

# Linting

Let the compiler worry about formatting and style. After building:
```
cmake --build build --target lint-all
```
This will invoke `clang-format` and `clang-tidy` for all source files.

Note that if using the MSVC `cl.exe` compiler, some commandline flags may be incorrect for `clang-tidy`.
