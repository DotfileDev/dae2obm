#include <dae2obm.hxx>

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <sstream>

int convert(const std::string_view input_file_name, [[maybe_unused]] const std::string_view output_file_name) {
    tinyxml2::XMLDocument collada_file{};
    const auto load_file_error = collada_file.LoadFile(input_file_name.data());
    if(load_file_error != tinyxml2::XMLError::XML_SUCCESS) {
        std::cerr << "Failed to open collada source file \"" << input_file_name << "\"; exiting...\n";
        return 1;
    }
    auto collada_root_node = collada_file.FirstChildElement("COLLADA");
    if(collada_root_node == nullptr) {
        std::cerr << "Collada root node was not found; exiting...\n";
        return 2;
    }
    const auto meshes = load_meshes(collada_root_node);
    // TODO: Here we'll save all meshes to *.obm file :)
    std::cout << "Success!\n";
    return 0;
}

std::vector<Mesh> load_meshes(tinyxml2::XMLElement* collada_root_node) {
    std::vector<Mesh> meshes{};
    auto geometry = collada_root_node->FirstChildElement("library_geometries")->FirstChildElement("geometry");
    if(geometry == nullptr) {
        std::cout << "No geometries found in geometries library; exiting...\n";
        std::exit(3);
    }
    while(geometry != nullptr) {
        std::cout << "Geometry found!\n";
        const auto mesh_id = geometry->Attribute("id", nullptr);
        const auto mesh_node = geometry->FirstChildElement("mesh");
        if(mesh_node == nullptr) {
            std::cout << "Geometry doesn't contain \"mesh\" node; exiting...";
            std::exit(4);
        }
        std::cout << "Mesh id: " << mesh_id << '\n';
        meshes.emplace_back(load_mesh(mesh_node, mesh_id));
        geometry = geometry->NextSiblingElement();
    }
    return meshes;
}

Mesh load_mesh(tinyxml2::XMLNode* mesh_node, const std::string_view mesh_id) {
    Mesh mesh{};
    auto vertex_attributes_node = mesh_node->FirstChildElement("source");
    while(vertex_attributes_node != nullptr) {
        const std::string vertex_attributes_node_id{vertex_attributes_node->Attribute("id")};
        const std::string attribute_name{mesh_id};
        if(vertex_attributes_node_id == attribute_name + "-positions") {
            load_positions(mesh.positions, vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-normals") {
            load_normals(mesh.normals, vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-map-0") {
            load_tex_coords(mesh.tex_coords, vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-colors-Col") {
            load_colors(mesh.colors, vertex_attributes_node);
        } else {
            std::cerr << "Error: Unknown mesh attribute: " << vertex_attributes_node_id << ".\n";
            std::exit(5);
        }
        vertex_attributes_node = vertex_attributes_node->NextSiblingElement("source");
    }
    // TODO: Indices are located in <polylist><vcount>.
    return mesh;
}

bool load_positions(std::vector<Vector3>& target, tinyxml2::XMLElement* positions_node) {
    // TODO: Collada provides additional info about vertices in <technique_common>.
    const auto positions_array_node{positions_node->FirstChildElement("float_array")};
    const std::size_t positions_count{positions_node->UnsignedAttribute("count")};
    std::stringstream positions_array{positions_array_node->GetText()};
    for(std::size_t i{positions_count / 3}; i < positions_count / 3; ++i) {
        auto& position = target.emplace_back();
        positions_array >> position.x >> position.y >> position.z;
    }
    std::cout << "Log:   All positions loaded.\n";
    return true;
}

bool load_normals(std::vector<Vector3>& target, tinyxml2::XMLElement* normals_node) {
    const auto normals_array_node{normals_node->FirstChildElement("float_array")};
    const std::size_t normals_count{normals_node->UnsignedAttribute("count")};
    std::stringstream normals_array{normals_array_node->GetText()};
    for(std::size_t i{normals_count / 3}; i < normals_count / 3; ++i) {
        auto& normal = target.emplace_back();
        normals_array >> normal.x >> normal.y >> normal.z;
    }
    std::cout << "Log:   All normals loaded.\n";
    return true;
}

bool load_tex_coords(std::vector<Vector2>& target, tinyxml2::XMLElement* tex_coords_node) {
    const auto tex_coords_array_node{tex_coords_node->FirstChildElement("float_array")};
    const std::size_t tex_coords_count{tex_coords_node->UnsignedAttribute("count")};
    std::stringstream tex_coords_array{tex_coords_array_node->GetText()};
    for(std::size_t i{tex_coords_count / 3}; i < tex_coords_count / 3; ++i) {
        auto& tex_coords = target.emplace_back();
        tex_coords_array >> tex_coords.x >> tex_coords.y;
    }
    std::cout << "Log:   All texture coords loaded.\n";
    return true;

}

bool load_colors(std::vector<Vector3>& target, tinyxml2::XMLElement* colors_node) {
    const auto colors_array_node{colors_node->FirstChildElement("float_array")};
    const std::size_t colors_count{colors_node->UnsignedAttribute("count")};
    std::stringstream colors_array{colors_array_node->GetText()};
    for(std::size_t i{colors_count / 3}; i < colors_count / 3; ++i) {
        auto& color = target.emplace_back();
        colors_array >> color.x >> color.y >> color.z;
    }
    std::cout << "Log:   All colors loaded.\n";
    return true;
}

int main(const int argc, const char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: dae2obm [src.dae] [dest.obm]\n";
        return 0;
    }
    return convert(argv[1], argv[2]);
}
