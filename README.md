# Model Viewer
A toy 3D model viewer I wrote to teach myself the fundamentals of realtime graphics programming.
### Controls:
- WSAD for movement
- Hold LSHIFT to increase movement speed
- Hold RMB to enable mouselook
- LMB to interact with GUI elements
### Current Features:
- 8X MSAA Antialiasing
- Fully-dynamic forward-rendered lighting
- Gamma-Correction and Tone-Mapping using [Luminance-based Extended Reinhard's Algorithm](https://64.github.io/tonemapping/#extended-reinhard-luminance-tone-map)
- Support for Albedo, Normal, Metalness, Roughness, and Ambient Occlusion texture mapping
- Support for both Blinn-Phong and Physically Based shading
### Planned Features:
- Screen-Space Ambient Occlusion
- Shadowmapping with Percentage-Closer Filtering
- Image-Based Lighting
- Emissive Materials / Bloom
- Weighted Blended Approximate OIT
### Dependencies:
- [GLFW](https://github.com/glfw/glfw) - Window context and input handling
- [GLAD](https://github.com/Dav1dde/glad) - OpenGL function bindings
- [GLM](https://github.com/g-truc/glm) - CPU-side linear algebra computations
- [STB](https://github.com/nothings/stb) - Image loading
- [Assimp](https://github.com/assimp/assimp) - 3D model loading
- [Dear ImGui](https://github.com/ocornut/imgui) - GUI elements
### Additional Credits:
- Skybox texture taken from https://github.com/rpgwhitelock/AllSkyFree_Godot and converted to cubemap using https://jaxry.github.io/panorama-to-cubemap/
### A note on model compatibility:
The intent of this project was for me to learn graphics programming. As such, my main focus in creating it has been the 3D renderer and not the model loader, and thus minimal care has been taken to ensure a high level of compatibility with a divserse range of 3D models. In particular, models with transparencies, emissive materials, untextured meshes, or meshes composed of nontriangle primitives may not display correctly or even outright crash the program. Additionally, the program has not been tested with files that are not of the glTF file format, though theoretically any format supported by Assimp should work. Model compatibility may improve with subsequent updates, but it is not a current priority for me. For demonstration purposes, [this backpack model](https://sketchfab.com/3d-models/survival-guitar-backpack-799f8c4511f84fab8c3f12887f7e6b36) in glTF format loads fine and does a good job highlighting the features of the program.