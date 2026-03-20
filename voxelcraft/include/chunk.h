#pragma once
#include <stdint.h>

#define CHUNK_W  16
#define CHUNK_H  8
#define CHUNK_D  16

/* Tipos de bloque */
#define BLOCK_AIR   0
#define BLOCK_GRASS 1
#define BLOCK_DIRT  2

typedef struct {
    uint8_t blocks[CHUNK_W][CHUNK_H][CHUNK_D];
    int      x, z;  /* posición del chunk en el mundo */
} Chunk;

/* Genera el terreno del chunk usando noise */
void chunk_generate(Chunk* c, int cx, int cz);

/* Construye la geometría del chunk en el buffer dado.
   Retorna el número de vértices generados. */
int chunk_build_mesh(Chunk* c, float* buf, int max_verts);
