#pragma once

#include <array>
#include <fstream>
#include <sstream>
#include <string>
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

struct Mesh {
    std::uint8_t present_attributes{};
    std::vector<Vector3> positions;
    std::vector<Vector2> tex_coords;
    std::vector<Vector3> normals;
    std::vector<Vector3> colors;
    std::vector<std::uint32_t> position_indices;
    std::vector<std::uint32_t> tex_coords_indices;
    std::vector<std::uint32_t> normal_indices;
    std::vector<std::uint32_t> color_indices;
};

int convert(const std::string_view input_file_name, const std::string_view output_file_name);

std::vector<Mesh> load_meshes(tinyxml2::XMLElement* collada_root_node);
Mesh load_mesh(tinyxml2::XMLNode* mesh_node, const std::string_view mesh_id);
bool check_present_attributes_and_load_indices(tinyxml2::XMLElement* indices_node, const std::size_t count,
        uint8_t& attribs, std::vector<std::uint32_t>& position_indices, std::vector<std::uint32_t>& normal_indices,
        std::vector<std::uint32_t>& tex_coords_indices, std::vector<std::uint32_t>& color_indices);

std::vector<Vector2> load_vector_vector2_from_xml_node(const tinyxml2::XMLElement* node);
std::vector<Vector3> load_vector_vector3_from_xml_node(const tinyxml2::XMLElement* node);

Vector2 load_vector2_from_sstream(std::stringstream& stream);
Vector3 load_vector3_from_sstream(std::stringstream& stream);

bool write_meshes(const std::string_view file_name, const std::vector<Mesh>& meshes);
void write_vector2(std::fstream& file, const Vector2& vec);
void write_vector3(std::fstream& file, const Vector3& vec);
