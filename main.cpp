#include <cstdio>
#include <string>
#include <iostream>
#include <cstdint>
#include <limits>

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s64 = std::int64_t;


#include <tiny_obj_loader/tiny_obj_loader.h>


// @ref GetBaseDir copy-pasted from https://github.com/syoyo/tinyobjloader/blob/master/examples/viewer/viewer.cc - 08.05.2018
// Added a +1 to the selection of the substring though... - JSolsvik 08.05.2018
static std::string GetBaseDir(const std::string& filepath) {
  if (filepath.find_last_of("/\\") != std::string::npos)
    return filepath.substr(0, filepath.find_last_of("/\\")+1);
  return ".";
}

static bool FileExists(const std::string& abs_filename) {
  bool ret;
  FILE* fp = fopen(abs_filename.c_str(), "rb");
  if (fp) {
    ret = true;
    fclose(fp);
  } else {
    ret = false;
  }

  return ret;
}

static std::string handleArguments(const int argc, const char** argv) 
{
    if (argc > 2 ) {
        std::cout << "ONLY 0 or 1 argument allowed \n";
        exit(1);
    }
    std::string objfilepath;

    if (argc == 2) {
        
        objfilepath = argv[1];
        if(!FileExists(objfilepath)) {
            std::cout << "File " << objfilepath << " does not exist\n";
            exit(1);
        }
    }
    else {
        objfilepath = "../assets/obj/cube/cube.obj";
    }
    return objfilepath;
}


int main(const int argc, const char** argv) { 

    auto objfilepath = handleArguments(argc, argv);


    // @doc The code below is a write off from https://github.com/syoyo/tinyobjloader - 07.05.2018
    std::string base_dir = GetBaseDir(objfilepath);

    auto attrib    = tinyobj::attrib_t{};
    auto shapes    = std::vector<tinyobj::shape_t>{};
    auto materials = std::vector<tinyobj::material_t>{};
    auto err       = std::string{};

    auto success = tinyobj::LoadObj(
        &attrib, 
        &shapes, 
        &materials, 
        &err, 
        objfilepath.c_str(), 
        base_dir.c_str() );

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

    struct OKVertex
    {
        // Using this as a flag if the Vertex has been initialized
        float x; 
        float y,z;
        float nx,ny,nz;
        float u,v;
        u8    r=255,g=255,b=255,a=255;
    };

    struct OKTriangle 
    {
        s64 a,b,c;
    };


    struct OKMaterial 
    {

    };

    struct OKMesh 
    {
        std::vector<OKTriangle> triangles;
        OKMaterial material;
    };

    std::vector<OKVertex> overkillVertices;
    std::vector<OKMesh>   overkillMeshes;

    overkillVertices.resize(attrib.vertices.size() / 3 );


    for (auto shape: shapes)
    {
        auto& mesh = shape.mesh;

        auto overkillMesh = OKMesh{};

        printf("\n\nMesh.name = %s\n", shape.name.data());
        printf("Mesh.number_of_indices: %lu\n", static_cast<u64>(mesh.indices.size()));
        printf("Mesh.number_of_faces: %lu\n",   static_cast<u64>(mesh.num_face_vertices.size()));
        
        

        const u8 TriangleStride = 3;
        for (u64 faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); faceIdx += TriangleStride)
        {

            if (mesh.num_face_vertices[faceIdx] != 3) {
                std::cerr << "ERR: mesh.num_face_vertices != 3  --> " << mesh.num_face_vertices[faceIdx] << '\n';
                std::cerr << "TRIANGULATE YOUR MESH'es!\n";
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

            const u8 VertStride = 3;
            const u8 NormStride = 3;
            const u8 TexStride = 2;
            
            auto debugPrint = [&]() 
            {
                // Uncompress vertices
                printf("\n"
                       "va = %f %f %f\n", vertices[ia.vertex_index * VertStride + 0], 
                                          vertices[ia.vertex_index * VertStride + 1], 
                                          vertices[ia.vertex_index * VertStride + 2]);

                printf("vb = %f %f %f\n",  vertices[ib.vertex_index * VertStride + 0], 
                                           vertices[ib.vertex_index * VertStride + 1], 
                                           vertices[ib.vertex_index * VertStride + 2]);

                printf("vc =%f %f %f\n", vertices[ic.vertex_index * VertStride + 0], 
                                         vertices[ic.vertex_index * VertStride + 1], 
                                         vertices[ic.vertex_index * VertStride + 2]);

                // Uncompress normals
                printf("\n"
                       "na = %f %f %f\n", normals[ia.normal_index * NormStride + 0], 
                                          normals[ia.normal_index * NormStride + 1], 
                                          normals[ia.normal_index * NormStride + 2]);

                printf("nb = %f %f %f\n",  normals[ib.normal_index * NormStride + 0], 
                                           normals[ib.normal_index * NormStride + 1], 
                                           normals[ib.normal_index * NormStride + 2]);

                printf("nc =%f %f %f\n", normals[ic.normal_index * NormStride + 0], 
                                         normals[ic.normal_index * NormStride + 1], 
                                         normals[ic.normal_index * NormStride + 2]);

                // Uncompress texture coordinates
                printf("\n"
                       "ta = %f %f\n", texcoords[ia.texcoord_index * TexStride + 0], 
                                       texcoords[ia.texcoord_index * TexStride + 1]);

                printf("tb = %f %f\n", texcoords[ib.texcoord_index * TexStride + 0], 
                                       texcoords[ib.texcoord_index * TexStride + 1]);

                printf("tc = %f %f\n", texcoords[ic.texcoord_index * TexStride + 0], 
                                       texcoords[ic.texcoord_index * TexStride + 1]);            



                // Face indicies
                printf("face[%lu] a = %d/%d/%d  ", faceIdx, ia.vertex_index
                                                          , ia.normal_index
                                                          , ia.texcoord_index);

                printf(" b =  %d/%d/%d  ", ib.vertex_index
                                         , ib.normal_index
                                         , ib.texcoord_index);

                printf(" c =  %d/%d/%d\n", ic.vertex_index
                                         , ic.normal_index
                                         , ic.texcoord_index);
            };

            // debugPrint();

            // triangleVertex.a
            overkillVertices[ia.vertex_index] = 
                OKVertex {
                    vertices[ia.vertex_index * VertStride + 0],
                    vertices[ia.vertex_index * VertStride + 1],
                    vertices[ia.vertex_index * VertStride + 2],
        
                    normals[ia.normal_index * NormStride + 0],
                    normals[ia.normal_index * NormStride + 1],
                    normals[ia.normal_index * NormStride + 2],

                    texcoords[ia.texcoord_index * TexStride + 0],                
                    texcoords[ia.texcoord_index * TexStride + 1]
                };

            // triangleVertex.b
            overkillVertices[ib.vertex_index] = 
                OKVertex {
                    vertices[ib.vertex_index * VertStride + 0],
                    vertices[ib.vertex_index * VertStride + 1],
                    vertices[ib.vertex_index * VertStride + 2],
        
                    normals[ib.normal_index * NormStride + 0],
                    normals[ib.normal_index * NormStride + 1],
                    normals[ib.normal_index * NormStride + 2],

                    texcoords[ib.texcoord_index * TexStride + 0],                
                    texcoords[ib.texcoord_index * TexStride + 1] 
                };

            // triangleVertex.c
            overkillVertices[ic.vertex_index] = 
                OKVertex {
                    vertices[ic.vertex_index * VertStride + 0],
                    vertices[ic.vertex_index * VertStride + 1],
                    vertices[ic.vertex_index * VertStride + 2],
        
                    normals[ic.normal_index * NormStride + 0],
                    normals[ic.normal_index * NormStride + 1],
                    normals[ic.normal_index * NormStride + 2],

                    texcoords[ic.texcoord_index * TexStride + 0],                
                    texcoords[ic.texcoord_index * TexStride + 1]
                };


            // triangleIndicies.abc
            overkillMesh.triangles.emplace_back( OKTriangle {
                ia.vertex_index,
                ib.vertex_index,
                ic.vertex_index
            });
        }
        overkillMeshes.push_back(overkillMesh);
    }


    printf("vertices count: %lu\n", static_cast<u64>(overkillVertices.size()));

    return 0; 
}