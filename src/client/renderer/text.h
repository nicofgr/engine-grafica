#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../../shared/types.h"

typedef struct Character{
        u32  TextureID;
        vec2 Size;
        vec2 Bearing;
        u32  Advance;
}Character;

void text_init();
Character CharMap_Get(const char c);

#endif
