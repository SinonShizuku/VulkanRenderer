#include "Launcher/VulkanAppLauncher.h"

int main() {
    VulkanAppLauncher::getSingleton(default_window_size).run();
    VulkanColorAttachment a;
    return 0;
}
