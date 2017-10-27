#pragma once

#include <string>
#include <tuple>
#include <vector>

#include <tinyxml2.hxx>

constexpr std::uint8_t POSITIONS_PRESENT  = 0b00000001;
constexpr std::uint8_t TEX_COORDS_PRESENT = 0b00000010;
constexpr std::uint8_t NORMALS_PRESENT    = 0b00000100;
constexpr std::uint8_t COLORS_PRESENT     = 0b00001000;

struct Vector2 {
    float x;
    float y;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

//struct Animation {
//};

struct Mesh {
    std::uint8_t present_attributes{};
    std::vector<Vector3> positions;
    std::vector<Vector2> tex_coords;
    std::vector<Vector3> normals;
    std::vector<Vector3> colors;
    std::vector<std::size_t> position_indices;
    std::vector<std::size_t> tex_coords_indices;
    std::vector<std::size_t> normal_indices;
    std::vector<std::size_t> color_indices;
    //std::vector<Animation>   animations;
};

struct Obm {
    std::array<char, 4> header;         // Should be 'OBMF'.
    uint8_t             meshes_count;
    std::vector<Mesh>   meshes;
};

int convert(const std::string_view input_file_name, const std::string_view output_file_name);

std::vector<Mesh> load_meshes(tinyxml2::XMLElement* collada_root_node);
Mesh load_mesh(tinyxml2::XMLNode* mesh_node, const std::string_view mesh_id);
bool load_positions(std::vector<Vector3>& target, tinyxml2::XMLElement* positions_node);
bool load_normals(std::vector<Vector3>& target, tinyxml2::XMLElement* normals_node);
bool load_tex_coords(std::vector<Vector2>& target, tinyxml2::XMLElement* tex_coords_node);
bool load_colors(std::vector<Vector3>& target, tinyxml2::XMLElement* colors_node);
bool check_present_attributes_and_load_indices(tinyxml2::XMLElement* indices_node, const std::size_t count,
        uint8_t& attribs, std::vector<std::size_t>& position_indices, std::vector<std::size_t>& normal_indices,
        std::vector<std::size_t>& tex_coords_indices, std::vector<std::size_t>& color_indices);

bool write_meshes(const std::string_view file_name, const std::vector<Mesh>& meshes);
