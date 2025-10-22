#pragma once
#include <shaderc/shaderc.hpp>
#include "../Start.h"

#ifdef NDEBUG
#pragma comment(lib, "shaderc_combined.lib")
#else
#pragma comment(lib, "shaderc_sharedd.lib")
#endif

class f_compile_glsl_to_spv {
    struct includer: public shaderc::CompileOptions::IncluderInterface {
        struct result_t : shaderc_include_result {
            std::string filepath;   //用来存被包含文件的文件路径
            std::vector<char> code; //用来存被包含文件的内容
        };

        shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type, const char* requesting_source, size_t) override {
            auto& result = *(new result_t);
            auto& filepath = result.filepath;
            auto& code = result.code;

            filepath = requesting_source;
            size_t pos = filepath.rfind('/');
            if (pos == -1)
                pos = filepath.rfind('\\');
            filepath.replace(pos + 1 + filepath.begin(), filepath.end(), requested_source);
            load_file(filepath.c_str(), code);
            static_cast<shaderc_include_result&>(result) = {
                filepath.c_str(),
                filepath.length(),
                code.data(),
                code.size(),
                this
            };
            return &result;
        }

        void ReleaseInclude(shaderc_include_result* data) override {
            delete static_cast<result_t*>(data);
        }
    };
    static shaderc::Compiler compiler;
    static shaderc::CompileOptions options;
    shaderc::SpvCompilationResult result;

    // static function
    static void load_file(const char* filepath, std::vector<char>& binaries) {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file) {
            outstream << std::format("[ ShaderLoader ] ERROR\nFailed to open the file: {}\n", filepath) << std::endl;
            return;
        }
        size_t filesize = size_t(file.tellg());
        binaries.resize(filesize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(binaries.data()), filesize);
        file.close();
    }
public:
    f_compile_glsl_to_spv() {
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetIncluder(std::make_unique<includer>());
    }

    // Non-const Function
    std::span<const uint32_t> operator()(std::span<const char> code, const char* filepath, const char* entry = "main") {
        result = compiler.CompileGlslToSpv(code.data(), code.size(), shaderc_glsl_infer_from_source, filepath, entry,  options);
        outstream << result.GetErrorMessage() << std::endl;
        return { result.begin(), size_t(result.end() - result.begin()) * 4 };
    }

    std::span<const uint32_t> operator()(const char* filepath, const char* entry = "main") {
        std::vector<char> binaries;
        load_file(filepath, binaries);
        if (size_t filesize = binaries.size(); filesize > 0)
            return (*this)(binaries, filepath, entry);
        return {};
    }
};

inline std::filesystem::path get_shader_path(const std::string& shaderName)
{
    // 这将构建一个绝对路径，例如：
    // "D:/my/project/Shader/Texture.vert.spv"
    return G_PROJECT_ROOT / "Shader" / shaderName;
}
