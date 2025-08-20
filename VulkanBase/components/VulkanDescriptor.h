#pragma once
#include "VulkanBuffers.h"
#include "../../Start.h"
#include "../VulkanCore.h"

class VulkanDescriptorSetLayout {
    VkDescriptorSetLayout handle = VK_NULL_HANDLE;
public:
    VulkanDescriptorSetLayout() = default;
    VulkanDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo &create_info) {
        create(create_info);
    }
    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept {MoveHandle;}
    ~VulkanDescriptorSetLayout() {DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroyDescriptorSetLayout);}

    // getter
    DefineHandleTypeOperator;
    DefineAddressFunction;

    // non-const function
    result_t create(VkDescriptorSetLayoutCreateInfo &create_info) {
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        VkResult result = vkCreateDescriptorSetLayout(VulkanCore::get_singleton().get_vulkan_device().get_device(),&create_info,nullptr,&handle);
        if (result) {
            outstream << std::format("[ VulkanDescriptorSetLayout ] ERROR\nFailed to create a descriptor set layout!\nError code: {}\n", int32_t(result));
        }
        return result;
    }
};

class VulkanDescriptorSet {
    friend class VulkanDescriptorPool;
    VkDescriptorSet handle = VK_NULL_HANDLE;
public:
    VulkanDescriptorSet() = default;
    VulkanDescriptorSet(VulkanDescriptorSet &&other) noexcept {MoveHandle;}

    // getter
    DefineHandleTypeOperator;
    DefineAddressFunction;

    // const function
    // image info
    void write(array_ref<const VkDescriptorImageInfo> descriptor_infos,VkDescriptorType descriptor_type, uint32_t dst_binding = 0, uint32_t dst_array_element = 0) const {
        VkWriteDescriptorSet write_descriptor_set{
            .dstSet = handle,
            .dstBinding = dst_binding,
            .dstArrayElement = dst_array_element,
            .descriptorCount = uint32_t(descriptor_infos.Count()),
            .descriptorType = descriptor_type,
            .pImageInfo = descriptor_infos.Pointer()
        };
        update(write_descriptor_set);
    }

    // buffer info
    void write(array_ref<const VkDescriptorBufferInfo> descriptor_infos,VkDescriptorType descriptor_type, uint32_t dst_binding = 0, uint32_t dst_array_element = 0) const {
        VkWriteDescriptorSet write_descriptor_set{
            .dstSet = handle,
            .dstBinding = dst_binding,
            .dstArrayElement = dst_array_element,
            .descriptorCount = uint32_t(descriptor_infos.Count()),
            .descriptorType = descriptor_type,
            .pBufferInfo = descriptor_infos.Pointer()
        };
        update(write_descriptor_set);
    }

    // texel buffer info
    void write(array_ref<const VkBufferView> descriptor_infos,VkDescriptorType descriptor_type, uint32_t dst_binding = 0, uint32_t dst_array_element = 0) const {
        VkWriteDescriptorSet write_descriptor_set{
            .dstSet = handle,
            .dstBinding = dst_binding,
            .dstArrayElement = dst_array_element,
            .descriptorCount = uint32_t(descriptor_infos.Count()),
            .descriptorType = descriptor_type,
            .pTexelBufferView = descriptor_infos.Pointer()
        };
        update(write_descriptor_set);
    }

    void write(array_ref<const VulkanBufferView> descriptor_infos,VkDescriptorType descriptor_type, uint32_t dst_binding = 0, uint32_t dst_array_element = 0) const {
        write({descriptor_infos[0].Address(),descriptor_infos.Count()},descriptor_type,dst_binding,dst_array_element);
    }

    // static function
    static void update(array_ref<VkWriteDescriptorSet> writes, array_ref<VkCopyDescriptorSet> copies = {}) {
        for (auto& i : writes)
            i.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        for (auto& i : copies)
            i.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        vkUpdateDescriptorSets(
            VulkanCore::get_singleton().get_vulkan_device().get_device(), writes.Count(), writes.Pointer(), copies.Count(), copies.Pointer());
    }
};

class VulkanDescriptorPool {
    VkDescriptorPool handle = VK_NULL_HANDLE;
public:
    VulkanDescriptorPool() = default;
    VulkanDescriptorPool(VkDescriptorPoolCreateInfo &create_info) {
        create(create_info);
    }
    VulkanDescriptorPool(uint32_t max_set_count, array_ref<const VkDescriptorPoolSize> pool_sizes, VkDescriptorPoolCreateFlags flags = 0) {
        create(max_set_count,pool_sizes,flags);
    }
    VulkanDescriptorPool(VulkanDescriptorPool &&other) noexcept {MoveHandle;}
    ~VulkanDescriptorPool() {DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroyDescriptorPool);}

    // getter
    DefineHandleTypeOperator;
    DefineAddressFunction;

    // const function
    result_t allocate_sets(array_ref<VkDescriptorSet>sets, array_ref<const VkDescriptorSetLayout> layouts) const {
        if (sets.Count() != layouts.Count()) {
            if (sets.Count() < layouts.Count()) {
                outstream << std::format("[ VulkanDescriptorPool ] ERROR\nFor each descriptor set, must provide a corresponding layout!\n");
                return VK_RESULT_MAX_ENUM;
            }
            else outstream << std::format("[ VulkanDescriptorPool ] WARNING\nProvided layouts are more than sets!\n");
        }
        VkDescriptorSetAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = handle,
            .descriptorSetCount = uint32_t(sets.Count()),
            .pSetLayouts = layouts.Pointer()
        };
        VkResult result = vkAllocateDescriptorSets(VulkanCore::get_singleton().get_vulkan_device().get_device(),&allocate_info,sets.Pointer());
        if (result) {
            outstream << std::format("[ VulkanDescriptorPool ] ERROR\nFailed to allocate descriptor sets!\nError code: {}\n", int32_t(result));
        }
        return result;
    }
    result_t allocate_sets(array_ref<VkDescriptorSet>sets, array_ref<const VulkanDescriptorSetLayout> layouts) const {
        return allocate_sets(sets,{layouts[0].Address(),layouts.Count()});
    }
    result_t allocate_sets(array_ref<VulkanDescriptorSet>sets, array_ref<const VkDescriptorSetLayout> layouts) const {
        return allocate_sets({&sets[0].handle,sets.Count()},layouts);
    }
    result_t allocate_sets(array_ref<VulkanDescriptorSet>sets, array_ref<const VulkanDescriptorSetLayout> layouts) const {
        return allocate_sets({&sets[0].handle,sets.Count()},{layouts[0].Address(),layouts.Count()});
    }
    result_t free_sets(array_ref<const VkDescriptorSet> sets) const {
        VkResult result = vkFreeDescriptorSets(VulkanCore::get_singleton().get_vulkan_device().get_device(),handle,sets.Count(),sets.Pointer());
        memset(const_cast<VkDescriptorSet*>(sets.Pointer()), 0, sets.Count() * sizeof(VkDescriptorSet));
        return result;
    }
    result_t free_sets(array_ref<const VulkanDescriptorSet> sets) const {
        return free_sets({&sets[0].handle,sets.Count()});
    }

    // non-const function
    result_t create(VkDescriptorPoolCreateInfo &create_info) {
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        VkResult result = vkCreateDescriptorPool(VulkanCore::get_singleton().get_vulkan_device().get_device(),&create_info,nullptr,&handle);
        if (result) {
            outstream << std::format("[ VulkanDescriptorPool ] ERROR\nFailed to create a descriptor pool!\nError code: {}\n", int32_t(result));
        }
        return result;
    }
    result_t create(uint32_t max_set_count, array_ref<const VkDescriptorPoolSize> pool_sizes, VkDescriptorPoolCreateFlags flags = 0) {
        VkDescriptorPoolCreateInfo create_info = {
            .flags = flags,
            .maxSets = max_set_count,
            .poolSizeCount = uint32_t(pool_sizes.Count()),
            .pPoolSizes = pool_sizes.Pointer()
        };
        return create(create_info);
    }
};
