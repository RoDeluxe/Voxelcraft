# 🟩 Voxelcraft
> Un clon de Minecraft hecho en C como homebrew para Nintendo Switch

![voxelcraft](https://github.com/user-attachments/assets/90598a89-ed7c-4fc1-af8c-832f9e4fdc7c)

---

## 📖 Descripción

Voxelcraft es un clon de Minecraft desarrollado como homebrew para Nintendo Switch. El juego genera un mundo de bloques de forma procedural usando Simplex Noise, permitiendo al jugador explorar, construir y destruir bloques libremente.

Desarrollado completamente en **C** usando **deko3d**, la API gráfica nativa de Nintendo Switch sobre hardware Nvidia Maxwell.

---

## 📋 Requisitos

| Requisito | Versión |
|-----------|---------|
| Nintendo Switch / Switch Lite | Cualquier modelo |
| Atmosphere CFW | v1.9.5 o superior |
| Firmware de sistema | 20.5.0 o superior |
| hbmenu | v3.6.0 o superior |

---

## 📦 Instalación

1. Descarga el archivo `voxelcraft.nro` de la sección [Releases](../../releases)
2. Copia la carpeta a tu tarjeta SD:
```
SD:/switch/voxelcraft/voxelcraft.nro
```
3. Abre el **Homebrew Launcher** en tu Switch
4. Selecciona **Voxelcraft** y presiona **A** para lanzar

---

## 🎮 Controles

### Menú Principal
| Botón | Acción |
|-------|--------|
| `A` | Jugar |
| `+` | Salir |

### En el Juego
| Botón | Acción |
|-------|--------|
| Joystick izquierdo | Moverse |
| Joystick derecho | Rotar cámara |
| `A` | Saltar |
| `ZR` | Romper bloque |
| `ZL` | Colocar bloque |
| `+` | Salir al sistema |

---

## ✨ Características

- 🌍 Mundo generado proceduralmente con **Simplex Noise**
- 🧱 25 chunks activos (5x5) = **80x80 bloques** de superficie
- ⛏️ 3 tipos de bloques: **hierba, tierra y piedra**
- 🏃 Física con **gravedad y colisiones AABB**
- 🎯 Sistema de **raycast** para interacción con bloques
- 🖥️ Menú principal con identidad visual propia
- ➕ Mira tipo Minecraft con hueco central

---

## 🔧 Detalles Técnicos

| Parámetro | Valor |
|-----------|-------|
| Lenguaje | C |
| API Gráfica | deko3d 0.5.0 |
| SDK | devkitPro / libnx |
| Shaders | GLSL 4.60 (compilados con uam) |
| Noise | Simplex Noise 2D |
| Resolución | 1280x720 |

---

## 📁 Estructura del Proyecto

```
voxelcraft/
├── src/
│   ├── main.c       # Bucle principal, renderizado, input
│   ├── chunk.c      # Sistema de chunks y generación del mundo
│   ├── player.c     # Física del jugador y colisiones
│   ├── raycast.c    # Detección de bloques apuntados
│   ├── noise.c      # Generador de Simplex Noise
│   └── font.c       # Fuente bitmap 8x8 para el HUD
├── include/
│   ├── chunk.h
│   ├── player.h
│   ├── raycast.h
│   ├── noise.h
│   └── font.h
├── romfs/
│   ├── shaders/     # Shaders compilados (.dksh)
│   └── textures/    # Atlas de texturas
├── icon.jpg         # Icono del homebrew (256x256)
└── Makefile
```

---

## 🔨 Compilar desde el código fuente

### Requisitos
- [devkitPro](https://devkitpro.org/) con `devkitA64` y `libnx`
- `switch-deko3d`
- `switch-glm`
- `uam` (compilador de shaders)

### Pasos

```bash
# Clonar el repositorio
git clone https://github.com/tuusuario/voxelcraft.git
cd voxelcraft

# Compilar shaders
uam -s vert -o romfs/shaders/cube.vert.dksh romfs/shaders/cube.vert.glsl
uam -s frag -o romfs/shaders/cube.frag.dksh romfs/shaders/cube.frag.glsl

# Compilar el proyecto
make
```

---

## ⚠️ Limitaciones conocidas

- El mundo **no se guarda** al salir
- Solo hay un tipo de bloque para colocar (tierra)
- Sin iluminación dinámica
- Sin sonido

---

## 📜 Créditos

Desarrollado por **8Deluxe**

Construido con [devkitPro](https://devkitpro.org/) y [deko3d](https://github.com/devkitPro/deko3d)

Algoritmo Simplex Noise basado en la implementación clásica de Ken Perlin

---

<div align="center">
  <strong>8DELUXE — 2026</strong>
</div>
