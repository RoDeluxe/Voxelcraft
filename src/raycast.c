#include "raycast.h"
#include <math.h>

RayHit raycast(Chunk* c,
    float ox, float oy, float oz,
    float dx, float dy, float dz,
    float max_dist)
{
    RayHit hit={0};

    /* Normalizar dirección */
    float len=sqrtf(dx*dx+dy*dy+dz*dz);
    if(len<0.0001f) return hit;
    dx/=len; dy/=len; dz/=len;

    /* DDA - Digital Differential Analyzer */
    int bx=(int)floorf(ox);
    int by=(int)floorf(oy);
    int bz=(int)floorf(oz);

    float sx=dx>0?1:-1, sy=dy>0?1:-1, sz=dz>0?1:-1;
    float tx=fabsf(1.0f/dx), ty=fabsf(1.0f/dy), tz=fabsf(1.0f/dz);

    float nx2=dx>0?(bx+1-ox)*tx:(ox-bx)*tx;
    float ny2=dy>0?(by+1-oy)*ty:(oy-by)*ty;
    float nz2=dz>0?(bz+1-oz)*tz:(oz-bz)*tz;

    int prev_bx=bx, prev_by=by, prev_bz=bz;
    float dist=0;

    while(dist<max_dist){
        /* Verificar bloque actual */
        if(bx>=0&&bx<CHUNK_W&&by>=0&&by<CHUNK_H&&bz>=0&&bz<CHUNK_D){
            if(c->blocks[bx][by][bz]!=BLOCK_AIR){
                hit.hit=1;
                hit.bx=bx; hit.by=by; hit.bz=bz;
                hit.nx=prev_bx; hit.ny=prev_by; hit.nz=prev_bz;
                return hit;
            }
        }

        prev_bx=bx; prev_by=by; prev_bz=bz;

        /* Avanzar al siguiente bloque */
        if(nx2<ny2 && nx2<nz2){
            bx+=(int)sx; dist=nx2; nx2+=tx;
        } else if(ny2<nz2){
            by+=(int)sy; dist=ny2; ny2+=ty;
        } else {
            bz+=(int)sz; dist=nz2; nz2+=tz;
        }
    }
    return hit;
}
