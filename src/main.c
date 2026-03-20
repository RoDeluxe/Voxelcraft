#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <switch.h>
#include <deko3d.h>
#include "chunk.h"
#include "player.h"
#include "raycast.h"
#include "raycast.h"
#include "raycast.h"

#define CMDMEM_SIZE    (4*1024*1024)
#define VTXMEM_SIZE    (8*1024*1024)
#define SHADERMEM_SIZE (1024*1024)
#define FB_WIDTH       1280
#define FB_HEIGHT      720
#define NUM_FRAMEBUF   2
#define MAX_VERTS      (VTXMEM_SIZE/(6*sizeof(float)))

static void* load_file(const char* path,uint32_t* sz){
    FILE* f=fopen(path,"rb");if(!f)return NULL;
    fseek(f,0,SEEK_END);uint32_t s=(uint32_t)ftell(f);rewind(f);
    void* b=malloc(s);fread(b,1,s,f);fclose(f);if(sz)*sz=s;return b;
}

static void transform_world(float* dst, const float* src, int nverts,
    float cam_x, float cam_y, float cam_z, float yaw, float pitch)
{
    float near=0.1f,far=200.0f;
    float fov=70.0f*(3.14159f/180.0f);
    float f=1.0f/tanf(fov*0.5f);
    float aspect=(float)FB_WIDTH/FB_HEIGHT;
    float cy=cosf(yaw),sy=sinf(yaw);
    float cp=cosf(pitch),sp=sinf(pitch);

    for(int i=0;i<nverts;i++){
        const float* s=src+i*6;
        float* d=dst+i*6;
        float tx=s[0]-cam_x, ty=s[1]-cam_y, tz=s[2]-cam_z;
        float rx= tx*cy+tz*sy;
        float ry= ty;
        float rz=-tx*sy+tz*cy;
        float rx2=rx;
        float ry2=ry*cp-rz*sp;
        float rz2=ry*sp+rz*cp;
        float w=-rz2;
        if(w<0.001f)w=0.001f;
        d[0]=(rx2*(f/aspect))/w;
        d[1]=(ry2*f)/w;
        d[2]=((rz2*(far+near)/(near-far)+(2.0f*far*near)/(near-far)))/w;
        d[3]=s[3];d[4]=s[4];d[5]=s[5];
    }
}

int main(void){
    romfsInit();
    padConfigureInput(1,HidNpadStyleSet_NpadStandard);

    Chunk chunk;
    chunk_generate(&chunk,0,0);

    /* Encontrar altura inicial para el jugador */
    int start_y=CHUNK_H-1;
    for(int y=CHUNK_H-1;y>=0;y--){
        if(chunk.blocks[8][y][8]!=BLOCK_AIR){start_y=y+1;break;}
    }

    Player player;
    player_init(&player, 8.5f, (float)start_y+0.1f, 8.5f);

    float* world_verts=(float*)malloc(MAX_VERTS*6*sizeof(float));
    int nverts=chunk_build_mesh(&chunk,world_verts,MAX_VERTS);
    float* draw_verts=(float*)malloc(MAX_VERTS*6*sizeof(float));
    int mesh_dirty=0;

    DkDeviceMaker dvm;dkDeviceMakerDefaults(&dvm);DkDevice dev=dkDeviceCreate(&dvm);
    DkQueueMaker qm;dkQueueMakerDefaults(&qm,dev);qm.flags=DkQueueFlags_Graphics;DkQueue queue=dkQueueCreate(&qm);

    DkMemBlockMaker mbm;
    dkMemBlockMakerDefaults(&mbm,dev,CMDMEM_SIZE);mbm.flags=DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached;
    DkMemBlock cmdbuf_mem=dkMemBlockCreate(&mbm);
    DkCmdBufMaker cbm;dkCmdBufMakerDefaults(&cbm,dev);DkCmdBuf cmdbuf=dkCmdBufCreate(&cbm);
    dkCmdBufAddMemory(cmdbuf,cmdbuf_mem,0,CMDMEM_SIZE);

    DkImageLayoutMaker ilm;
    dkImageLayoutMakerDefaults(&ilm,dev);
    ilm.flags=DkImageFlags_UsageRender|DkImageFlags_UsagePresent|DkImageFlags_HwCompression;
    ilm.format=DkImageFormat_RGBA8_Unorm;ilm.dimensions[0]=FB_WIDTH;ilm.dimensions[1]=FB_HEIGHT;
    DkImageLayout fbl;dkImageLayoutInitialize(&fbl,&ilm);
    uint32_t fbs=dkImageLayoutGetSize(&fbl),fba=dkImageLayoutGetAlignment(&fbl);
    fbs=(fbs+fba-1)&~(fba-1);
    dkMemBlockMakerDefaults(&mbm,dev,NUM_FRAMEBUF*fbs);mbm.flags=DkMemBlockFlags_GpuCached|DkMemBlockFlags_Image;
    DkMemBlock fb_mem=dkMemBlockCreate(&mbm);
    DkImage fb[NUM_FRAMEBUF];DkImageView fbv[NUM_FRAMEBUF];DkImage const* fbp[NUM_FRAMEBUF];
    for(int i=0;i<NUM_FRAMEBUF;i++){dkImageInitialize(&fb[i],&fbl,fb_mem,i*fbs);dkImageViewDefaults(&fbv[i],&fb[i]);fbp[i]=&fb[i];}

    dkImageLayoutMakerDefaults(&ilm,dev);
    ilm.flags=DkImageFlags_UsageRender|DkImageFlags_HwCompression;
    ilm.format=DkImageFormat_ZF32;ilm.dimensions[0]=FB_WIDTH;ilm.dimensions[1]=FB_HEIGHT;
    DkImageLayout dpl;dkImageLayoutInitialize(&dpl,&ilm);
    uint32_t dps=dkImageLayoutGetSize(&dpl),dpa=dkImageLayoutGetAlignment(&dpl);
    dps=(dps+dpa-1)&~(dpa-1);
    dkMemBlockMakerDefaults(&mbm,dev,dps);mbm.flags=DkMemBlockFlags_GpuCached|DkMemBlockFlags_Image;
    DkMemBlock dp_mem=dkMemBlockCreate(&mbm);
    DkImage dp_img;DkImageView dp_view;
    dkImageInitialize(&dp_img,&dpl,dp_mem,0);dkImageViewDefaults(&dp_view,&dp_img);

    NWindow* win=nwindowGetDefault();
    DkSwapchainMaker scm;dkSwapchainMakerDefaults(&scm,dev,win,fbp,NUM_FRAMEBUF);
    DkSwapchain sc=dkSwapchainCreate(&scm);

    dkMemBlockMakerDefaults(&mbm,dev,VTXMEM_SIZE);mbm.flags=DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached;
    DkMemBlock vtx_mem=dkMemBlockCreate(&mbm);
    float* vtx_gpu=(float*)dkMemBlockGetCpuAddr(vtx_mem);

    dkMemBlockMakerDefaults(&mbm,dev,SHADERMEM_SIZE);
    mbm.flags=DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached|DkMemBlockFlags_Code;
    DkMemBlock sh_mem=dkMemBlockCreate(&mbm);void* sc2=dkMemBlockGetCpuAddr(sh_mem);
    uint32_t vsz,fsz;
    void* vd=load_file("romfs:/shaders/cube.vert.dksh",&vsz);
    void* fd=load_file("romfs:/shaders/cube.frag.dksh",&fsz);
    memcpy(sc2,vd,vsz);uint32_t fo=(vsz+255)&~255u;memcpy((char*)sc2+fo,fd,fsz);
    free(vd);free(fd);
    DkShader vs,fs;DkShaderMaker shm;
    dkShaderMakerDefaults(&shm,sh_mem,0);dkShaderInitialize(&vs,&shm);
    dkShaderMakerDefaults(&shm,sh_mem,fo);dkShaderInitialize(&fs,&shm);

    DkVtxAttribState att[2]={
        {0,0,0,              DkVtxAttribSize_3x32,DkVtxAttribType_Float,0},
        {0,0,3*sizeof(float),DkVtxAttribSize_3x32,DkVtxAttribType_Float,0},
    };
    DkVtxBufferState vbs={6*sizeof(float),0};

    float move_speed=45.0f;
    float look_speed=0.04f;

    /* Tiempo para física */
    uint64_t last_tick=armGetSystemTick();

    PadState pad;padInitializeDefault(&pad);

    while(appletMainLoop()){
        padUpdate(&pad);
        if(padGetButtonsDown(&pad)&HidNpadButton_Plus)break;

        /* Delta time */
        uint64_t now=armGetSystemTick();
        float dt=(float)(now-last_tick)/19200000.0f;
        last_tick=now;
        if(dt>0.05f)dt=0.05f;

        /* Joystick derecho - rotar cámara */
        HidAnalogStickState rs=padGetStickPos(&pad,1);
        float rx=(float)rs.x/32767.0f;
        float ry=(float)rs.y/32767.0f;
        if(fabsf(rx)>0.1f) player.yaw+=rx*look_speed;
        if(fabsf(ry)>0.1f){
            player.pitch-=ry*look_speed;
            if(player.pitch> 1.4f)player.pitch= 1.4f;
            if(player.pitch<-1.4f)player.pitch=-1.4f;
        }

        /* Joystick izquierdo - mover jugador */
        HidAnalogStickState ls=padGetStickPos(&pad,0);
        float lx=(float)ls.x/32767.0f;
        float ly=(float)ls.y/32767.0f;
        if(fabsf(lx)>0.1f){
            player.vx+=cosf(player.yaw)*lx*move_speed*dt;
            player.vz+=sinf(player.yaw)*lx*move_speed*dt;
        }
        if(fabsf(ly)>0.1f){
            player.vx+=sinf(player.yaw)*ly*move_speed*dt;
            player.vz-=cosf(player.yaw)*ly*move_speed*dt;
        }

        /* Saltar con A */
        if((padGetButtonsDown(&pad)&HidNpadButton_A) && player.on_ground)
            player.vy=8.0f;

        /* Raycast - dirección de la cámara */
        float rdx=-cosf(player.pitch)*sinf(player.yaw);
        float rdy=sinf(player.pitch);
        float rdz=-cosf(player.pitch)*cosf(player.yaw);
        float cam_eye_x=player.x, cam_eye_y=player.y+1.6f, cam_eye_z=player.z;
        RayHit rhit=raycast(&chunk,cam_eye_x,cam_eye_y,cam_eye_z,rdx,rdy,rdz,5.0f);

        /* Y - romper bloque */
        if(padGetButtonsDown(&pad)&HidNpadButton_ZR){
            int tbx=(int)(player.x), tby=(int)(player.y), tbz=(int)(player.z);
            if(tbx>=0&&tbx<CHUNK_W&&tby>=0&&tby<CHUNK_H&&tbz>=0&&tbz<CHUNK_D)
                chunk.blocks[tbx][tby][tbz]=BLOCK_AIR;
            mesh_dirty=1;
        }
        /* X - colocar bloque */
        if((padGetButtonsDown(&pad)&HidNpadButton_ZL) && rhit.hit){
            int px=rhit.nx,py=rhit.ny,pz=rhit.nz;
            if(px>=0&&px<CHUNK_W&&py>=0&&py<CHUNK_H&&pz>=0&&pz<CHUNK_D)
                if(chunk.blocks[px][py][pz]==BLOCK_AIR){
                    chunk.blocks[px][py][pz]=BLOCK_DIRT;
                    mesh_dirty=1;
                }
        }
        /* Raycast - dirección de la cámara */

        /* Y - romper bloque */
        if(padGetButtonsDown(&pad)&HidNpadButton_ZR){
            int tbx=(int)(player.x), tby=(int)(player.y), tbz=(int)(player.z);
            if(tbx>=0&&tbx<CHUNK_W&&tby>=0&&tby<CHUNK_H&&tbz>=0&&tbz<CHUNK_D)
                chunk.blocks[tbx][tby][tbz]=BLOCK_AIR;
            mesh_dirty=1;
        }
        /* X - colocar bloque */
        if((padGetButtonsDown(&pad)&HidNpadButton_ZL) && rhit.hit){
            int px=rhit.nx,py=rhit.ny,pz=rhit.nz;
            if(px>=0&&px<CHUNK_W&&py>=0&&py<CHUNK_H&&pz>=0&&pz<CHUNK_D)
                if(chunk.blocks[px][py][pz]==BLOCK_AIR){
                    chunk.blocks[px][py][pz]=BLOCK_DIRT;
                    mesh_dirty=1;
                }
        }
        /* Actualizar física */
        player_update(&player,&chunk,dt);

        /* Cámara = ojos del jugador (1.6 unidades sobre los pies) */
        float cam_x=player.x;
        float cam_y=player.y+1.6f;
        float cam_z=player.z;

        if(mesh_dirty){
            nverts=chunk_build_mesh(&chunk,world_verts,MAX_VERTS);
            mesh_dirty=0;
        }
        if(mesh_dirty){
            nverts=chunk_build_mesh(&chunk,world_verts,MAX_VERTS);
            mesh_dirty=0;
        }
        if(mesh_dirty){
            nverts=chunk_build_mesh(&chunk,world_verts,MAX_VERTS);
            mesh_dirty=0;
        }
        /* Rebuild mesh si cambió el mundo */
        transform_world(draw_verts,world_verts,nverts,
            cam_x,cam_y,cam_z,player.yaw,player.pitch);
        memcpy(vtx_gpu,draw_verts,nverts*6*sizeof(float));

        int slot=dkQueueAcquireImage(queue,sc);
        dkCmdBufClear(cmdbuf);
        DkImageView const* ct=&fbv[slot];
        DkImageView const* dt2=&dp_view;
        dkCmdBufBindRenderTargets(cmdbuf,&ct,1,dt2);
        DkViewport vp2={0.0f,0.0f,FB_WIDTH,FB_HEIGHT,0.0f,1.0f};
        DkScissor ss={0,0,FB_WIDTH,FB_HEIGHT};
        dkCmdBufSetViewports(cmdbuf,0,&vp2,1);
        dkCmdBufSetScissors(cmdbuf,0,&ss,1);
        dkCmdBufClearColorFloat(cmdbuf,0,DkColorMask_RGBA,0.529f,0.808f,0.922f,1.0f);
        dkCmdBufClearDepthStencil(cmdbuf,true,1.0f,false,0);
        DkShader const* shaders[]={&vs,&fs};
        dkCmdBufBindShaders(cmdbuf,DkStageFlag_GraphicsMask,shaders,2);
        DkRasterizerState rst;dkRasterizerStateDefaults(&rst);
        rst.cullMode=DkFace_Back;
        rst.frontFace=DkFrontFace_CCW;
        dkCmdBufBindRasterizerState(cmdbuf,&rst);
        DkColorState cs;dkColorStateDefaults(&cs);dkCmdBufBindColorState(cmdbuf,&cs);
        DkColorWriteState cws;dkColorWriteStateDefaults(&cws);dkCmdBufBindColorWriteState(cmdbuf,&cws);
        DkDepthStencilState ds;dkDepthStencilStateDefaults(&ds);ds.depthTestEnable=true;ds.depthWriteEnable=true;dkCmdBufBindDepthStencilState(cmdbuf,&ds);
        dkCmdBufBindVtxBuffer(cmdbuf,0,dkMemBlockGetGpuAddr(vtx_mem),nverts*6*sizeof(float));
        dkCmdBufBindVtxAttribState(cmdbuf,att,2);
        dkCmdBufBindVtxBufferState(cmdbuf,&vbs,1);
        dkCmdBufDraw(cmdbuf,DkPrimitive_Triangles,nverts,1,0,0);
        DkCmdList list=dkCmdBufFinishList(cmdbuf);
        dkQueueSubmitCommands(queue,list);
        dkQueuePresentImage(queue,sc,slot);
    }

    free(world_verts);free(draw_verts);
    dkQueueWaitIdle(queue);
    dkSwapchainDestroy(sc);dkMemBlockDestroy(sh_mem);
    dkMemBlockDestroy(vtx_mem);dkMemBlockDestroy(dp_mem);
    dkMemBlockDestroy(fb_mem);dkMemBlockDestroy(cmdbuf_mem);
    dkCmdBufDestroy(cmdbuf);dkQueueDestroy(queue);dkDeviceDestroy(dev);
    romfsExit();return 0;
}
