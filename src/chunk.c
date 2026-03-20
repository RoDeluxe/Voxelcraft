#include "chunk.h"
#include "noise.h"
#include <string.h>
#include <math.h>

/* Colores por tipo de bloque y cara */
static const float colors[3][6][3] = {
    /* AIR - no se usa */
    {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
    /* GRASS: top, bottom, front, back, left, right */
    {
        {0.55f,0.80f,0.38f}, /* top   - verde hierba  */
        {0.45f,0.34f,0.22f}, /* bottom- tierra oscura */
        {0.48f,0.73f,0.33f}, /* front - verde lado    */
        {0.42f,0.65f,0.28f}, /* back  - verde oscuro  */
        {0.45f,0.68f,0.30f}, /* left  - verde medio   */
        {0.50f,0.72f,0.32f}, /* right - verde medio   */
    },
    /* DIRT */
    {
        {0.60f,0.46f,0.32f},
        {0.45f,0.34f,0.22f},
        {0.58f,0.44f,0.30f},
        {0.55f,0.42f,0.28f},
        {0.57f,0.43f,0.29f},
        {0.59f,0.45f,0.31f},
    },
};

void chunk_generate(Chunk* c, int cx, int cz) {
    memset(c->blocks, BLOCK_AIR, sizeof(c->blocks));
    c->x = cx; c->z = cz;

    for(int x=0;x<CHUNK_W;x++) {
        for(int z=0;z<CHUNK_D;z++) {
            float wx = (cx*CHUNK_W + x) * 0.1f;
            float wz = (cz*CHUNK_D + z) * 0.1f;

            /* Altura entre 2 y 6 */
            float n = simplex2(wx, wz);
            int height = (int)(3 + n * 2.5f);
            if(height < 1) height = 1;
            if(height >= CHUNK_H) height = CHUNK_H-1;

            /* Capa superior: hierba */
            c->blocks[x][height][z] = BLOCK_GRASS;

            /* Capas inferiores: tierra */
            for(int y=0; y<height; y++)
                c->blocks[x][y][z] = BLOCK_DIRT;
        }
    }
}

static uint8_t get_block(Chunk* c, int x, int y, int z) {
    if(x<0||x>=CHUNK_W||y<0||y>=CHUNK_H||z<0||z>=CHUNK_D)
        return BLOCK_AIR;
    return c->blocks[x][y][z];
}

/* Añadir quad (2 triángulos, 6 vértices) al buffer */
static int add_face(float* buf, int vi,
    float x0,float y0,float z0,
    float x1,float y1,float z1,
    float x2,float y2,float z2,
    float x3,float y3,float z3,
    float r,float g,float b)
{
    float* v=buf+vi*6;
    /* Tri 1 */
    v[ 0]=x0;v[ 1]=y0;v[ 2]=z0;v[ 3]=r;v[ 4]=g;v[ 5]=b;
    v[ 6]=x1;v[ 7]=y1;v[ 8]=z1;v[ 9]=r;v[10]=g;v[11]=b;
    v[12]=x2;v[13]=y2;v[14]=z2;v[15]=r;v[16]=g;v[17]=b;
    /* Tri 2 */
    v[18]=x2;v[19]=y2;v[20]=z2;v[21]=r;v[22]=g;v[23]=b;
    v[24]=x3;v[25]=y3;v[26]=z3;v[27]=r;v[28]=g;v[29]=b;
    v[30]=x0;v[31]=y0;v[32]=z0;v[33]=r;v[34]=g;v[35]=b;
    return vi+6;
}

int chunk_build_mesh(Chunk* c, float* buf, int max_verts) {
    int vi=0;
    float ox=c->x*CHUNK_W, oz=c->z*CHUNK_D;

    for(int x=0;x<CHUNK_W;x++) {
        for(int y=0;y<CHUNK_H;y++) {
            for(int z=0;z<CHUNK_D;z++) {
                uint8_t b=c->blocks[x][y][z];
                if(b==BLOCK_AIR) continue;

                float fx=ox+x, fy=y, fz=oz+z;
                const float (*col)[3]=colors[b];

                /* Top */
                if(get_block(c,x,y+1,z)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx,fy+1,fz+1, fx+1,fy+1,fz+1,
                        fx+1,fy+1,fz,   fx,fy+1,fz,
                        col[0][0],col[0][1],col[0][2]);
                /* Bottom */
                if(get_block(c,x,y-1,z)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx,fy,fz+1,   fx+1,fy,fz+1,
                        fx+1,fy,fz,   fx,fy,fz,
                        col[1][0],col[1][1],col[1][2]);
                /* Front +Z */
                if(get_block(c,x,y,z+1)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx,fy,fz+1,   fx+1,fy,fz+1,
                        fx+1,fy+1,fz+1, fx,fy+1,fz+1,
                        col[2][0],col[2][1],col[2][2]);
                /* Back -Z */
                if(get_block(c,x,y,z-1)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx+1,fy,fz,   fx,fy,fz,
                        fx,fy+1,fz,   fx+1,fy+1,fz,
                        col[3][0],col[3][1],col[3][2]);
                /* Left -X */
                if(get_block(c,x-1,y,z)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx,fy,fz,     fx,fy,fz+1,
                        fx,fy+1,fz+1, fx,fy+1,fz,
                        col[4][0],col[4][1],col[4][2]);
                /* Right +X */
                if(get_block(c,x+1,y,z)==BLOCK_AIR && vi+6<=max_verts)
                    vi=add_face(buf,vi,
                        fx+1,fy,fz+1, fx+1,fy,fz,
                        fx+1,fy+1,fz, fx+1,fy+1,fz+1,
                        col[5][0],col[5][1],col[5][2]);
            }
        }
    }
    return vi;
}
