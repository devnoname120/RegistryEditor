#ifndef __REGS_H__
#define __REGS_H__

typedef struct RegistryKey
{
	char *keyPath;
	char *keyName;
	int keyType;
	int keySize;
} RegistryKey;

extern const int REGDATA_SIZE;

extern RegistryKey regData[];

#endif