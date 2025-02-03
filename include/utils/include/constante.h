#pragma once
#ifndef CONSTANTE_H
#define CONSTANTE_H

#define PPM 32.0f
#define GRAVITY_X 0
#define GRAVITY_Y 0
#define ZOOMMAX 1.75f
#define ZOOMSPEED 2.0f
#define TILESIZE 32
/**
 * @brief En gros j'aggrandis un tout petit peu la taille d'une tile pour cacher les trous.
 * Comme on travaille sur des valeurs discrètes (pixels), effectuer des zoom peut générer des mini décalages entre les tiles.
*/
#define TILESIZEW 33 
#define PLAYERSIZE_W 20
#define PLAYERSIZE_H 48
#define PLAYERSIZE 48

#define PLAYERCATEGORY 0x0008
#define ENNEMYCATEGORY 0x004
#define TILECATEGORY 0x0010
#define BORDERCATEGORY 0x0016
#define PROJECTILECATEGORY 0x0040

#endif