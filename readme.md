# DokiSync

**Sync between multiple anime watch & track platforms**

Small utility that helps synchronize your anime library between different services.

Current status: **early development**

### Features
- Downloads full anime catalog from `anilib.me`
- Parses JSON responses using **Glaze**
- Stores minimal anime metadata in local SQLite database using **sqlite_orm**

### Stack

- **C++20**
- **Glaze**
- **cpr**
- **spdlog**
- **sqlite_orm**

### Build instructions

#### Prerequisites

- CMake 3.21+
- C++20 capable compiler (GCC 11+, Clang 13+, MSVC 2019+)
- Git

#### Build

```bash
git clone https://github.com/rbprsp/shikilib.git
cd shikilib

mkdir build && cd build
cmake ..
cmake --build . --config Release
```
