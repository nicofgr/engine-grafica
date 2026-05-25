#include "mesh.h"
#include "cglm/vec3.h"
#include "constants.h"
#include "types.h"
#include <string.h>

void mesh_free(Mesh* mesh){
        free(mesh->faces.array);
        free(mesh->vertices.array);
        free(mesh->normals.array);
}

int mesh_pushVert(Mesh* mesh, vec3 vertice){
        if(mesh->vertices.size > 0){ // Look if vertice dont already exist
                for(int i = 0; i < mesh->vertices.size; i++){
                        if( mesh->vertices.array[i][0] == vertice[0] &&
                            mesh->vertices.array[i][1] == vertice[1] &&
                            mesh->vertices.array[i][2] == vertice[2]){
                                return i;
                        }
                }
        }
        if(mesh->vertices.size == 0){
                mesh->vertices.array = (vec3*)malloc(sizeof(vec3));
        }else{
                mesh->vertices.array = (vec3*)realloc(mesh->vertices.array, sizeof(vec3)*(mesh->vertices.size+1));
        }
        glm_vec3_copy(vertice, mesh->vertices.array[mesh->vertices.size]);
        mesh->vertices.size++;
        return mesh->vertices.size-1;
}

void mesh_pushVerts(Mesh* mesh, const Vec3_Array vertices){
        if(mesh->vertices.size == 0){
                mesh->vertices.array = (vec3*)malloc(sizeof(vec3)*vertices.size);
        }else{
                mesh->vertices.array = (vec3*)realloc(mesh->vertices.array, sizeof(vec3)*(mesh->vertices.size+vertices.size));
        }
        for(int i = 0; i < vertices.size; i++){
                glm_vec3_copy(vertices.array[i], mesh->vertices.array[mesh->vertices.size]);
                mesh->vertices.size++;
        }
}

void mesh_pushNormals(Mesh* mesh, const Vec3_Array normals){
        if(mesh->normals.size == 0){
                mesh->normals.array = (vec3*)malloc(sizeof(vec3)*normals.size);
        }else{
                mesh->normals.array = (vec3*)realloc(mesh->normals.array, sizeof(vec3)*(mesh->normals.size+normals.size));
        }
        for(int i = 0; i < normals.size; i++){
                glm_vec3_copy(normals.array[i], mesh->normals.array[mesh->normals.size]);
                mesh->normals.size++;
        }
}

void mesh_pushFaces(Mesh* mesh, const u32_Array faces){
        if(mesh->faces.size == 0){
                mesh->faces.array = (u32*)malloc(sizeof(u32)*faces.size*3);
        }else{
                mesh->faces.array = (u32*)realloc(mesh->faces.array, sizeof(u32)*(mesh->faces.size+faces.size)*3);
        }
        //printf("%d faces\n\n", faces.size);
        for(int i = 0; i < faces.size; i++){
                mesh->faces.array[mesh->faces.size*3]   = faces.array[i*3];
                mesh->faces.array[mesh->faces.size*3+1] = faces.array[(i*3)+1];
                mesh->faces.array[mesh->faces.size*3+2] = faces.array[(i*3)+2];
                mesh->faces.size++;
        }
}
void mesh_removeFace(Mesh* mesh, u32 face){  // Copiar a ultima pra face removida e realloc
        u32 x1 = mesh->faces.array[(face*3)];
        u32 x2 = mesh->faces.array[(face*3)+1];
        u32 x3 = mesh->faces.array[(face*3)+2];
        printf("Removing face %d: %d %d %d\n", face, x1, x2, x3);
        if(mesh->faces.size > 1){
                memcpy(&mesh->faces.array[face*3], &mesh->faces.array[(mesh->faces.size-1)*3], sizeof(u32)*3);
        }
        u32* temp = (u32*)realloc(mesh->faces.array, sizeof(u32)*((mesh->faces.size-1)*3));
        mesh->faces.array = temp;
        mesh->faces.size--;
}
void mesh_subdivideFace(Mesh* mesh, u32 face){
        u32 x1 = mesh->faces.array[face*3];
        u32 x2 = mesh->faces.array[face*3+1];
        u32 x3 = mesh->faces.array[face*3+2];
        mesh_removeFace(mesh, face);
        
        // New vertices
        vec3 v4;
        vec3 v5;
        vec3 v6;
        glm_vec3_add(mesh->vertices.array[x1], mesh->vertices.array[x2], v4);
        glm_vec3_scale(v4, 0.5f, v4);
        glm_vec3_add(mesh->vertices.array[x2], mesh->vertices.array[x3], v5);
        glm_vec3_scale(v5, 0.5f, v5);
        glm_vec3_add(mesh->vertices.array[x3], mesh->vertices.array[x1], v6);
        glm_vec3_scale(v6, 0.5f, v6);
        
        Vec3_Array normals = {.array = NULL, .size = 0};
        Vec3Array_Push(&normals, (vec3){0.0f, 0.0f, 0.0f});
        Vec3Array_Push(&normals, (vec3){0.0f, 0.0f, 0.0f});
        Vec3Array_Push(&normals, (vec3){0.0f, 0.0f, 0.0f});
        mesh_pushNormals(mesh, normals);
        free(normals.array);

        // New faces from new vertices
        u32 x4 = mesh_pushVert(mesh, v4);
        u32 x5 = mesh_pushVert(mesh, v5);
        u32 x6 = mesh_pushVert(mesh, v6);
        u32 temp[12] = {x4, x5, x6,
                        x1, x4, x6,
                        x4, x2, x5,
                        x5, x3, x6};
        u32_Array new_faces = {.array=temp, .size = 4};
        mesh_pushFaces(mesh, new_faces);
}

void mesh_printData(const Mesh mesh){
        puts("VERTICES:");
        for(int i = 0; i < mesh.vertices.size; i++){
                float x = mesh.vertices.array[i][0];
                float y = mesh.vertices.array[i][1];
                float z = mesh.vertices.array[i][2];
                float nx = mesh.normals.array[i][0];
                float ny = mesh.normals.array[i][1];
                float nz = mesh.normals.array[i][2];
                printf("v%d: (%.2f %.2f %.2f) | nv%d: (%.2f %.2f %.2f)\n",i, x, y, z, i, nx, ny, nz);
        }
        puts("FACES:");
        for(int i = 0; i < mesh.faces.size; i++){
                printf("%d ", mesh.faces.array[i*3]);
                printf("%d ", mesh.faces.array[(i*3)+1]);
                printf("%d ", mesh.faces.array[(i*3)+2]);
                puts("");
        }
        fflush(stdout);
}

void mesh_clear(Mesh* mesh){
        if(mesh->faces.array != NULL){
                free(mesh->faces.array);
                mesh->faces.size = 0;
        }
        if(mesh->vertices.array != NULL){
                free(mesh->vertices.array);
                mesh->vertices.size = 0;
        }
        if(mesh->normals.array != NULL){
                free(mesh->normals.array);
                mesh->normals.size = 0;
        }
}

void mesh_create_icosahedron(Mesh* mesh){
        mesh_clear(mesh);
        vec3 vertices[12*2] = {{0.0f, 1.0f, GOLDEN_RATIO},
                             {0.0f, 1.0f, -GOLDEN_RATIO},
                             {0.0f, -1.0f, GOLDEN_RATIO},
                             {0.0f, -1.0f, -GOLDEN_RATIO},
                             {1.0f, GOLDEN_RATIO, 0.0f},
                             {1.0f, -GOLDEN_RATIO, 0.0f},
                             {-1.0f, GOLDEN_RATIO, 0.0f},
                             {-1.0f, -GOLDEN_RATIO, 0.0f},
                             {GOLDEN_RATIO, 0.0f, 1.0f,},
                             {GOLDEN_RATIO, 0.0f, -1.0f,},
                             {-GOLDEN_RATIO, 0.0f, 1.0f,},
                             {-GOLDEN_RATIO, 0.0f, -1.0f,}};
        Vec3_Array icosahedron_vertices = {.array = vertices, .size = 12};
        u32 faces[20*3] = { // Faces
                        0, 2, 8,
                        0, 8, 4,
                        0, 4, 6,
                        0, 6, 10,
                        0, 10, 2,

                        3, 1, 9,
                        3, 9, 5,
                        3, 5, 7,
                        3, 7, 11,
                        3, 11, 1,

                        1,4,9,
                        9,4,8,
                        9,8,5,
                        5,8,2,
                        5,2,7,
                        7,2,10,
                        7,10,11,
                        11,10,6,
                        11,6,1,
                        1,6,4
        };
        u32_Array icosahedron_faces = {.array=faces, .size=20};
        mesh_pushVerts(mesh, icosahedron_vertices);
        mesh_pushFaces(mesh, icosahedron_faces);
        for(int i = 0; i < mesh->vertices.size; i++){
                glm_vec3_normalize(mesh->vertices.array[i]);
        }
        mesh_pushNormals(mesh, mesh->vertices);
        //mesh_printData(*mesh);
        
        printf("Face center positions \n");
        for(int i = 0; i < icosahedron_faces.size; i++){
                vec3 sum_vec;
                u32 vert1_id = icosahedron_faces.array[(i*3)];
                u32 vert2_id = icosahedron_faces.array[(i*3) + 1];
                u32 vert3_id = icosahedron_faces.array[(i*3) + 2];
                glm_vec3_add(mesh->vertices.array[vert1_id], mesh->vertices.array[vert2_id], sum_vec);
                glm_vec3_add(mesh->vertices.array[vert3_id], sum_vec, sum_vec);
                glm_vec3_divs(sum_vec, 3, sum_vec);
                printf("[%2d](%.2f %.2f %.2f)\n",i, sum_vec[0], sum_vec[1], sum_vec[2]);
        }
        puts("");
}

void mesh_subdivide(Mesh* mesh){
        u32 nFaces = mesh->faces.size;
        for(int i = 0; i < nFaces-1; i++){
                mesh_subdivideFace(mesh, i);
        }
        mesh_subdivideFace(mesh, 0);
}

void mesh_create_sphere(Mesh* mesh, u32 nSubdiv){
        mesh_clear(mesh);
        mesh_create_icosahedron(mesh);
        for(int i = 0; i < nSubdiv; i++){
                mesh_subdivide(mesh);
        }
        for(int i = 0; i < mesh->vertices.size; i++){
                glm_vec3_normalize(mesh->vertices.array[i]);
                glm_vec3_copy(mesh->vertices.array[i], mesh->normals.array[i]);
        }
        printf("[MESH] Vertex count: %d\n", mesh->vertices.size);
}

void mesh_new(Mesh* mesh){
        mesh->vertices.array = NULL;
        mesh->normals.array  = NULL;
        mesh->faces.array    = NULL;
        mesh->vertices.size  = 0;
        mesh->normals.size   = 0;
        mesh->faces.size     = 0;
}

void mesh_create_triangle(Mesh* mesh, const u32 subdivision_level){
        mesh_clear(mesh);
        float L = 1;
        float invsqrt3 = 1/sqrt(3);
        vec3 vertices[3] = {{0.0, L*invsqrt3, 0.0}, {-L*0.5, -L*0.5*invsqrt3, 0.0}, {L*0.5, -L*0.5*invsqrt3, 0.0}};
        Vec3_Array triangle_vertices = {.array = vertices, .size = 3};
        u32 faces[3] = {0, 1, 2};
        u32_Array triangle_faces = {.array=faces, .size=1};

        mesh_pushVerts(mesh, triangle_vertices);
        mesh_pushFaces(mesh, triangle_faces);
        for(int i = 0; i < mesh->vertices.size; i++){
                glm_vec3_normalize(mesh->vertices.array[i]); // WARNING TODO Wrong normals
        }
        mesh_pushNormals(mesh, mesh->vertices);

        for(int i = 0; i < subdivision_level; i++){
                mesh_subdivide(mesh);
        }
}
