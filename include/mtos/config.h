#ifndef MTOS_CONFIG_H
#define MTOS_CONFIG_H

#define MTOS_CONFIG_H_IN

#ifdef PROJECT_CONFIG_FILE
#include PROJECT_CONFIG_FILE
#endif

#include <mtos/default-config.h>

#undef MTOS_CONFIG_H_IN

#endif // MTOS_CONFIG_H
