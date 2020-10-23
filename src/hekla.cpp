#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#define VK_CHECK(call) \
    do { \
        VkResult result_ = call; \
        assert(result_ == VK_SUCCESS); \
    } while(0)

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount)
{
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            printf("Found discrete GPU: %s", props.deviceName);
            return physicalDevices[i];
        }
    }

    if (physicalDeviceCount > 0)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[0], &props);

        printf("Picking fallback GPU: %s", props.deviceName);
        return physicalDevices[0];
    }

    printf("No GPU found!");
    return 0;
}

VkInstance createInstance()
{
    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_VERSION_1_2;

    VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
    const char* debugLayers[] =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
#endif

    const char* extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    };
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

    VkInstance instance = 0;
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

    return instance;
}

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice)
{
    float queuePriorities[] = { 1.0f };

    VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;

    VkDevice device = 0;
    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

    return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    createInfo.hinstance = GetModuleHandle(0);
    createInfo.hwnd = glfwGetWin32Window(window);
    VkSurfaceKHR surface = 0;
    VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
    return surface;
#else
#error Unsupported platform
#endif
}

int main()
{
    int rc = glfwInit();
    assert(rc);

    VkInstance instance = createInstance();
    assert(instance);

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
    assert(physicalDevice);

    VkDevice device = createDevice(instance, physicalDevice);
    assert(device);

    GLFWwindow* window = glfwCreateWindow(1024,768, "hekla", nullptr, nullptr);
    assert(window);

    VkSurfaceKHR surface = createSurface(instance, window);
    assert(surface);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

}