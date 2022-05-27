# FarPDF

# Build

Dependencies: `Qt6`, `C++ 20 Compiler` and `libmupdf`

```bash
cmake -S . -B build \
      --install-prefix=/usr/local \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 6

# The binary will end up in `build/src/FarPDF`.

cmake --install build

# The binary will installed to /usr/local and
#   an app launcher will be installed to yor desktop's launcher
```

