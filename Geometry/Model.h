#pragma once
#include<vector>



#ifdef WIN32
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#else
#include<glm1_0/glm.hpp>
#include<glm1_0/gtc/matrix_transform.hpp>
#endif

#include "../Start.h"
#include "../VulkanBase/components/VulkanMemory.h"
#include "../VulkanBase/components/VulkanTexture.h"
#include "../VulkanBase/components/VulkanDescriptor.h"
#include "../VulkanBase/components/VulkanSampler.h"

#include "tiny_gltf.h"

class VulkanglTFModel {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 color;
    };

    VulkanVertexBuffer vertices;
    struct {
        int count;
        VulkanIndexBuffer index_buffer;
    } indices;

    struct Node;
    // 图元
    struct Primitive {
        uint32_t first_index;
        uint32_t index_count;
        int32_t material_index;
    };

    struct Mesh {
        std::vector<Primitive> primitives;
    };

    // A node represents an object in the glTF scene graph
    struct Node {
        Node* parent;
        std::vector<Node*> children;
        Mesh mesh;
        glm::mat4 matrix;
        ~Node() {
            for (auto& child : children) {
                delete child;
            }
        }
    };

    // A glTF material stores information in e.g. the texture that is attached to it and colors
    struct Material {
        glm::vec4 base_color_factor = glm::vec4(1.0f);
        uint32_t base_color_texture_index;
    };

    // Contains the texture for a single glTF image
    // Images may be reused by texture objects and are as such separated
    struct Image {
        VulkanTexture2D texture;
        // We also store (and create) a descriptor set that's used to access this texture from the fragment shader
        VulkanDescriptorSet descriptor_set;
        VulkanSampler sampler;
    };

    struct Texture {
        uint32_t image_index;
    };

    std::vector<Image> images;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Node*> nodes;

    ~VulkanglTFModel() {
        for (auto node : nodes) {
            delete node;
        }
    }

    void load_images(tinygltf::Model& input) {
        images.resize(input.images.size());
        for (size_t i = 0; i < input.images.size(); i++) {
            tinygltf::Image& gltf_image = input.images[i];
            const uint8_t* buffer = &gltf_image.image[0];

            VkFormat format_initial;
            if (gltf_image.component == 3) {
                format_initial = VK_FORMAT_R8G8B8_UNORM;
            } else if (gltf_image.component == 4) {
                format_initial = VK_FORMAT_R8G8B8A8_UNORM;
            } else {
                outstream << std::format("[ Model ] ERROR\nUnsupported image component count : {}\n", gltf_image.component);
                format_initial = VK_FORMAT_R8G8B8A8_UNORM;
            }
            VkFormat format_final = VK_FORMAT_R8G8B8A8_UNORM;
            images[i].texture.create(
                buffer,
                { (uint32_t)gltf_image.width, (uint32_t)gltf_image.height },
                format_initial,
                format_final,
                true
            );
        }
    }

    void load_textures(tinygltf::Model& input) {
        textures.resize(input.textures.size());
        for (size_t i = 0; i < input.textures.size(); i++) {
            textures[i].image_index = input.textures[i].source;
        }
    }

    void load_materials(tinygltf::Model& input) {
        materials.resize(input.materials.size());
        for (size_t i = 0; i < input.materials.size(); i++) {
            tinygltf::Material& gltf_material = input.materials[i];
            if (gltf_material.values.find("baseColorFactor") != gltf_material.values.end()) {
                materials[i].base_color_factor = glm::make_vec4(gltf_material.values["baseColorFactor"].ColorFactor().data());
            }
            if (gltf_material.values.find("baseColorTexture") != gltf_material.values.end()) {
                materials[i].base_color_texture_index = gltf_material.values["baseColorTexture"].TextureIndex();
            }
        }
    }

    void load_node(const tinygltf::Node& input_node, const tinygltf::Model& input,
        VulkanglTFModel::Node* parent, std::vector<uint32_t>& index_buffer, std::vector<VulkanglTFModel::Vertex>& vertex_buffer) {
        auto* node = new VulkanglTFModel::Node{};
        node->matrix = glm::mat4(1.0f);
        node->parent = parent;

        // Get the local node matrix
        if (input_node.translation.size() == 3) {
            node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(input_node.translation.data())));
        }
        if (input_node.rotation.size() == 4) {
            glm::quat q = glm::make_quat(input_node.rotation.data());
            node->matrix *= glm::mat4(q);
        }
        if (input_node.scale.size() == 3) {
            node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(input_node.scale.data())));
        }
        if (input_node.matrix.size() == 16) {
            node->matrix = glm::make_mat4x4(input_node.matrix.data());
        }

        // load the node's children
        if (!input_node.children.empty()) {
            for (int i : input_node.children) {
                load_node(input.nodes[i], input, node, index_buffer, vertex_buffer);
            }
        }

        // load vertices and indices from the buffers
        if (input_node.mesh > -1) {
            const tinygltf::Mesh mesh = input.meshes[input_node.mesh];
            for (size_t i = 0; i < mesh.primitives.size(); i++) {
                const tinygltf::Primitive& gltf_primitive = mesh.primitives[i];
                uint32_t first_index = static_cast<uint32_t>(index_buffer.size());
                uint32_t vertex_start = static_cast<uint32_t>(vertex_buffer.size());
                uint32_t index_count = 0;
                // Vertices
                {
                    const float* position_buffer = nullptr;
                    const float* normals_buffer = nullptr;
                    const float* tex_coords_buffer = nullptr;
                    size_t vertex_count = 0;

                    if (gltf_primitive.attributes.find("POSITION") != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[gltf_primitive.attributes.find("POSITION")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        position_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        vertex_count = accessor.count;
                    }
                    if (gltf_primitive.attributes.find("NORMAL") != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[gltf_primitive.attributes.find("NORMAL")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        normals_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                    if (gltf_primitive.attributes.find("TEXCOORD_0") != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor &accessor = input.accessors[gltf_primitive.attributes.find("TEXCOORD_0")->second];
                        const tinygltf::BufferView &view = input.bufferViews[accessor.bufferView];
                        tex_coords_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }

                    // Append data to model's vertex buffer
                    for (size_t v = 0; v < vertex_count; v++) {
                        Vertex vert{};
                        vert.pos = glm::vec4(glm::make_vec3(&position_buffer[v * 3]), 1.0f);
                        vert.normal = glm::normalize(glm::vec3(normals_buffer ? glm::make_vec3(&normals_buffer[v * 3]) : glm::vec3(0.0f)));
                        vert.uv = tex_coords_buffer ? glm::make_vec2(&tex_coords_buffer[v * 2]) : glm::vec3(0.0f);
                        vert.color = glm::vec3(1.0f);
                        vertex_buffer.push_back(vert);
                    }
                }

                // Indices
                {
                    const tinygltf::Accessor& accessor = input.accessors[gltf_primitive.indices];
                    const tinygltf::BufferView& buffer_view = input.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = input.buffers[buffer_view.buffer];

                    index_count += static_cast<uint32_t>(accessor.count);

                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                index_buffer.push_back(buf[index] + vertex_start);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                index_buffer.push_back(buf[index] + vertex_start);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                index_buffer.push_back(buf[index] + vertex_start);
                            }
                            break;
                        }
                        default:
                            outstream << std::format("[ Model ] ERROR\nIndex component type {} not supported!\n", accessor.componentType);
                            return;
                    }
                }
                Primitive primitive{};
                primitive.first_index = first_index;
                primitive.index_count = index_count;
                primitive.material_index = gltf_primitive.material;
                node->mesh.primitives.push_back(primitive);
            }
        }

        if (parent) parent->children.push_back(node);
        else nodes.push_back(node);
    }
};




