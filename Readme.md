# Vulkan Renderer

## 项目配置

### glm
解压 https://github.com/g-truc/glm/releases 至`${SUBJECT_ROOT}/Submodule`下。

### glfw
在 https://www.glfw.org/download.html 下载预编译glfw库，解压至`${SUBJECT_ROOT}/Submodule`下。

### Vulkan SDK
在 https://vulkan.lunarg.com/sdk/home 下载Vulkan SDK，根据你的Vulkan路径配置CMakeLists.txt中VULKAN_SDK_PATH变量。

### ImGui
在 https://github.com/ocornut/imgui/releases 获取imgui库，解压至`${SUBJECT_ROOT}/Submodule`下。

### stb_image.h
将单头文件库stb_image.h配置到你的环境中，并自行调整stb_image_implementation.cpp中的引用。