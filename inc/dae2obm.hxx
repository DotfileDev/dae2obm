#pragma once

#include <string>
#include <tuple>
#include <vector>

#include <tinyxml2.hxx>

// TODO: This is bad. Get rid of these.
#define POSITIONS_PRESENT  0b00000001
#define TEX_COORDS_PRESENT 0b00000010
#define NORMALS_PRESENT    0b00000100
#define COLORS_PRESENT     0b00001000

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
