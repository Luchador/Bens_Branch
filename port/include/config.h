#pragma once

#include <stdint.h>

#define CONFIG_FNAME "pd.ini"
#define CONFIG_PATH "$S/" CONFIG_FNAME

void configInit(void);

// loads config from file (path extensions such as ! apply)
int configLoad(const char *fname);

// saves config to file (path extensions such as ! apply)
int configSave(const char *fname);

// registers a variable in the config file
// this should be done before configInit() is called, preferably in a module constructor
void configRegisterInt(const char *key, int *var, int min, int max);
void configRegisterUInt(const char* key, uint32_t* var, uint32_t min, uint32_t max);
void configRegisterFloat(const char *key, float *var, float min, float max);
void configRegisterString(const char *key, char *var, uint32_t maxstr);
