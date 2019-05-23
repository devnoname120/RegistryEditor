#ifndef __MAIN_H__
#define __MAIN_H__

#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ime_dialog.h>
#include <psp2/types.h>
#include <psp2/registrymgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>

#include <vita2d.h>

enum {
	KEY_TYPE_INT=0,
	KEY_TYPE_STR=1,
	KEY_TYPE_BIN=2,
};

// Font
#define FONT_SIZE 1
#define FONT_X_SPACE 15
#define FONT_Y_SPACE 23

// Scroll bar
#define SCROLL_BAR_X 0
#define SCROLL_BAR_WIDTH 8
#define SCROLL_BAR_MIN_HEIGHT 4

#define MAX_POSITION 16
#define MAX_ENTRIES 16

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//ANALOG
#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64
#define ANALOG_SENSITIVITY 16

enum Colors 
{
	// Primary colors
	RED				= 0xFF0000FF,
	GREEN			= 0xFF00FF00,
	BLUE			= 0xFFFF0000,
	// Secondary colors
	CYAN			= 0xFFFFFF00,
	MAGENTA			= 0xFFFF00FF,
	YELLOW			= 0xFF00FFFF,
	// Tertiary colors
	AZURE			= 0xFFFF7F00,
	VIOLET			= 0xFFFF007F,
	ROSE			= 0xFF7F00FF,
	ORANGE			= 0xFF007FFF,
	CHARTREUSE		= 0xFF00FF7F,
	SPRING_GREEN	= 0xFF7FFF00,
	// Grayscale
	WHITE			= 0xFFFFFFFF,
	LITEGRAY		= 0xFFBFBFBF,
	GRAY			= 0xFF7F7F7F,
	DARKGRAY		= 0xFF3F3F3F,
	BLACK			= 0xFF000000
};

enum 
{
	SCE_CTRL_RIGHT_ANALOG_UP	= 0x0020000,
	SCE_CTRL_RIGHT_ANALOG_RIGHT	= 0x0040000,
	SCE_CTRL_RIGHT_ANALOG_DOWN	= 0x0080000,
	SCE_CTRL_RIGHT_ANALOG_LEFT	= 0x0100000,

	SCE_CTRL_LEFT_ANALOG_UP		= 0x0200000,
	SCE_CTRL_LEFT_ANALOG_RIGHT	= 0x0400000,
	SCE_CTRL_LEFT_ANALOG_DOWN	= 0x0800000,
	SCE_CTRL_LEFT_ANALOG_LEFT	= 0x1000000,
};

typedef struct RegistryKey
{
	char *keyPath;
	char *keyName;
	int keyType;
	int keySize;
} RegistryKey;

typedef struct RegistryDirectory
{
	char *name;
	struct RegistryDirectory *parent;
	int numSubDirs;
	int numKeys;
	struct RegistryDirectory **subdirs;
	RegistryKey **keys;
} RegistryDirectory;

#endif

