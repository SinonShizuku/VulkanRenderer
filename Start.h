#pragma once

// includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <cstring>
#include <functional>
#include <thread>

// GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//如果你惯用左手坐标系，在此定义GLM_FORCE_LEFT_HANDED
#ifdef WIN32
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#else
#include<glm1_0/glm.hpp>
#include<glm1_0/gtc/matrix_transform.hpp>
#endif

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib") //链接编译所需的静态库

// stb_image.h
// #include <stb/stb_image.h>

// Vulkan
#ifdef _WIN32                        //考虑平台是Windows的情况（请自行解决其他平台上的差异）
#define VK_USE_PLATFORM_WIN32_KHR    //在包含vulkan.h前定义该宏，会一并包含vulkan_win32.h和windows.h
#define NOMINMAX                     //定义该宏可避免windows.h中的min和max两个宏与标准库中的函数名冲突
#pragma comment(lib, "vulkan-1.lib") //链接编译所需的静态存根库
#endif
#include <vulkan/vulkan.h>



// global defines
#ifndef NDEBUG
#define ENABLE_DEBUG_MESSENGER true
#else
#define ENABLE_DEBUG_MESSENGER false
#endif

// 封装Vulkan相关类的一些函数
#define DestroyHandleBy(device,Func) if (handle) { Func(device, handle, nullptr); handle = VK_NULL_HANDLE; }
#define MoveHandle handle = other.handle; other.handle = VK_NULL_HANDLE;
#define DefineMoveAssignmentOperator(type) type& operator=(type&& other) { this->~type(); MoveHandle; return *this; }
#define DefineHandleTypeOperator operator decltype(handle)() const { return handle; }
#define DefineAddressFunction const decltype(handle)* Address() const { return &handle; }

// 分割（函数块内）部分执行
#define ExecuteOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }



// global configs
constexpr VkExtent2D default_window_size = { 1920, 1080 };





// 一些封装内容
// 返回值封装
//情况1：根据函数返回值确定是否抛异常
#ifdef VK_RESULT_THROW
class result_t {
    VkResult result;
public:
    static void(*callback_throw)(VkResult);
    result_t(VkResult result) :result(result) {}
    result_t(result_t&& other) noexcept :result(other.result) { other.result = VK_SUCCESS; }
    ~result_t() noexcept(false) {
        if (uint32_t(result) < VK_RESULT_MAX_ENUM)
            return;
        if (callback_throw)
            callback_throw(result);
        throw result;
    }
    operator VkResult() {
        VkResult result = this->result;
        this->result = VK_SUCCESS;
        return result;
    }
};
inline void(*result_t::callback_throw)(VkResult);

//情况2：若抛弃函数返回值，让编译器发出警告
#elif defined VK_RESULT_NODISCARD
struct [[nodiscard]] result_t {
    VkResult result;
    result_t(VkResult result) :result(result) {}
    operator VkResult() const { return result; }
};
//在本文件中关闭弃值提醒（因为我懒得做处理）
#pragma warning(disable:4834)
#pragma warning(disable:6031)

//情况3：啥都不干
#else
using result_t = VkResult;
#endif

// array_ref封装
template<typename T>
class array_ref {
    T* const pArray = nullptr;
    size_t count = 0;
public:
    //从空参数构造，count为0
    array_ref() = default;
    //从单个对象构造，count为1
    array_ref(T& data) :pArray(&data), count(1) {}
    //从顶级数组构造
    template<size_t elementCount>
    array_ref(T(&data)[elementCount]) : pArray(data), count(elementCount) {}
    //从指针和元素个数构造
    array_ref(T* pData, size_t elementCount) :pArray(pData), count(elementCount) {}
    //复制构造，若T带const修饰，兼容从对应的无const修饰版本的arrayRef构造
    //24.01.07 修正因复制粘贴产生的typo：从pArray(&other)改为pArray(other.Pointer())
    array_ref(const array_ref<std::remove_const_t<T>>& other) :pArray(other.Pointer()), count(other.Count()) {}
    //Getter
    T* Pointer() const { return pArray; }
    size_t Count() const { return count; }
    //Const Function
    T& operator[](size_t index) const { return pArray[index]; }
    T* begin() const { return pArray; }
    T* end() const { return pArray + count; }
    //Non-const Function
    //禁止复制/移动赋值
    array_ref& operator=(const array_ref&) = delete;
};

// 输出方式封装
inline auto& outstream = std::cout;//不是constexpr，因为std::cout具有外部链接

// 封装闭区间“在内”
template<std::signed_integral T>
constexpr bool between_closed(T min, T num, T max) {
    return ((num - min) | (max - num)) >= 0;
}
