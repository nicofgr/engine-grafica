#ifndef MESH_H
#define MESH_H
#include "../shared/types.h"

typedef struct Mesh{ // Verts, Edges, Faces, Normals, Vert colors
        Vec3_Array vertices;
        Vec3_Array normals;
        u32_Array  faces;
}Mesh;

void mesh_create_sphere(Mesh* mesh, u32 nSubdiv);
void mesh_create_triangle(Mesh* mesh, const u32 subdivision_level);

void mesh_new(Mesh* mesh);
void mesh_free(Mesh* mesh);

void mesh_printData(const Mesh mesh);


#endif
