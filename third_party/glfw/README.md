# GLFW (Vendored)

- **Upstream Repository**: https://github.com/glfw/glfw
- **Version**: 3.4.0
- **License**: zlib/libpng (see `LICENSE.md`)

## Notes for Null Engine Maintainers
This copy of GLFW has been curated to remove unused docs, examples, tests, and demo dependencies (Nuklear, GLAD, stb, linmath).
The core multi-platform library (`src/`), public headers (`include/`), CMake modules (`CMake/`), and platform protocol shims (`deps/wayland`, `deps/mingw`) are retained for full cross-platform builds across Linux, Windows, and macOS.
