/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/apputil.h>

#include <vita2d.h>*/
#include "main.h"
#include "ime_dialog.h"

// Store positions so that the right item is selected when the user presses the back button
int topList[10] = {0};
int curPosList[10] = {0};
int posInd = 0;


void initSceAppUtil()
{
	// Init SceAppUtil
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	memset(&init_param, 0, sizeof(SceAppUtilInitParam));
	memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);

	// Set common dialog config
	SceCommonDialogConfigParam config;
	sceCommonDialogConfigParamInit(&config);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&config.language);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&config.enterButtonAssign);
	sceCommonDialogSetConfigParam(&config);
}

void getButtons(int *holdButtons, int *pressedButtons)
{
	static int hold_n = 0;
	static int old_buttons, current_buttons, pressed_buttons, hold_buttons;
	SceCtrlData pad;

	memset(&pad, 0, sizeof(SceCtrlData));
	sceCtrlPeekBufferPositive(0, &pad, 1);

	if (pad.ly < ANALOG_CENTER - ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_UP;
	else if (pad.ly > ANALOG_CENTER + ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_DOWN;

	if (pad.lx < ANALOG_CENTER - ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_LEFT;
	else if (pad.lx > ANALOG_CENTER + ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_RIGHT;

	if (pad.ry < ANALOG_CENTER - ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_UP;
	else if (pad.ry > ANALOG_CENTER + ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_DOWN;

	if (pad.rx < ANALOG_CENTER - ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_LEFT;
	else if (pad.rx > ANALOG_CENTER + ANALOG_THRESHOLD)
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_RIGHT;

	current_buttons = pad.buttons;
	pressed_buttons = current_buttons & ~old_buttons;
	hold_buttons = pressed_buttons;

	if (old_buttons == current_buttons)
	{
		hold_n++;
		if (hold_n >= 10)
		{
			hold_buttons = current_buttons;
			hold_n = 6;
		}
	}
	else
	{
		hold_n = 0;
		old_buttons = current_buttons;
	}

	*holdButtons = hold_buttons;
	*pressedButtons = pressed_buttons;
}

int startsWith(const char *str1, const char *str2)
{
	return strncmp(str1, str2, strlen(str1)) == 0;
}

void initializeRegistryDirectory(RegistryDirectory *root)
{
	RegistryDirectory *curDir = root;
	int i = 0;

	while(i < REGDATA_SIZE)
	{
		//printf("%d\n", i);
		if(startsWith(curDir->name, regData[i].keyPath) && strcmp(curDir->name, regData[i].keyPath) != 0)
		{
			//Child Node
			RegistryDirectory *childDir = (RegistryDirectory*)malloc(sizeof(RegistryDirectory));

			//Initialize child
			childDir->numKeys = 0;
			childDir->numSubDirs = 0;
			childDir->parent = curDir;
			childDir->keys = NULL;
			childDir->subdirs = NULL;

			//Set child name
			int len = strstr(regData[i].keyPath + strlen(curDir->name), "/") - regData[i].keyPath;
			childDir->name = (char*)malloc(len + 2);
			memset(childDir->name, 0, len + 2);
			strncpy(childDir->name, regData[i].keyPath, len + 1);

			//Insert child into parent
			curDir->numSubDirs++;
			curDir->subdirs = (RegistryDirectory**)realloc(curDir->subdirs, sizeof(RegistryDirectory*) * curDir->numSubDirs);
			curDir->subdirs[curDir->numSubDirs - 1] = childDir;

			//Next
			curDir = childDir;
		}
		else if(strcmp(curDir->name, regData[i].keyPath) == 0)
		{
			//Insert into current dir
			curDir->numKeys++;
			curDir->keys = (RegistryKey**)realloc(curDir->keys, sizeof(RegistryKey*) * curDir->numKeys);
			curDir->keys[curDir->numKeys - 1] = &regData[i];

			i++;
		}
		else
			curDir = curDir->parent;
		//curDir
	}
}

void freeRegistryDirectoryTree(RegistryDirectory *dir)
{
	int i;

	//Free children
	for(i = 0; i < dir->numSubDirs; i++)
	{
		freeRegistryDirectoryTree(dir->subdirs[i]);
	}

	if(strcmp(dir->name, "/")) //Don't free root
	{
		free(dir->name);
		free(dir->subdirs);
		free(dir->keys);
		free(dir);
	}
}

void printRegistryDirectoryTree(RegistryDirectory *dir, int depth)
{
	int i, j;

	if(!dir)
		return;

	for(j = 0; j < depth; i++)
		printf("    ");

	if(dir->parent)
		printf("%s\n", dir->name + strlen(dir->parent->name));
	else
		printf("%s\n", dir->name);

	for(i = 0; i < dir->numSubDirs; i++)
	{
		printRegistryDirectoryTree(dir->subdirs[i], depth+1);
	}

	for(i = 0; i < dir->numKeys; i++)
	{
		for(j = 0; j < depth + 1; j++)
			printf("    ");

		printf("%s\n", dir->keys[i]->keyName);
	}
}

void drawScrollBar(int pos, int n)
{
	if (n > MAX_POSITION)
	{
		vita2d_draw_rectangle(SCROLL_BAR_X, 4 * FONT_Y_SPACE, SCROLL_BAR_WIDTH, MAX_ENTRIES * FONT_Y_SPACE, GRAY);

		int y = (4 * FONT_Y_SPACE) + ((pos * FONT_Y_SPACE) / (n * FONT_Y_SPACE)) * (MAX_ENTRIES * FONT_Y_SPACE);
		int height = ((MAX_POSITION * FONT_Y_SPACE) / (n * FONT_Y_SPACE)) * (MAX_ENTRIES * FONT_Y_SPACE);

		vita2d_draw_rectangle(0, MIN(y, ((4 * FONT_Y_SPACE) + MAX_ENTRIES * FONT_Y_SPACE - height)), SCROLL_BAR_WIDTH, MAX(height, SCROLL_BAR_MIN_HEIGHT), AZURE);
	}
}

char int2hex[] = "0123456789ABCDEF";

// buf2 needs to be at least 3x sizeof(buf)
void convert2hex(char *buf, char *buf2, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		buf2[3*i+0] = int2hex[buf[i] >> 4];
		buf2[3*i+1] = int2hex[buf[i] & 0xf];
		buf2[3*i+2] = ' ';
	}
	buf2[3*i+0] = 0;
}

int convert2bin(char *buf, char *buf2, int len)
{
	int cnt = 0;
	int i;
	unsigned char tmp;

	tmp = 0;
	for (i = 0; i < len; i++){
		// ignore spaces
		if (buf2[i] == ' ')
			continue;

		if (buf2[i] >= '0' && buf2[i] <= '9')
		{
			tmp = (tmp << 4) | (buf2[i] - '0');
			cnt++;
		}
		else if (buf2[i] >= 'A' && buf2[i] <= 'F')
		{
			tmp = (tmp << 4) | (0x0a + buf2[i] - 'A');
			cnt++;
		}
		else if (buf2[i] >= 'a' && buf2[i] <= 'f')
		{
			tmp = (tmp << 4) | (0x0a + buf2[i] - 'a');
			cnt++;
		}
		else
		{
			// bad character
			return -1;
		}

		if (cnt % 2 == 0)
		{
			buf[cnt/2-1] = tmp;
			tmp = 0;
		}
	}

	if (cnt % 2 == 1)
		return -1;

	return cnt/2;
}

int regMgrGetKeyInt(char *path, char *keyName)
{
	int val = -1, ret = 0;

	ret = sceRegMgrGetKeyInt(path, keyName, &val);

	if(ret < 0)
		return ret;
	else
		return val;
}

/*
	Errors from SceMgrGetKey
	0x800D000E - Not supported on this device
	0x800D000A - Path doesn't exist
	0x800D0014 - Key doesn't exist
	0x800D0015 - ?
	0x800D0017 - Something not found ex (/CONFIG/US_ENGLISH/input_type: 0x800D0017)
	0x800D000B - Wrong data type
	0x80022005 - You suck at returning my strings (No size provided)
*/

int main()
{
	curPosList[0] = 1;
	topList[0] = 0;
	int curSize = 0, ret = 0, imeStatus = 0, keyIndex = 0;
	int holdButtons = 0, pressedButtons = 0;
	int writeRet = 0;
	char buf[2048];
	char buf2[2048*3];
	vita2d_pgf *pgf;
	RegistryDirectory root =
	{
		"/",
		NULL,
		0,
		0,
		NULL,
		NULL
	};
	RegistryDirectory *curDir = &root;
	RegistryDirectory *writeDir = NULL;

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	initSceAppUtil();
	initializeRegistryDirectory(&root);

	vita2d_init();
	pgf = vita2d_load_default_pgf();

	while (1)
	{
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_pgf_draw_text(pgf, 20, FONT_Y_SPACE, VIOLET, FONT_SIZE, "RegistryEditor");
		vita2d_pgf_draw_text(pgf, 20, 2 * FONT_Y_SPACE, LITEGRAY, FONT_SIZE, curDir->name);

		imeStatus = updateImeDialog();

		if(imeStatus == IME_DIALOG_RESULT_FINISHED && writeDir)
		{
			char *output = (char *)getImeDialogInputTextUTF8();
			RegistryKey *key = writeDir->keys[keyIndex];

			if(key->keyType == KEY_TYPE_INT)
				writeRet = sceRegMgrSetKeyInt(writeDir->name, key->keyName, atoi(output));
			else if(key->keyType == KEY_TYPE_STR)
				if (strlen(output)+1 <= key->keySize)
				{
					writeRet = sceRegMgrSetKeyStr(writeDir->name, key->keyName, output, strlen(output)+1);
				} else {
					writeRet = -1;
				}
			else if(key->keyType == KEY_TYPE_BIN)
			{
				int ret = convert2bin(buf, output, strlen(output));
				if (ret > 0 && ret <= key->keySize) {
					writeRet = sceRegMgrSetKeyBin(writeDir->name, key->keyName, buf, ret);
				} else {
					writeRet = -1;
				}
			}

			writeDir = NULL;
			keyIndex = 0;
		}

		curSize = curDir->numSubDirs + curDir->numKeys + 1;
		drawScrollBar(topList[posInd], curSize);

		if(curDir->parent && !topList[posInd])
			vita2d_pgf_draw_text(pgf, 20, 4 * FONT_Y_SPACE, (!curPosList[posInd]) ? GREEN : CYAN, FONT_SIZE, "..");

		for(int i = topList[posInd]; i < curDir->numSubDirs && (i - topList[posInd]) < MAX_POSITION; i++)
		{
			uint32_t color = CYAN;
			int y = (5 * FONT_Y_SPACE) + ((i - topList[posInd]) * FONT_Y_SPACE);

			if(i == curPosList[posInd] - 1)
				color = GREEN;

			vita2d_pgf_draw_text(pgf, 20, y, color, FONT_SIZE, curDir->subdirs[i]->name + strlen(curDir->name));
		}

		// determine maximum text width for keyNames
		int max_x = 0;
		for(int i = (topList[posInd] < curDir->numSubDirs) ? 0 : topList[posInd] - curDir->numSubDirs; i < curDir->numKeys && (i - (topList[posInd] - curDir->numSubDirs)) < MAX_POSITION; i++)
		{
			uint32_t color = WHITE;
			int y = ((5 + curDir->numSubDirs) * FONT_Y_SPACE) + ((i - topList[posInd]) * FONT_Y_SPACE);

			if(i == curPosList[posInd] - 1 - curDir->numSubDirs)
				color = GREEN;

			int x = vita2d_pgf_draw_text(pgf, 20, y, color, FONT_SIZE, curDir->keys[i]->keyName);
			if (max_x < x) max_x = x;
		}

		// add some margin
		max_x += 20 + 20;

		for(int i = (topList[posInd] < curDir->numSubDirs) ? 0 : topList[posInd] - curDir->numSubDirs; i < curDir->numKeys && (i - (topList[posInd] - curDir->numSubDirs)) < MAX_POSITION; i++)
		{
			RegistryKey *key = curDir->keys[i];
			uint32_t color = WHITE;
			int y = ((5 + curDir->numSubDirs) * FONT_Y_SPACE) + ((i - topList[posInd]) * FONT_Y_SPACE);

			if(i == curPosList[posInd] - 1 - curDir->numSubDirs)
				color = GREEN;

			if(key->keyType == KEY_TYPE_INT)
			{
				ret = regMgrGetKeyInt(curDir->name, key->keyName);

				if(ret == 0x800D000E)
					vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, "Not supported on this device");
				else
					vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, (ret < 0) ? "Error %08X" : "%d", ret);
			}
			else if(key->keyType == KEY_TYPE_STR)
			{
				memset(buf, 0, 2048);
				ret = sceRegMgrGetKeyStr(curDir->name, key->keyName, buf, key->keySize);

				if(ret < 0)
				{
					if(ret == 0x800D000E)
						vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, "Not supported on this device");
					else
						vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, "Error %08X", ret);
				}
				else
					vita2d_pgf_draw_text(pgf, max_x, y, color, FONT_SIZE, buf);
			}
			else if(key->keyType == KEY_TYPE_BIN)
			{
				memset(buf, 0, 2048);
				ret = sceRegMgrGetKeyBin(curDir->name, key->keyName, buf, key->keySize);

				if(ret < 0)
				{
					if(ret == 0x800D000E)
						vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, "Not supported on this device");
					else
						vita2d_pgf_draw_textf(pgf, max_x, y, color, FONT_SIZE, "Error %08X", ret);
				}
				else
				{
					convert2hex(buf, buf2, key->keySize);
					vita2d_pgf_draw_text(pgf, max_x, y, color, FONT_SIZE, buf2);
				}
			}
		}

		getButtons(&holdButtons, &pressedButtons);

		if(pressedButtons & SCE_CTRL_START)
			break;

		if(holdButtons & SCE_CTRL_DOWN || holdButtons & SCE_CTRL_LEFT_ANALOG_DOWN)
		{			
			curPosList[posInd] = (curPosList[posInd] + 1) % curSize;

			if(curPosList[posInd] - topList[posInd] >= MAX_POSITION && curSize > MAX_POSITION && topList[posInd] < (curSize - MAX_POSITION - 1))
				topList[posInd]++;
			else if(curPosList[posInd] == 0 && curSize > MAX_POSITION)
				topList[posInd] = 0;
		}
		else if(holdButtons & SCE_CTRL_UP || holdButtons & SCE_CTRL_LEFT_ANALOG_UP)
		{
			if(--curPosList[posInd] < 0)
			{
				curPosList[posInd] = curSize - 1;

				if(curSize > MAX_POSITION)
				topList[posInd] = curSize - MAX_POSITION - 1;
			}
			
			if(curPosList[posInd] <= topList[posInd] && topList[posInd] && curSize > MAX_POSITION)
				topList[posInd]--;
		}

		if(pressedButtons & SCE_CTRL_CROSS)
		{
			if(curPosList[posInd] == 0)
			{
				if(curDir->parent) {
					posInd--;
					curDir = curDir->parent;
				}

				curPosList[posInd] = 0;
				topList[posInd] = 0;
			}
			else if(curPosList[posInd]- 1 < curDir->numSubDirs && curDir->numSubDirs > 0)
			{
				curDir = curDir->subdirs[curPosList[posInd] - 1];

				posInd++;
				curPosList[posInd] = 0;
				topList[posInd] = 0;
			}
			else
			{
				ret = -1;
				memset(buf2, 0, 2048);
				RegistryKey *key = curDir->keys[curPosList[posInd] - 1 - curDir->numSubDirs];

				if(key->keyType == KEY_TYPE_INT)
				{
					ret = regMgrGetKeyInt(curDir->name, key->keyName);
					sprintf(buf2, "%d", ret);
				}
				else if(key->keyType == KEY_TYPE_STR)
					ret = sceRegMgrGetKeyStr(curDir->name, key->keyName, buf2, key->keySize);
				else if(key->keyType == KEY_TYPE_BIN)
				{
					ret = sceRegMgrGetKeyBin(curDir->name, key->keyName, buf, key->keySize);
					if (ret == 0)
						convert2hex(buf, buf2, key->keySize);
				}

				if(ret >= 0)
				{
					initImeDialog(key->keyName, buf2, 2048);
					writeDir = curDir;
					keyIndex = curPosList[posInd] - 1 - curDir->numSubDirs;
				}

			}
		}
		else if(pressedButtons & SCE_CTRL_CIRCLE && curDir->parent)
		{
			curDir = curDir->parent;
			posInd--;
		}

		if(writeRet < 0)
			vita2d_pgf_draw_textf(pgf, 20, (23 * FONT_Y_SPACE), RED, FONT_SIZE, "Error writing to key: 0x%08X", writeRet);

		vita2d_end_drawing();
		vita2d_common_dialog_update();
		vita2d_swap_buffers();
		sceDisplayWaitVblankStart();
	}

	vita2d_fini();
	vita2d_free_pgf(pgf);
	freeRegistryDirectoryTree(&root);

	sceKernelExitProcess(0);
	return 0;
}
