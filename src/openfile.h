/**
 * File              : openfile.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 18.09.2021
 * Last Modified Date: 21.01.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/*
 * open file at path with default program
 */

#ifndef k_lib_openfile_h__
#define k_lib_openfile_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int openfile(const char *path) {
#ifdef __APPLE__
#include <objc/objc-runtime.h>
  id str = ((id (*)(Class, SEL, const char *))objc_msgSend)(
      objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"),
      path);
  id url = ((id (*)(Class, SEL, id))objc_msgSend)(
      objc_getClass("NSURL"), sel_registerName("fileURLWithPath:"), str);
#include <TargetConditionals.h>
#if TARGET_OS_MAC
  id ws = ((id (*)(Class, SEL))objc_msgSend)(
      objc_getClass("NSWorkspace"), sel_registerName("sharedWorkspace"));
  ((void (*)(id, SEL, id))objc_msgSend)(ws, sel_registerName("openURL:"), url);
#else
  id app = ((id (*)(Class, SEL))objc_msgSend)(
      objc_getClass("UIApplication"), sel_registerName("sharedApplication"));
  id opt = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSDictionary"),
                                              sel_registerName("dictionary"));
  ((void (*)(id, SEL, id, id, id))objc_msgSend)(
      app, sel_registerName("openURL:options:completionHandler:"), url, opt,
      NULL);
#endif
#elif defined _WIN32
#include <windows.h>
  ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#else
#include <stdlib.h>
#include <strings.h>
  char open_file_command[BUFSIZ];
  sprintf(open_file_command, "xdg-open %s", path);
  system(open_file_command);
#endif

  return 0;
}

#ifdef __cplusplus
}
#endif

#endif // k_lib_openfile_h__
