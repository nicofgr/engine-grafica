#include "renderer/text.h"
#include "freetype/freetype.h"
#include <glad/glad.h>

FT_Library ft;
FT_Face face;

// Map
typedef struct CharMap{
        char*      charIndex;
        Character* chars;
        u32        size;
}CharMap;

CharMap Characters;

void CharMap_insert(CharMap* cMap, const char c, const Character character){
        if(cMap->size == 0){
                cMap->charIndex = (char*)malloc(sizeof(char));
                cMap->chars = (Character*)malloc(sizeof(Character));
        }else{
                cMap->charIndex = (char*)realloc(cMap->charIndex, sizeof(char)*(cMap->size+1));
                cMap->chars = (Character*)realloc(cMap->chars, sizeof(Character)*(cMap->size+1));
        }
        cMap->charIndex[cMap->size] = c;
        cMap->chars[cMap->size] = character;
        cMap->size++;
}

void CharMap_print(){
        for(int i = 0; i < Characters.size; i++){
                printf("%c ", Characters.charIndex[i]);
        }
        fflush(stdout);
}

Character CharMap_Get(const char c){
        for(int i = 0; i < Characters.size; i++){
                if(Characters.charIndex[i] == c){
                        return Characters.chars[i];
                }
        }
}

void text_init(){
        if (FT_Init_FreeType(&ft)){
                fprintf(stderr, "[ERROR][FREETYPE] Could not init freetype library\n");
                exit(0);
        }
        if(FT_New_Face(ft, "fonts/CodeNewRomanNerdFont-Regular.otf", 0, &face)){
                fprintf(stderr, "[ERROR][FREETYPE] Failes do load font\n");
                exit(0);
        }
        FT_Set_Pixel_Sizes(face, 0, 48);

        Characters.size = 0;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for(unsigned char c = 0; c < 128; c++){
                if(FT_Load_Char(face, c, FT_LOAD_RENDER)){
                        fprintf(stderr, "[ERROR::FREETYPE] Failed to load glyph\n");
                        exit(1);
                }
                // Texture generation
                u32 texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RED,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        0,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer
                );
                // Texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                
                Character character;
                character.TextureID = texture;
                glm_vec2_copy((vec2){face->glyph->bitmap.width, face->glyph->bitmap.rows}, character.Size);
                glm_vec2_copy((vec2){face->glyph->bitmap_left, face->glyph->bitmap_top}, character.Bearing);
                character.Advance = face->glyph->advance.x;

                CharMap_insert(&Characters, c, character);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        //CharMap_print();
}


