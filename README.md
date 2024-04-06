A toy 3D model viewer I wrote to teach myself the fundamentals of realtime graphics programming.
### Features
- Fully dynamic lighting
- Directional Shadowmapping
- Physically Based Shading
- Image-based Global Illumination
- Multisampled Antialiasing
### Supported Formats
3D model files must be of the `.glTF` format and skyboxes must be equirectangular projections of the `.hdr` format.
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
- **[Dear ImGui](https://github.com/ocornut/imgui)** - GUI elements
- **[imGuIZMO.quat](https://github.com/BrutPitt/imGuIZMO.quat)** - Rotational GUI elements
### A note on model compatibility:
The intent of this project was for me to learn graphics programming. As such, my main focus in creating it has been the 3D renderer and not the model loader. While most glTF models should work just fine, certain models may not display correctly or even outright crash the program. Model compatibility may improve with subsequent updates, but it is not a current priority for me.