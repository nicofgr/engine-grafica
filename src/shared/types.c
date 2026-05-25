#include "types.h"
#include "cglm/types.h"
#include "cglm/vec3.h"

void Vec3Array_Push(Vec3_Array* vArr, vec3 v){
        if(vArr->size == 0){
                vArr->array = (vec3*) malloc(sizeof(vec3));
        }else{
                vArr->array = (vec3*) realloc(vArr->array, sizeof(vec3)*(vArr->size+1));
        }
        u32 index = vArr->size;
        glm_vec3_copy(v, vArr->array[index]);
        vArr->size++;
}



