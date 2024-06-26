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

As a convenience, presets called `vcpkg` and `vcpkg-clang` are provided which obtains and builds dependencies on windows using vcpkg.
In order to use one, `VCPKG_ROOT` needs to be set.
One way to do this is to create a `CMakeUserPresets.json` file with contents analogous to this:
```
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "inherits": "vcpkg-clang",
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
and the powershell developer command prompt. **The build will not locate tools correctly from a standard powershell instance**.[1]
* Ubuntu 23.10 with clang/ninja
Alternate presets could be created to support other platforms, shells, and dependency-management methods.

# Running

You can run from any location, so long as the source repo hasn't been moved
(the repo root is used to resolve location of assets and shaders).

Eg:
```
build/1.1.hello_window.exe
cd build
./1.2.hello_triangle_ebo.exe
...
```

To run all from a ninja-based build on windows, run:
```
foreach ($f in Get-ChildItem build -Filter *.exe ) { . $f; }
```

# Linting

Let the compiler worry about formatting and style. After building:
```
cmake --build build --target lint-all
```
This will invoke `clang-format` and `clang-tidy` for all source files.

Note that if using the MSVC `cl.exe` compiler, some commandline flags may be incorrect for `clang-tidy`, which may trigger spurious errors.
When `clang` is used, `build/compile_commands.json` will be populated with command-line flags that `clang-tidy` can leverage.

[1] This can cause problems if, eg, your editor/ide (eg: `vscode`) does not have the dev command prompt environment variables set. One workaround is to [spawn your editor from a dev command prompt](https://code.visualstudio.com/docs/cpp/config-msvc#_check-your-microsoft-visual-c-installation).

# TODO
- use gl types consistently instead of `int`, `unsigned int`, etc
- apply blender glsl style guide consistently: https://developer.blender.org/docs/handbook/guidelines/glsl/