#pragma once
#include "chunk.h"

typedef struct {
    float x, y, z;      /* posición (pies del jugador) */
    float vx, vy, vz;   /* velocidad                   */
    float yaw, pitch;    /* dirección de la cámara      */
    int on_ground;       /* 1 si está en el suelo       */
} Player;

void player_init(Player* p, float x, float y, float z);
void player_update(Player* p, Chunk* c, float dt);

/* Colisión AABB jugador vs mundo */
int player_on_ground(Player* p, Chunk* c);
