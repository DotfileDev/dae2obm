#include <dae2obm.hxx>

#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <sstream>

int convert(const std::string_view input_file_name, const std::string_view output_file_name) {
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
    const auto write_success = write_meshes(output_file_name, meshes);
    if(!write_success) {
        std::cerr << "Failed to write to file \"" << output_file_name << "\"; exiting...";
        return 7;
    }
    std::cout << "Log:   Model converted with no errors.\n";
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
        const auto mesh_id = geometry->Attribute("id", nullptr);
        const auto mesh_node = geometry->FirstChildElement("mesh");
        if(mesh_node == nullptr) {
            std::cout << "Geometry doesn't contain \"mesh\" node; exiting...";
            std::exit(4);
        }
        std::cout << "Log:   Current mesh id: " << mesh_id << '\n';
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
    const auto indices_node = mesh_node->FirstChildElement("polylist");
    if(indices_node == nullptr) {
        std::cerr << "Error: Indices node was not found; exiting...\n";
        std::exit(6);
    }
    const std::size_t indices_count = indices_node->UnsignedAttribute("count");
    check_present_attributes_and_load_indices(indices_node, indices_count, mesh.present_attributes,
            mesh.position_indices, mesh.tex_coords_indices, mesh.normal_indices, mesh.color_indices);
    return mesh;
}

bool load_positions(std::vector<Vector3>& target, tinyxml2::XMLElement* positions_node) {
    // TODO: Collada provides additional info about vertices in <technique_common>.
    const auto positions_array_node{positions_node->FirstChildElement("float_array")};
    const std::size_t positions_count{positions_array_node->UnsignedAttribute("count")};
    std::stringstream positions_array{positions_array_node->GetText()};
    for(std::size_t i{}; i < positions_count / 3; ++i) {
        auto& position = target.emplace_back();
        positions_array >> position.x >> position.y >> position.z;
    }
    std::cout << "Log:   All positions loaded.\n";
    return true;
}

bool load_normals(std::vector<Vector3>& target, tinyxml2::XMLElement* normals_node) {
    const auto normals_array_node{normals_node->FirstChildElement("float_array")};
    const std::size_t normals_count{normals_array_node->UnsignedAttribute("count")};
    std::stringstream normals_array{normals_array_node->GetText()};
    for(std::size_t i{}; i < normals_count / 3; ++i) {
        auto& normal = target.emplace_back();
        normals_array >> normal.x >> normal.y >> normal.z;
    }
    std::cout << "Log:   All normals loaded.\n";
    return true;
}

bool load_tex_coords(std::vector<Vector2>& target, tinyxml2::XMLElement* tex_coords_node) {
    const auto tex_coords_array_node{tex_coords_node->FirstChildElement("float_array")};
    const std::size_t tex_coords_count{tex_coords_array_node->UnsignedAttribute("count")};
    std::stringstream tex_coords_array{tex_coords_array_node->GetText()};
    for(std::size_t i{}; i < tex_coords_count / 2; ++i) {
        auto& tex_coords = target.emplace_back();
        tex_coords_array >> tex_coords.x >> tex_coords.y;
    }
    std::cout << "Log:   All texture coords loaded.\n";
    return true;

}

bool load_colors(std::vector<Vector3>& target, tinyxml2::XMLElement* colors_node) {
    const auto colors_array_node{colors_node->FirstChildElement("float_array")};
    const std::size_t colors_count{colors_array_node->UnsignedAttribute("count")};
    std::stringstream colors_array{colors_array_node->GetText()};
    for(std::size_t i{}; i < colors_count / 3; ++i) {
        auto& color = target.emplace_back();
        colors_array >> color.x >> color.y >> color.z;
    }
    std::cout << "Log:   All colors loaded.\n";
    return true;
}

bool check_present_attributes_and_load_indices(tinyxml2::XMLElement* indices_node, const std::size_t count,
        uint8_t& attribs, std::vector<std::size_t>& position_indices, std::vector<std::size_t>& normal_indices,
        std::vector<std::size_t>& tex_coords_indices, std::vector<std::size_t>& color_indices) {
    // TODO: Check if attributes are present.
    attribs = POSITIONS_PRESENT | TEX_COORDS_PRESENT | NORMALS_PRESENT | COLORS_PRESENT;
    std::stringstream indices_stream{indices_node->FirstChildElement("p")->GetText()};
    for(std::size_t i{}; i < count / 4; ++i) {
        auto& posi = position_indices.emplace_back();
        auto& txci = tex_coords_indices.emplace_back();
        auto& nrmi = normal_indices.emplace_back();
        auto& clri = color_indices.emplace_back();
        indices_stream >> posi >> txci >> nrmi >> clri;
    }
    return true;
}

bool write_meshes(const std::string_view file_name, const std::vector<Mesh>& meshes) {
    std::fstream output_file{file_name.data(), std::ios::out | std::ios::binary | std::ios::trunc};
    if(!output_file.good()) {
        return false;
    }
    const auto meshes_count = static_cast<std::uint8_t>(meshes.size());
    output_file.write("OBMF", 4)
            .write(reinterpret_cast<const char*>(&meshes_count), 1);
    for(const auto& mesh : meshes) {
        const auto positions_count = static_cast<std::uint32_t>(mesh.positions.size());
        const auto attributes_count = static_cast<std::uint32_t>(mesh.tex_coords.size());
        const auto positions_indices_count = static_cast<std::uint32_t>(mesh.position_indices.size());
        const auto attributes_indices_count = static_cast<std::uint32_t>(mesh.tex_coords.size());
        output_file.write(reinterpret_cast<const char*>(&mesh.present_attributes), sizeof(mesh.present_attributes))
                .write(reinterpret_cast<const char*>(&positions_count), sizeof(positions_count))
                .write(reinterpret_cast<const char*>(&attributes_count), sizeof(attributes_count))
                .write(reinterpret_cast<const char*>(&positions_indices_count), sizeof(positions_indices_count))
                .write(reinterpret_cast<const char*>(&attributes_indices_count), sizeof(attributes_indices_count));
        for(const auto& position : mesh.positions) {
            output_file.write(reinterpret_cast<const char*>(&position.x), sizeof(float))
                    .write(reinterpret_cast<const char*>(&position.y), sizeof(float))
                    .write(reinterpret_cast<const char*>(&position.z), sizeof(float));
        }
        for(const auto& tex_coords : mesh.tex_coords) {
            output_file.write(reinterpret_cast<const char*>(&tex_coords.x), sizeof(float))
                    .write(reinterpret_cast<const char*>(&tex_coords.y), sizeof(float));
        }
        for(const auto& normal : mesh.normals) {
            output_file.write(reinterpret_cast<const char*>(&normal.x), sizeof(float))
                    .write(reinterpret_cast<const char*>(&normal.y), sizeof(float))
                    .write(reinterpret_cast<const char*>(&normal.z), sizeof(float));
        }
        for(const auto& color : mesh.colors) {
            output_file.write(reinterpret_cast<const char*>(&color.x), sizeof(float))
                    .write(reinterpret_cast<const char*>(&color.y), sizeof(float))
                    .write(reinterpret_cast<const char*>(&color.z), sizeof(float));
        }
        for(const auto& position_index : mesh.position_indices) {
            const auto index = static_cast<std::uint32_t>(position_index);
            output_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
        for(const auto& tex_coords_index : mesh.tex_coords_indices) {
            const auto index = static_cast<std::uint32_t>(tex_coords_index);
            output_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
        for(const auto& normal_index : mesh.normal_indices) {
            const auto index = static_cast<std::uint32_t>(normal_index);
            output_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
        for(const auto& color_index : mesh.color_indices) {
            const auto index = static_cast<std::uint32_t>(color_index);
            output_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
    }
    return true;
}

int main(const int argc, const char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: dae2obm [src.dae] [dest.obm]\n";
        return 0;
    }
    return convert(argv[1], argv[2]);
}
