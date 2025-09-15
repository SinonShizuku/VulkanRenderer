# Vulkan Renderer

## 项目配置

### glm库
解压 https://github.com/g-truc/glm/releases 至`${SUBJECT_ROOT}/Submodule`下。

### glfw库
在 https://www.glfw.org/download.html 下载预编译glfw库，解压至`${SUBJECT_ROOT}/Submodule`下。

### vulkan SDK
在 https://vulkan.lunarg.com/sdk/home 下载Vulkan SDK，根据你的Vulkan路径配置CMakeLists.txt中VULKAN_SDK_PATH变量。