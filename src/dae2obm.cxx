#include <dae2obm.hxx>

#include <cstdlib>
#include <cstring>

#include <chrono>
#include <iostream>

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
            mesh.positions = load_vector_vector3_from_xml_node(vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-map-0") {
            mesh.tex_coords = load_vector_vector2_from_xml_node(vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-normals") {
            mesh.normals = load_vector_vector3_from_xml_node(vertex_attributes_node);
        } else if(vertex_attributes_node_id == attribute_name + "-colors-Col") {
            mesh.colors = load_vector_vector3_from_xml_node(vertex_attributes_node);
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

std::vector<Vector2> load_vector_vector2_from_xml_node(const tinyxml2::XMLElement* node) {
    std::vector<Vector2> vectors{};
    const auto values_array_node{node->FirstChildElement("float_array")};
    std::stringstream values_array_stream{values_array_node->GetText()};
    while(!values_array_stream.eof()) {
        vectors.emplace_back(load_vector2_from_sstream(values_array_stream));
    }
    return vectors;
}

std::vector<Vector3> load_vector_vector3_from_xml_node(const tinyxml2::XMLElement* node) {
    std::vector<Vector3> vectors{};
    const auto values_array_node{node->FirstChildElement("float_array")};
    std::stringstream values_array_stream{values_array_node->GetText()};
    while(!values_array_stream.eof()) {
        vectors.emplace_back(load_vector3_from_sstream(values_array_stream));
    }
    return vectors;
}

bool check_present_attributes_and_load_indices(tinyxml2::XMLElement* indices_node, const std::size_t count,
        uint8_t& attribs, std::vector<std::uint32_t>& position_indices, std::vector<std::uint32_t>& normal_indices,
        std::vector<std::uint32_t>& tex_coords_indices, std::vector<std::uint32_t>& color_indices) {
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

Vector2 load_vector2_from_sstream(std::stringstream& stream) {
    Vector2 vec{};
    stream >> vec.x >> vec.y;
    return vec;
}

Vector3 load_vector3_from_sstream(std::stringstream& stream) {
    Vector3 vec{};
    stream >> vec.x >> vec.y >> vec.z;
    return vec;
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
            write_vector3(output_file, position);
        }
        for(const auto& tex_coords : mesh.tex_coords) {
            write_vector2(output_file, tex_coords);
        }
        for(const auto& normal : mesh.normals) {
            write_vector3(output_file, normal);
        }
        for(const auto& color : mesh.colors) {
            write_vector3(output_file, color);
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

void write_vector2(std::fstream& file, const Vector2& vec) {
    file.write(reinterpret_cast<const char*>(&vec.x), sizeof(float))
            .write(reinterpret_cast<const char*>(&vec.y), sizeof(float));
}

void write_vector3(std::fstream& file, const Vector3& vec) {
    file.write(reinterpret_cast<const char*>(&vec.x), sizeof(float))
            .write(reinterpret_cast<const char*>(&vec.y), sizeof(float))
            .write(reinterpret_cast<const char*>(&vec.z), sizeof(float));
}

int main(const int argc, const char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: dae2obm [src.dae] [dest.obm]\n";
        return 0;
    }
    const auto start_time = std::chrono::steady_clock::now();
    const auto exit_code = convert(argv[1], argv[2]);
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsed_time = end_time - start_time;
    std::cout << "Conversion time: " << elapsed_time.count() << "s.\n";
    return exit_code;
}
