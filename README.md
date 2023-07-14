# C1K3

![title](https://raw.githubusercontent.com/computermouth/c1k3-assets/master/gifs/title.gif)

C1K3 is a clone of [PhobosLab](https://phoboslab.org/)'s incredible [JS13K](https://js13kgames.com/) entry: [Q1K3](https://js13kgames.com/entries/q1k3). This version is written in C on top of SDL2 + GLES2.

While Q1K3 strives to fit inside a 13KB package, C1K3 does not. The binary weighs in at ~3MB, and links in quite a bit more in dependencies. Textures, models, and maps are included directly into the binary in their raw file formats where possible.

![action](https://raw.githubusercontent.com/computermouth/c1k3-assets/master/gifs/action.gif)

## build

```
# in your cloned directory
$ git submodule update --init
$ make release
```

### linux dependencies (debian-family)

```
$ sudo apt-get install -y gcc make xxd pkg-config libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

### windows dependencies (msys2)

I build and link c1k3 against ANGLE on Windows, using the ucrt64 env of [msys2](https://www.msys2.org/).

```
$ pacman -Sy make vim mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-angleproject
```

### mac dependencies (brew)

```
$ brew install pkg-config sdl2 sdl2_ttf sdl2_mixer
```

## asset sources?

C1K3 uses xxd to include the binary representations of the assets built for [Q1K3](https://github.com/phoboslab/q1k3).

In that repository are tools that bake the models and maps into smaller files. Those files were committed to [c1k3-assets](https://github.com/computermouth/c1k3-assets), rather than including the tools and rebuilding the assets on every build. This was done because it's my hope to switch to parsing [glTF files](https://en.wikipedia.org/wiki/GlTF) which are created in Blender to replace the Quake maps and the original author's model format.
