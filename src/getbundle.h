/**
 * File              : getbundle.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 07.10.2022
 * Last Modified Date: 05.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/*
 * Get bundle of application
 * for mac os - APP/Contents/Resources,
 * for win - executable directory
 * for linux/unix - /usr/[local]/share/APP
 */

#ifndef k_lib_getbundle_h__
#define k_lib_getbundle_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> //dirname
#if defined __APPLE__
#elif defined _WIN32
#include <Windows.h>
#else
#include <unistd.h> //readlink
#endif

static char *
getbundle(char *argv[]) {
  if (!argv || !argv[0])
    return NULL;
#ifdef _WIN32
  return dirname((char *)argv[0]);
#else
  char *bundle = (char *)malloc(BUFSIZ);
  if (!bundle)
    return NULL;
#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
  sprintf(bundle, "%s%s", dirname((char *)argv[0]), "/../Resources");
  return bundle;
#else
  free(bundle);
  return dirname((char *)argv[0]);
#endif
#else
  char selfpath[128];
  if (readlink("/proc/self/exe", selfpath, sizeof(selfpath) - 1) < 0) {
    free(bundle);
    return NULL;
  }
  sprintf(bundle, "%s/../share/%s", dirname(selfpath), basename(argv[0]));
  return bundle;
#endif
#endif
  return NULL;
}

#ifdef __cplusplus
}
#endif
#endif // k_lib_getbundle_h__
