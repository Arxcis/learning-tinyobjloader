#include <cstdio>
#include <string>
#include <iostream>
#include <cstdint>

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

#include <tiny_obj_loader/tiny_obj_loader.h>


int main() { 

    // @doc The code below is a write off from https://github.com/syoyo/tinyobjloader - 07.05.2018
    constexpr char inputfile[] = "../assets/obj/moriknob/testObj.obj";

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

    std::vector<Vertex> overkillVertices;
    overkillVertices.resize(attrib.vertices.size());
    std::vector<Triangle> overkillTriangles;

    for (auto shape: shapes)
    {
        auto& mesh = shape.mesh;
        printf("\n\nshape.name = %s\n", shape.name.data());
        printf("Size of shape.indices: %lu\n", static_cast<u64>(mesh.indices.size()));

        std::size_t indexOffset = 0;

        for (u64 faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); ) 
        {
            if (mesh.num_face_vertices[faceIdx] != 3) {
                std::cerr << "ERR: mesh.num_face_vertices != 3\n";
                exit(0);
            }

            auto& indices = mesh.indices;
            auto& vertices = attrib.vertices;
            auto& normals = attrib.normals;
            auto& texcoords = attrib.texcoords;
           // auto& colors = attrib.colors; This is just an extension of the obj format.


            auto& ia = indices[faceIdx + 0]; 
            auto& ib = indices[faceIdx + 1];
            auto& ic = indices[faceIdx + 2];

            // Uncompress vertices
            {
                const u8 Stride = 3;
                printf("\n"
                       "va = %f %f %f\n", vertices[ia.vertex_index * Stride + 0], 
                                          vertices[ia.vertex_index * Stride + 1], 
                                          vertices[ia.vertex_index * Stride + 2]);

                printf("vb = %f %f %f\n",  vertices[ib.vertex_index * Stride + 0], 
                                           vertices[ib.vertex_index * Stride + 1], 
                                           vertices[ib.vertex_index * Stride + 2]);

                printf("vc =%f %f %f\n", vertices[ic.vertex_index * Stride + 0], 
                                         vertices[ic.vertex_index * Stride + 1], 
                                         vertices[ic.vertex_index * Stride + 2]);
            }

            // Uncompress normals
            {
                const u8 Stride = 3;
                printf("\n"
                       "na = %f %f %f\n", normals[ia.normal_index * Stride + 0], 
                                          normals[ia.normal_index * Stride + 1], 
                                          normals[ia.normal_index * Stride + 2]);

                printf("nb = %f %f %f\n",  normals[ib.normal_index * Stride + 0], 
                                           normals[ib.normal_index * Stride + 1], 
                                           normals[ib.normal_index * Stride + 2]);

                printf("nc =%f %f %f\n", normals[ic.normal_index * Stride + 0], 
                                         normals[ic.normal_index * Stride + 1], 
                                         normals[ic.normal_index * Stride + 2]);
            }

            // Uncompress texture coordinates
            {
                const u8 Stride = 2;
                printf("\n"
                       "ta = %f %f\n", texcoords[ia.texcoord_index * Stride + 0], 
                                       texcoords[ia.texcoord_index * Stride + 1]);

                printf("tb = %f %f\n", texcoords[ib.texcoord_index * Stride + 0], 
                                       texcoords[ib.texcoord_index * Stride + 1]);

                printf("tc = %f %f\n", texcoords[ic.texcoord_index * Stride + 0], 
                                       texcoords[ic.texcoord_index * Stride + 1]);

            }

            printf("face[%lu] a = %d/%d/%d  ", faceIdx, ia.vertex_index
                                                      , ia.normal_index
                                                      , ia.texcoord_index);

            printf(" b =  %d/%d/%d  ", ib.vertex_index
                                     , ib.normal_index
                                     , ib.texcoord_index);

            printf(" c =  %d/%d/%d\n", ic.vertex_index
                                     , ic.normal_index
                                     , ic.texcoord_index);


            if (faceIdx > 2)
                exit(0);

            faceIdx += 3;
        }
    }

    return 0; 
}