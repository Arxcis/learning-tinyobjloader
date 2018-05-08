#include <cstdio>
#include <string>
#include <iostream>

#include <tiny_obj_loader/tiny_obj_loader.h>


struct Vertex
{
    float         x,y,z;
    float         nx,ny,nz;
    float         u,v;
    unsigned char r=255,g=255,b=255,a=255;
};

struct Triangle 
{
    unsigned long a,b,c;
};


int main() { 

    // @doc The code below is a write off from https://github.com/syoyo/tinyobjloader - 07.05.2018
    constexpr char inputfile[] = "../assets/obj/cube/cube.obj";

    auto attrib    = tinyobj::attrib_t{};
    auto shapes    = std::vector<tinyobj::shape_t>{};
    auto materials = std::vector<tinyobj::material_t>{};
    auto err       = std::string{};

    auto success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile);

    if (!err.empty()) {
        std::cerr << "ERR: " << err << '\n';
    }

    if (!success) {
        exit(1);
    }


    std::cout << "# of vertices  : " << (attrib.vertices.size() / 3)  << '\n';
    std::cout << "# of normals   : " << (attrib.normals.size() / 3)   << '\n';
    std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2) << '\n';
    std::cout << "# of shapes    : " << shapes.size()                 << '\n';
    std::cout << "# of materials : " << materials.size()              << '\n';

    // @note
    // Preparing the overkill vertices
    // These vertices has to have the same .size() as .obj attrib.vertices
    // There is a 1 to 1 relationship.
    // The only thing missing is the corresponding texcoords and normals.
    // In .obj files the number of texcoords and normals is often fewer than number
    // of vertices. This is because many texcoords and normals are shared between
    // vertices to save space. In overkill vertices we do not do that. We keep data
    // much closer to how the GPU expects the data. Actually we keeep it just the way
    // the GPU wants it.
    // We just fill the texels and normals into the preallocated vertex slots 
    // below. - JSolsvik 08.05.2018

    std::vector<Vertex> overkillVertices;
    overkillVertices.resize(attrib.vertices.size());
    std::vector<Triangle> overkillTriangles;

    for (auto shape: shapes)
    {
        auto mesh = shape.mesh;
        printf("shape.name = %s\n", shape.name.data());
        printf("Size of shape.indices: %lu\n", static_cast<unsigned long>(mesh.indices.size()));

        std::size_t indexOffset = 0;

        for (std::size_t faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); ++faceIdx) 
        {
            std::size_t faceVertexCount = mesh.num_face_vertices[faceIdx];
            
            if (faceVertexCount != 3) {
                std::cerr << "ERR: VertexNum != 3\n";
                exit(0);
            }

            printf("face[%lu].a = %d/%d/%d  ", faceIdx, mesh.indices[faceIdx + 0].vertex_index
                                                      , mesh.indices[faceIdx + 0].normal_index
                                                      , mesh.indices[faceIdx + 0].texcoord_index);

            printf("face[%lu].b = %d/%d/%d  ", faceIdx, mesh.indices[faceIdx + 1].vertex_index
                                                      , mesh.indices[faceIdx + 1].normal_index
                                                      , mesh.indices[faceIdx + 1].texcoord_index);

            printf("face[%lu].c = %d/%d/%d\n", faceIdx, mesh.indices[faceIdx + 2].vertex_index
                                                      , mesh.indices[faceIdx + 2].normal_index
                                                      , mesh.indices[faceIdx + 2].texcoord_index);
        }
    }

    return 0; 
}