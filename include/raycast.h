#pragma once
#include "chunk.h"

typedef struct {
    int hit;           /* 1 si golpeó un bloque        */
    int bx, by, bz;   /* bloque golpeado               */
    int nx, ny, nz;   /* bloque adyacente (para poner) */
} RayHit;

/* Lanza un rayo desde (ox,oy,oz) en dirección (dx,dy,dz)
   max_dist = distancia máxima en bloques */
RayHit raycast(Chunk* c,
    float ox, float oy, float oz,
    float dx, float dy, float dz,
    float max_dist);
