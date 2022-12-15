# FS-LevelRenderer_OGL
## Full Sail Level Renderer Project 
### Author: Pedro J Colon
#### Credit goes for Gateware project to making this happen! https://gitlab.com/gateware-development/gateware

Camera Controls:
This uses flight/tank controls so a heads up on it being a bit janky.

W - Forward
A - Left
S - Backward
D - Right


Shift - Down
Space - Up


Mouse X - Look Horizontoal
Mouse Y - Look Vertical

Features:
1. One or More Blender Game Levels to export from (.blend)
2. Exporting OBJ Names & Locations from your Levels (GameLevel.txt)
3. Run-time file I/O parsing of exported level information (GameLevel.txt)
4. Reading binary model data for all referenced models (*.h2b + h2bParser.h)
5. Transfering model geometry, material and matrix data to GPU
6. One model imported from GameLevel.txt is drawing correctly
7. Working 3D Fly-through Camera (Assignment 1 or Better)
8. All models successfully drawn at proper location, orientation & scale
9. All models have correct OBJ material colors showing
10. Directional light with ambient term and specular highlights (Assignment 2 or Better)
