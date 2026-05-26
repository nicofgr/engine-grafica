#ifndef MODEL_H
#define MODEL_H

#include "../../shared/types.h"

// MODEL
u32  model_create(u32* meshID, u32 n_meshes, u32 materialID);
void model_change_material(u32 modelID, u32 materialID);
u32  model_get_meshID(u32 modelID);
u32  model_get_materialID(u32 modelID);
u32  model_duplicate(u32 modelID);

void model_setup();
void model_freeall();

// MATERIAL
typedef struct Material{
        Color_RGB ambient;
        Color_RGB diffuse;
        Color_RGB specular;
        float     shininess;
        Color_RGB emission;
}Material;

u32 materialArray_push(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);
Material materialArray_Get(u32 materialID);
#endif
