#ifndef SAVEMAP_H
#define SAVEMAP_H

#define MAP_TAG			{'C','S','M'}
#define MAP_VERSION		4

float ConvertHeight(unsigned char brightness);
void LoadJPGMap(const char* relative);
void FreeMap();
bool LoadMap(const char* name);
bool SaveMap(const char* name);

#endif
