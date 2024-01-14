A toy 3D model viewer I wrote to teach myself the fundamentals of realtime graphics programming.
### Controls:
- **Movement** - `WSAD`, hold `LSHIFT` to increase speed
- **Enable mouselook** - Hold `Right Click`
- **Interact with GUI** - Use `Left Click` when not in mouselook mode
### Current Features
- 8X MSAA Antialiasing
- Fully-dynamic forward-rendered lighting
- Gamma-Correction and Tone-Mapping
- Support for Albedo, Normal, Metalness, Roughness, and Ambient Occlusion texture mapping
- Physically Based Shading
- Image-based Global Illumination
- Shadowmapping with Jittered Poisson Disk PCF
### Planned Features:
- Screen-Space Ambient Occlusion
- Emissive Materials / Bloom
- Weighted Blended Approximate OIT
### Dependencies:
- **[GLFW](https://github.com/glfw/glfw)** - Window context and input handling
- **[GLAD](https://github.com/Dav1dde/glad)** - OpenGL function bindings
- **[GLM](https://github.com/g-truc/glm)** - CPU-side linear algebra computations
- **[STB](https://github.com/nothings/stb)** - Image loading
- **[Assimp](https://github.com/assimp/assimp)** - 3D model loading
- **[Dear ImGui](https://github.com/ocornut/imgui)** - GUI elements
### Custom Skyboxes:
Neither the uploaded source code nor precompiled release package contains a skybox texture as high-resolution hdr files can easily take up hundreds of megabytes of storage. However, any equirectangular
environment projection in the `.hdr` file format should work as a skybox for the model viewer. For the model viewer to recognize
an hdr file to be used, the file must be called `skybox.hdr` and placed in the same directory as the executable. The website [polyhaven.com](polyhaven.com/hdris) is a great resource for finding free hdr skybox textures. I recommend using textures with a 16k resolution as I have found smaller textures to appear blurry and OpenGL does not seem to like larger textures.
### A note on model compatibility:
The intent of this project was for me to learn graphics programming. As such, my main focus in creating it has been the 3D renderer and not the model loader, and thus minimal care has been taken to ensure a high level of compatibility with a divserse range of 3D models. In particular, models with transparencies, emissive materials, untextured meshes, or meshes composed of nontriangle primitives may not display correctly or even outright crash the program. Additionally, the program has not been tested with files that are not of the glTF file format, though theoretically any format supported by Assimp should work. Model compatibility may improve with subsequent updates, but it is not a current priority for me. For demonstration purposes, [this backpack model](https://sketchfab.com/3d-models/survival-guitar-backpack-799f8c4511f84fab8c3f12887f7e6b36) in glTF format loads fine and does a good job highlighting the features of the program.