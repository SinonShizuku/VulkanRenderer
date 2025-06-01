#include "VulkanAppLauncher.h"

int main() {
    VkExtent2D window_size = { 1920, 1080 };
    VulkanAppLauncher::getSingleton(window_size).run();

    return 0;
}
