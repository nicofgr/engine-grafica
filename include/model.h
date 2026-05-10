#ifndef MODEL_H
#define MODEL_H
#include "types.h"

typedef struct Model{
        Vec3_Array vertices;
        u32_Array  faces;
}Model;

void model_create_sphere(Model* model);
void model_free(Model* model);

#endif
