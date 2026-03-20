#include "player.h"
#include <math.h>

#define GRAVITY    -20.0f
#define JUMP_VEL    8.0f
#define PLAYER_H    1.8f
#define PLAYER_W    0.6f

void player_init(Player* p, float x, float y, float z){
    p->x=x; p->y=y; p->z=z;
    p->vx=p->vy=p->vz=0.0f;
    p->yaw=0.0f; p->pitch=0.0f;
    p->on_ground=0;
}

/* Verifica si hay bloque sólido en esa posición del mundo */
static int is_solid(Chunk* c, int bx, int by, int bz){
    if(bx<0||bx>=CHUNK_W||by<0||by>=CHUNK_H||bz<0||bz>=CHUNK_D)
        return 0;
    return c->blocks[bx][by][bz] != BLOCK_AIR;
}

/* Colisión en un eje - retorna 1 si hay colisión */
static int collides(Chunk* c, float x, float y, float z){
    /* Esquinas del AABB del jugador */
    float hw=PLAYER_W*0.5f;
    int x0=(int)floorf(x-hw), x1=(int)floorf(x+hw);
    int y0=(int)floorf(y),     y1=(int)floorf(y+PLAYER_H);
    int z0=(int)floorf(z-hw), z1=(int)floorf(z+hw);

    for(int bx=x0;bx<=x1;bx++)
        for(int by=y0;by<=y1;by++)
            for(int bz=z0;bz<=z1;bz++)
                if(is_solid(c,bx,by,bz)) return 1;
    return 0;
}

int player_on_ground(Player* p, Chunk* c){
    return collides(c, p->x, p->y-0.05f, p->z);
}

void player_update(Player* p, Chunk* c, float dt){
    /* Gravedad */
    p->vy += GRAVITY * dt;

    /* Mover en Y */
    float ny = p->y + p->vy * dt;
    if(!collides(c, p->x, ny, p->z)){
        p->y = ny;
        p->on_ground = 0;
    } else {
        if(p->vy < 0) p->on_ground = 1;
        p->vy = 0;
    }

    /* Mover en X */
    float nx = p->x + p->vx * dt;
    if(!collides(c, nx, p->y, p->z))
        p->x = nx;
    else
        p->vx = 0;

    /* Mover en Z */
    float nz = p->z + p->vz * dt;
    if(!collides(c, p->x, p->y, nz))
        p->z = nz;
    else
        p->vz = 0;

    /* Fricción horizontal */
    p->vx *= 0.8f;
    p->vz *= 0.8f;
}
