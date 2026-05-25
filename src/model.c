
#include "model.h"
#include "constants.h"

// MODEL
typedef struct Model{ // Meshes, texture_color_buffers, materials, rig, shaders, uv mapping
        // Meshes
        u32* meshID;
        u32  n_meshes;
        // Material
        u32 materialID;
}Model;

typedef struct ModelArray{
        Model* array;
        u32     size;
}ModelArray;

ModelArray modelArray;


typedef struct MaterialArray{
        Material* array;
        u32       size;
}MaterialArray;

MaterialArray materialArray;

u32 materialArray_push(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);

// DEFINITIONS
static u32 modelArray_push(u32* meshID, u32 n_meshes, u32 materialID){
        if(modelArray.size == 0){
                modelArray.array = (Model*) malloc(sizeof(Model));
        }else{
                modelArray.array = (Model*) realloc(modelArray.array, sizeof(Model)*(modelArray.size+1));
        }
        u32 index = modelArray.size;
        modelArray.array[index].meshID = (u32*) malloc(sizeof(u32) * n_meshes);
        for(int i = 0; i < n_meshes; i++){
                modelArray.array[index].meshID[i] = meshID[i];
        }
        modelArray.array[index].n_meshes   = n_meshes;
        modelArray.array[index].materialID = materialID;
        modelArray.size++;
        return index;
}

static Model modelArray_Get(u32 modelID){
        return modelArray.array[modelID];
}

u32 model_get_meshID(u32 modelID){
        return modelArray.array[modelID].meshID[0]; // WARNING
}

u32 model_get_materialID(u32 modelID){
        return modelArray.array[modelID].materialID; // WARNING
}

u32 model_create(u32* meshID, u32 n_meshes, u32 materialID){
        u32 index = modelArray_push(meshID, n_meshes, materialID);
        return index;
}

u32 model_duplicate(u32 modelID){
        Model duplicate = modelArray_Get(modelID);
        u32 newID       = modelArray_push(duplicate.meshID, duplicate.n_meshes, duplicate.materialID);
        return newID;
}

void model_setup(){
        modelArray.array = NULL;
        modelArray.size  = 0;

        // Standard material
        materialArray_push(color.gray, color.gray, color.gray, 32.0f, color.black);
}

void model_freeall(){
        for(int i = 0; i < modelArray.size; i++){
                free(modelArray.array[i].meshID);
        }
        free(modelArray.array); 
}

void model_change_material(u32 modelID, u32 materialID){
        if(materialID >= materialArray.size){
                fprintf(stderr,"[ERROR] Material %d is not on materialArray", materialID);
                exit(1);
        }
        modelArray.array[modelID].materialID = materialID;
}

// Material Stuff ============================================

u32 materialArray_push(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission){
        if(materialArray.size == 0){
                materialArray.array = (Material*) malloc(sizeof(Material));
        }else{
                materialArray.array = (Material*) realloc(materialArray.array, sizeof(Material)*(materialArray.size+1));
        }
        u32 index = materialArray.size;
        materialArray.array[index].ambient   = ambient;
        materialArray.array[index].diffuse   = diffuse;
        materialArray.array[index].specular  = specular;
        materialArray.array[index].shininess = shininess;
        materialArray.array[index].emission  = emission;
        materialArray.size++;
        return index;
}

Material materialArray_Get(u32 materialID){
        if(materialID >= materialArray.size){
                fprintf(stderr, "[ERROR] materialID (%d) greater than materialArray size (%d)\n", materialID, materialArray.size);
                exit(0);
        }
        return materialArray.array[materialID];
}


