# C++ Cloth Simulation (Vellum)

A real-time **mass–spring cloth simulator** written in modern C++ and rendered with **OpenGL 3.3 Core**. Particles are connected on a grid by structural springs; motion is integrated with **Verlet integration**, and spring lengths are enforced each frame with **positional constraint relaxation**. The result is a stable, interactive cloth that sags under gravity, can be reset, re-rendered in different modes, and manipulated with the mouse.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [How It Works](#how-it-works)
4. [Architecture](#architecture)
5. [Project Structure](#project-structure)
6. [Dependencies](#dependencies)
7. [Building](#building)
8. [Running](#running)
9. [Controls](#controls)
10. [Simulation Parameters](#simulation-parameters)
11. [Rendering](#rendering)
12. [Module Reference](#module-reference)
13. [Troubleshooting](#troubleshooting)
14. [Roadmap](#roadmap)
15. [License](#license)

---

## Overview

This project simulates a rectangular sheet of cloth as a 2D grid of particles. By default it uses a **20×20** grid with **0.5** unit spacing. The **top-left and top-right corners** are pinned so the cloth hangs from two fixed points.

The application runs a classic game loop: read input → advance physics with a fixed internal timestep → upload vertex data to the GPU → draw. Everything is CPU-side physics with GPU rendering via VAOs, VBOs, and GLSL shaders.

**Executable name:** `ClothSimulation`  
**Default window:** 800×600, title `"Cloth Simulation"`  
**OpenGL profile:** 3.3 Core (via GLFW + GLEW)

---

## Features

| Area | What is implemented |
|------|---------------------|
| **Physics** | Mass–spring grid, Verlet integration, iterative distance constraints, gravity, velocity damping |
| **Constraints** | Structural springs (horizontal + vertical neighbors only); pinned corners; grab-and-drag via mouse |
| **Rendering** | Line mode (springs as green segments) and triangle mode (lit solid mesh with optional wireframe) |
| **Interaction** | Reset, toggle render mode, wireframe, constraint iteration count, scroll-wheel zoom, mouse grab/release |
| **Stability** | Fixed physics substep (`1/120` s), frame delta clamped to avoid spiral-of-death on lag spikes |

**Not yet implemented** (see [Roadmap](#roadmap)): shear/bend springs, wind, collisions, self-collision, tearing, GPU compute, runtime damping keys, texture mapping.

---

## How It Works

### Mass–spring model

Each grid cell corner is a **particle** with position `pos`, previous position `prevPos`, and an optional `fixed` flag. Neighboring particles are linked by **springs** with a **rest length** equal to the grid spacing. Only **structural** springs are created (right and down neighbors)—no diagonal shear or bend springs yet.

### Verlet integration

For each non-fixed particle each substep:

```
velocity ≈ pos - prevPos
pos = pos + damping * velocity + acceleration * dt²
prevPos = previous pos
```

Acceleration comes from **gravity** (`GRAVITY = -9.81` on the Y axis). Damping scales implicit velocity each step (`DEFAULT_DAMPING = 0.994`).

### Constraint satisfaction

After integration, spring lengths are corrected with several **iterations** of a simple position-based solver. For each spring connecting particles A and B:

1. Compute vector `delta = B.pos - A.pos` and current length `dist`.
2. Compute correction factor `diff = (dist - restLength) / dist`.
3. Move A and B along `delta` by `constraintAlpha * diff` (split evenly if both are free; full correction on the free particle if the other is pinned).

This is **not** a full XPBD solver; it is a fast, iterative Jacobi-style relaxation that works well for cloth at moderate iteration counts (default **10**).

### Time stepping

Wall-clock `deltaTime` from GLFW is clamped to at most `1/30` s. The simulator then runs as many **fixed substeps** of `1/120` s as needed:

```
substeps = max(1, floor(deltaTime / (1/120)))
```

Each substep: integrate → satisfy constraints. This decouples visual framerate from physics stability.

### Mouse interaction

A screen-space ray is built from the mouse position and camera matrices. The closest particle within a threshold distance to that ray can be **grabbed**: it is temporarily marked `fixed` and moved to the intersection of the ray with the plane `z = 0`. On release, pin state is restored (only top corners remain pinned).

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp                              │
│  Game loop · input · camera · cloth.update() · render()     │
└────────────┬───────────────────────────────┬────────────────┘
             │                               │
     ┌───────▼────────┐              ┌───────▼────────┐
     │   WindowGL     │              │     Cloth      │
     │  GLFW + GLEW   │              │ Verlet + PBD   │
     │  input/events  │              │ particles/     │
     └────────────────┘              │ springs        │
                                     └───────┬────────┘
                                             │ particle positions
                                     ┌───────▼────────┐
                                     │    Renderer    │
                                     │ shaders · VAO  │
                                     │ lines / tris   │
                                     └───────┬────────┘
                                             │
                                     ┌───────▼────────┐
                                     │     Shader     │
                                     │ compile/link   │
                                     └────────────────┘
```

**Data flow per frame:**

1. Poll keyboard/mouse/scroll from `WindowGL`.
2. Update camera view matrix if the user scrolled.
3. Handle grab/release and call `Cloth::update(deltaTime)`.
4. `Renderer::render(cloth)` uploads positions (and normals in triangle mode) and draws.
5. Swap buffers and poll GLFW events.

---

## Project Structure

```
C++ vellum/
├── CMakeLists.txt          # Main build definition
├── build.bat               # Windows build script (Visual Studio 2019, x64)
├── build.sh                # Unix-style build script
├── setup_libraries.ps1     # Downloads GLFW, GLEW, GLM into external/
├── fix_glew.ps1            # Re-fetches GLEW DLLs if needed
├── README.md               # This file
├── PROJECT_SUMMARY.md      # Short implementation status notes
│
├── src/
│   ├── main.cpp            # Entry point, game loop, camera, mouse picking
│   ├── cloth.h / cloth.cpp # Physics: particles, springs, integration
│   ├── renderer.h / .cpp   # OpenGL draw path, shaders embedded as strings
│   ├── shader.h / .cpp     # GLSL compile, link, uniform helpers
│   ├── window_gl.h / .cpp  # GLFW window, GLEW init, input
│   └── math_types.hpp      # GLM includes, constants, grid index helper
│
├── external/
│   ├── include/            # GLFW, GLEW, GLM headers
│   └── lib/                # Prebuilt Windows libs (glfw3, glew32s, …)
│
├── build/                  # CMake output (generated; not required in repo)
│   └── bin/Release/        # ClothSimulation.exe (after Release build)
│
└── test_opengl/            # Minimal OpenGL smoke test (separate target)
```

**Auxiliary / test sources** (not linked into the main executable):

- `src/minimal_test.cpp`, `src/simple_test.cpp`
- `CMakeLists_minimal.txt`, `CMakeLists_test.txt`

---

## Dependencies

| Library | Role | Version (bundled setup) |
|---------|------|-------------------------|
| **OpenGL** | Graphics API | 3.3+ Core |
| **GLFW** | Window, context, input | 3.3.8 (via `setup_libraries.ps1`) |
| **GLEW** | OpenGL extension loader | 2.1.0, **static** (`glew32s.lib` + `GLEW_STATIC`) |
| **GLM** | Vectors, matrices | 0.9.9.8 |
| **CMake** | Build system | 3.10+ |
| **C++ compiler** | — | **C++17** required |

### Windows (recommended: bundled libraries)

Run once from the project root (requires network):

```powershell
.\setup_libraries.ps1
```

This populates `external/include` and `external/lib`. If the app fails at runtime with missing GLEW DLL issues, run `.\fix_glew.ps1` (the main build uses **static** GLEW, so DLLs are usually not required).

### Windows (alternative: vcpkg)

```powershell
vcpkg install glfw3 glew glm
```

You would need to adjust `CMakeLists.txt` to use vcpkg’s imported targets instead of `external/lib` paths.

### Linux (Debian/Ubuntu)

```bash
sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libglm-dev
```

On Linux you may need to change `CMakeLists.txt` to link system libraries (`glfw`, `GLEW::GLEW`) instead of `external/lib/*.lib`.

### macOS

```bash
brew install glfw glew glm cmake
```

---

## Building

### Prerequisites

- **CMake** 3.10 or newer  
- A **C++17** toolchain  
- **Windows:** Visual Studio 2019 (or Build Tools) with the **x64** C++ workload  
- **OpenGL 3.3+** capable GPU and up-to-date graphics drivers  

### Windows (quick)

```bat
build.bat
```

Or manually:

```powershell
mkdir build -ErrorAction SilentlyContinue
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

Output: `build\bin\Release\ClothSimulation.exe`

For a **Debug** build:

```powershell
cmake --build . --config Debug
```

Executable: `build\bin\Debug\ClothSimulation.exe`

### Linux / macOS

```bash
chmod +x build.sh
./build.sh
```

Or:

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

Adjust `CMakeLists.txt` for system library paths if you are not using the Windows `external/` layout.

### CMake details

- Target: `ClothSimulation`
- Defines: `GLEW_STATIC` (matches static GLEW link)
- Include paths: `src/`, `external/include`
- Link (Windows bundle): `glfw3.lib`, `glew32s.lib`, OpenGL system libs
- Runtime output: `${CMAKE_BINARY_DIR}/bin` (with Visual Studio multi-config: `bin/Release` or `bin/Debug`)

---

## Running

**Windows (Release):**

```powershell
cd build\bin\Release
.\ClothSimulation.exe
```

**Linux / macOS:**

```bash
cd build/bin
./ClothSimulation
```

On startup you should see console messages for grid initialization, spring count, and renderer setup. The cloth appears in the XY plane at `z = 0`, centered near the origin, with the top corners pinned.

---

## Controls

| Input | Action |
|-------|--------|
| **R** | Reset cloth to initial grid and pin state |
| **Space** | Toggle render mode: **lines** (springs) ↔ **triangles** (solid mesh) |
| **W** | Toggle **wireframe** (triangle mode only) |
| **Up / Down** | Increase / decrease constraint iterations (clamped **1–20**) |
| **Scroll wheel** | Zoom camera in/out (distance **2–20** units along Z) |
| **Left mouse (click)** | Grab nearest particle along view ray |
| **Left mouse (drag)** | Move grabbed particle on the `z = 0` plane |
| **Left mouse (release)** | Release particle; restore corner pins if applicable |
| **Close window** | Exit |

> **Note:** `Cloth::setDamping()` and `Cloth::togglePin()` exist in code but are not bound to keyboard keys in `main.cpp`. Damping is fixed at compile time via `DEFAULT_DAMPING` in `math_types.hpp`.

---

## Simulation Parameters

Constants live in `src/math_types.hpp`:

| Symbol | Default | Meaning |
|--------|---------|---------|
| `GRAVITY` | `-9.81f` | Vertical acceleration (m/s² style units) |
| `DEFAULT_DAMPING` | `0.994f` | Per-step velocity scaling in Verlet |
| `DEFAULT_SPRING_STIFFNESS` | `1500.0f` | Stored on each `Spring` (`k`); constraint solver uses `constraintAlpha` for corrections |
| `DEFAULT_CONSTRAINT_ALPHA` | `0.5f` | Blending factor per constraint iteration |
| `DEFAULT_CONSTRAINT_ITERATIONS` | `10` | Relaxation passes per substep (runtime: Up/Down) |

Grid setup in `main.cpp`:

```cpp
Cloth cloth(20, 20, 0.5f);  // cols, rows, spacing
```

To change grid size or spacing, edit that line and rebuild. Very large grids (e.g. 80×80+) will cost more CPU per frame.

---

## Rendering

### Line mode (default)

- Draws each spring as a **green** line segment (`GL_LINES`, width 3).
- Simple shader: MVP transform only, no lighting.

### Triangle mode

- Fills the cloth mesh using precomputed **index buffer** (two triangles per grid quad).
- **Phong-style diffuse** lighting in the fragment shader with a fixed light at `(5, 5, 5)`.
- **Normals** are computed per frame by accumulating face normals at each vertex, then normalizing.
- **W** enables `GL_LINE` polygon mode for wireframe overlay on the mesh.

### Camera

- Perspective projection: 45° FOV, near `0.1`, far `100`.
- Camera starts at `(0, 0, 8)` looking at the origin.
- Scroll adjusts Z distance; view matrix is updated each zoom.

### OpenGL state

- Depth testing enabled in `WindowGL` constructor.
- Core profile 3.3; GLEW loaded after context creation.

---

## Module Reference

### `Cloth` (`cloth.h` / `cloth.cpp`)

- **`Particle`**: `pos`, `prevPos`, `fixed`.
- **`Spring`**: indices `i`, `j`, `restLength`, stiffness `k`.
- **`update(dt)`**: fixed substeps + Verlet + constraints.
- **`reset()`**: Rebuilds grid and pins.
- **`getClosestParticle` / `grabParticle` / `updateGrabbedParticle` / `releaseParticle`**: mouse interaction.

### `Renderer` (`renderer.h` / `renderer.cpp`)

- Owns two `Shader` programs (lines vs triangles).
- **`initialize()`**: compile embedded GLSL, create VAOs/VBOs.
- **`render(cloth)`**: clear, update buffers, draw current mode.
- **`setRenderMode(0|1)`**, **`setWireframeMode(bool)`**, matrix setters.

### `Shader` (`shader.h` / `shader.cpp`)

- Load from files or strings; compile vertex/fragment stages; link program.
- Uniform setters: `setMat4`, `setVec3`, `setFloat`, etc.
- Logs compile/link errors to `stderr`.

### `WindowGL` (`window_gl.h` / `window_gl.cpp`)

- Creates 3.3 Core context, initializes GLEW, registers framebuffer and scroll callbacks.
- **`isKeyPressed`**, **`isMouseButtonPressed`**, **`isMouseButtonJustPressed`**, **`getScrollY`**.

### `main.cpp`

- Wires components together; implements ray casting for mouse picks and the main loop.

---

## Troubleshooting

### CMake: “does not appear to contain CMakeLists.txt”

Ensure `CMakeLists.txt` exists in the project root. Re-run `cmake ..` from `build/`.

### Link errors: `__imp_glewInit` / unresolved GLEW symbols

The project links **static** GLEW (`glew32s.lib`). The target must define **`GLEW_STATIC`** (already set in `CMakeLists.txt`). Do not mix `glew32.lib` (DLL import) with static headers.

### “Failed to initialize GLFW” / “Failed to create GLFW window”

- Update graphics drivers.
- Confirm OpenGL 3.3+ support.
- On laptops, ensure the discrete GPU is used for the executable if applicable.

### Black or empty window

- Check console for shader compile errors.
- Try **Space** to switch to triangle mode.
- Press **R** to reset if particles flew off-screen (unstable settings).

### Cloth explodes or jitters

- Increase constraint iterations (**Up** arrow).
- Reduce effective timestep load (fewer substeps happen automatically if FPS is low, but huge `deltaTime` is clamped).
- Lower `DEFAULT_CONSTRAINT_ALPHA` or adjust damping in `math_types.hpp`.

### `build.bat` fails: wrong Visual Studio generator

Edit `build.bat` to match your installed VS version, for example:

```bat
cmake .. -G "Visual Studio 17 2022" -A x64
```

### Linux: cannot find `glfw3.lib`

Windows `.lib` files in `external/lib` are not used on Linux. Install dev packages and update `CMakeLists.txt` to use `find_package(glfw3)` / `GLEW::GLEW`.

### Warning `LNK4098: defaultlib 'LIBCMT' conflicts...`

Harmless in many setups: static GLEW was built against a different CRT than `/MD`. The Release build still produces a working executable.

---

## Roadmap

Planned enhancements (from project design notes):

1. **Physics:** shear and bend springs, wind, sphere/plane collision, self-collision, tearing.
2. **Interaction:** runtime damping control, paint pins with mouse, improved orbit camera.
3. **Rendering:** textures, better materials, optional GPU compute for particles.
4. **Tooling:** CI, cross-platform CMake package imports (vcpkg/Conan).

---

## License

This project is provided as open source for learning and modification. Add a specific license file (e.g. MIT) if you intend to distribute it formally.

---

## Quick Reference

```text
Build (Windows):  build.bat
Run (Windows):    build\bin\Release\ClothSimulation.exe
Grid default:     20 × 20, spacing 0.5
Physics step:     1/120 s (with substeps)
Pins:             top-left and top-right corners
```

For a shorter status snapshot of implemented vs planned work, see `PROJECT_SUMMARY.md`.
