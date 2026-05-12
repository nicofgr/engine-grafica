#ifndef MESH_H
#define MESH_H
#include "types.h"

typedef struct Mesh{
        Vec3_Array vertices;
        Vec3_Array normals;
        u32_Array  faces;
}Mesh;

void mesh_create_sphere(Mesh* mesh);
void mesh_free(Mesh* mesh);
void mesh_printData(const Mesh mesh);

#endif
