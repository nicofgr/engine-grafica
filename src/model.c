#include "model.h"
#include "constants.h"
#include <string.h>

void model_free(Model* model){
        free(model->faces.array);
        free(model->vertices.array);
}

int model_pushVert(Model* model, vec3 vertice){
        if(model->vertices.size == 0){
                model->vertices.array = (vec3*)malloc(sizeof(vec3));
        }else{
                model->vertices.array = (vec3*)realloc(model->vertices.array, sizeof(vec3)*(model->vertices.size+1));
        }
        glm_vec3_copy(vertice, model->vertices.array[model->vertices.size]);
        model->vertices.size++;
        return model->vertices.size-1;
}

void model_pushVerts(Model* model, const Vec3_Array vertices){
        if(model->vertices.size == 0){
                model->vertices.array = (vec3*)malloc(sizeof(vec3)*vertices.size);
        }else{
                model->vertices.array = (vec3*)realloc(model->vertices.array, sizeof(vec3)*(model->vertices.size+vertices.size));
        }
        for(int i = 0; i < vertices.size; i++){
                glm_vec3_copy(vertices.array[i], model->vertices.array[model->vertices.size]);
                model->vertices.size++;
        }
}
void model_pushFaces(Model* model, const u32_Array faces){
        if(model->faces.size == 0){
                model->faces.array = (u32*)malloc(sizeof(u32)*faces.size);
        }else{
                model->faces.array = (u32*)realloc(model->faces.array, sizeof(u32)*(model->faces.size+faces.size));
        }
        for(int i = 0; i < faces.size; i++){
                model->faces.array[model->faces.size] = faces.array[i];
                model->faces.size++;
        }
}
void model_removeFace(Model* model, u32 face){  // Copiar a ultima pra face removida e realloc
        u32 x1 = model->faces.array[face*3];
        u32 x2 = model->faces.array[face*3+1];
        u32 x3 = model->faces.array[face*3+2];
        //printf("Removing face %d: %d %d %d\n", face, x1, x2, x3);
        memcpy(&model->faces.array[face*3], &model->faces.array[model->faces.size-3], sizeof(u32)*3);
        u32* temp = (u32*)realloc(model->faces.array, sizeof(u32)*(model->faces.size-3));
        if(temp == NULL)
                exit(0);
        model->faces.array = temp;
        model->faces.size -= 3;
}
void model_subdivideFace(Model* model, u32 face){
        u32 x1 = model->faces.array[face*3];
        u32 x2 = model->faces.array[face*3+1];
        u32 x3 = model->faces.array[face*3+2];
        model_removeFace(model, face);
        
        vec3 v4;
        vec3 v5;
        vec3 v6;
        glm_vec3_add(model->vertices.array[x1], model->vertices.array[x2], v4);
        glm_vec3_scale(v4, 0.5f, v4);
        glm_vec3_add(model->vertices.array[x2], model->vertices.array[x3], v5);
        glm_vec3_scale(v5, 0.5f, v5);
        glm_vec3_add(model->vertices.array[x3], model->vertices.array[x1], v6);
        glm_vec3_scale(v6, 0.5f, v6);

        u32 x4 = model_pushVert(model, v4);
        u32 x5 = model_pushVert(model, v5);
        u32 x6 = model_pushVert(model, v6);
        u32 temp[12] = {x4, x5, x6,
                       x1, x4, x6,
                       x4, x2, x5,
                       x5, x3, x6};
        u32_Array new_faces = {.array=temp, .size = 12};
        model_pushFaces(model, new_faces);
}

void model_printData(const Model model){
        puts("VERTICES:");
        for(int i = 0; i < model.vertices.size; i++){
                float x = model.vertices.array[i][0];
                float y = model.vertices.array[i][1];
                float z = model.vertices.array[i][2];
                printf("%.2f %.2f %.2f\n", x, y, z);
        }
        puts("FACES:");
        for(int i = 0; i < model.faces.size; i++){
                printf("%d ", model.faces.array[i]);
                if((i+1)%3 == 0)
                        puts("");
        }
        fflush(stdout);
}

void model_clear(Model* model){
        if(model->faces.array != NULL){
                free(model->faces.array);
                model->faces.size = 0;
        }
        if(model->vertices.array != NULL){
                free(model->vertices.array);
                model->faces.array = 0;
        }
}

void model_create_icosahedron(Model* model){
        model_clear(model);
        vec3 vertices[12] = {{0.0f, 1.0f, GOLDEN_RATIO},
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
        u32_Array icosahedron_faces = {.array=faces, .size=20*3};
        model_pushVerts(model, icosahedron_vertices);
        model_pushFaces(model, icosahedron_faces);
        for(int i = 0; i < model->vertices.size; i++){
                glm_vec3_normalize(model->vertices.array[i]);
        }
}

void model_subdivide(Model* model){
        u32 nFaces = model->faces.size/3.0f;
        for(int i = 0; i < nFaces-1; i++){
                model_subdivideFace(model, i);
        }
        model_subdivideFace(model, 0);
}

void model_create_sphere(Model* model){
        model_clear(model);
        model_create_icosahedron(model);
        model_subdivide(model);
        model_subdivide(model);
        model_subdivide(model);
        for(int i = 0; i < model->vertices.size; i++){
                glm_vec3_normalize(model->vertices.array[i]);
        }
}
