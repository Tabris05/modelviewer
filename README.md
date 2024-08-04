A toy 3D model viewer I wrote to teach myself the fundamentals of realtime graphics programming.
### Features
- PBR Material System
- Dynamic Direct Lighting
- Directional Shadowmapping
- Image-based Global Illumination
- Physically-Based Bloom
- Multisampled Antialiasing
### Supported Formats
3D model files must be of the `.glTF` or `.glb` format and skyboxes must be equirectangular projections of the `.hdr` format.
### Controls
- **Movement** - `WSAD` to move forward/backward/left/right, `Space/LCTRL` to move up/down, and hold `LSHIFT` to increase speed
- **Mouselook** Hold `Left Click` when not hovering over a GUI element
- **Lock Axis of Rotation** - Hold `LSHIFT/LCTRL/LALT` when hovering over a rotational GUI element
### Dependencies:
- **[GLFW](https://github.com/glfw/glfw)** - Window context and input handling
- **[GLAD](https://github.com/Dav1dde/glad)** - OpenGL function bindings
- **[GLM](https://github.com/g-truc/glm)** - CPU-side linear algebra computations
- **[STB](https://github.com/nothings/stb)** - Image loading
- **[fastgltf](https://github.com/spnda/fastgltf)** - 3D model loading
- **[MikkTSpace](https://github.com/mmikk/MikkTSpace)** - Calculate vertex tangent vectors
- **[Dear ImGui](https://github.com/ocornut/imgui)** - GUI elements
- **[imGuIZMO.quat](https://github.com/BrutPitt/imGuIZMO.quat)** - Rotational GUI elements
