/**
 * File              : strfind.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.02.2022
 * Last Modified Date: 05.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**Usage:
 *to replace all matches:
 *		  char * new_string = strarep(old_string, search_pattern,
 *replace_string);
 *to replace from string position:
 *		  char * new_string = strrep(old_string, position, length,
 *replace_string);
 *to find position of first match:
 *		  int pos = strfind(string, search_pattern)
 *to count matches:
 *		  int count = strcnt(string, search_pattern)
 */

#ifndef k_lib_strfnd_h__
#define k_lib_strfnd_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// check haystack does starts with and include needle - return position of
// pointer haystack + needle_len or NULL
static char *strinc(const char *haystack, const char *needle);

// find needle in haystack and return its position
static long strfnd(const char *haystack, const char *needle);
#define strfind(haystack, needle) strfnd(haystack, needle)

// find one of strings of null-terminated array in haystack and callback
// it's position in haystack and index in array for eatch match
// add '*pos += len;' in callback function to increase search algorithm speed
static void
strfnda(const char *haystack, // string where search needle
        const char *needle[], // null-terminated array of search strings
        void *userdata, // data to transfer via callback
        int (*callback)( /// callback - return non-zero to stop function
            void *userdata, // data transfered via callback
            size_t *pos, // position of founded needle in haystack
            int index, // index of founded needle in array
            int len // string length of founded needle
            ));

// return count of needle matches in haystack
static int strcnt(const char *haystack, const char *needle);

// return new allocated string with contents of haystack replaced from position
// to position+len by replace string
static char *strrep(const char *haystack, size_t pos, size_t len,
                    const char *replace);

// return new allocated string with contents of haystack and inserted needle in
// pos position
static char *strins(const char *haystack, const char *needle, int pos);

// return new allocated string with contents of haystack replaced all needle
// matches by replace string
static char *strarep(const char *haystack, const char *needle,
                     const char *replace);

// create out file copy of in file with all needle replaced by replace and
// return it's count
static int fstrrep(FILE *in, FILE *out, const char *needle,
                   const char *replace);

// fstrep with filepaths. return -1 if file_in access error, -2 if file_out
// access error
static int fpstrrep(const char *file_in, const char *file_out,
                    const char *needle, const char *replace);

// iterate file content and callback buffer with buffer len, file position and
// founded needle
// if needle has founded - buffer is needle
static void fstrfnd(FILE *file, const char *needle, void *userdata,
                    int (*callback)(void *userdata, off_t *pos, bool found,
                                    int len, char *buf));
/**
 * implimation of functions
 */

char *strinc(const char *haystack, const char *needle) {
  char *hp = (char *)haystack; // haystack pointer
  char *np = (char *)needle; // needle pointer

  // for each char in neddle check if matches haystack
  while (*np)
    if (*hp++ != *np++)
      return NULL;

  return hp++; // return pointer next to needle len
}

long strfnd(const char *haystack, const char *needle) {
  // find position of search word in haystack
  const char *p = strstr(haystack, needle);
  if (p)
    return p - haystack;
  return -1;
}

int strcnt(const char *haystack, // string to scan
           const char *needle // search pattern
           ) {
  char *ptr = (char *)haystack; // haystack pointer
  int i = 0, cnt = 0;
  while (*ptr) {
    char *p = strinc(ptr, needle);
    if (p) {
      cnt++;
      ptr = p;
    }
    i++;
    ptr++;
  }

  return cnt;
}

char *strrep(const char *haystack, // string
             size_t pos, // start position
             size_t len, // length of replace string
             const char *replace // replace string
             ) {
  // find len of string
  size_t stringlen = strlen(haystack);
  size_t replacelen = strlen(replace);

  // allocate result string with enough memory
  char *result = (char *)malloc(stringlen - len + replacelen + 1);
  if (result == NULL)
    return NULL;

  // replace string
  int i = 0;
  while (*haystack) {
    if (i == pos) { // if found word
      strncat(result, replace, replacelen); // copy new string to result
      i += replacelen; // move result counter
      haystack += len; // move read string counter
    } else {
      result[i++] = *haystack++;
    }
  }
  result[i] = '\0';
  return result;
}

char *strins(const char *haystack, const char *needle, int pos) {
  char *hp = (char *)haystack; // haystack pointer
  char *np = (char *)needle; // needle pointer

  char *str = (char *)malloc(strlen(haystack) + strlen(needle) +
                             1); // allocate enough memory
  if (str == NULL)
    return NULL;

  int i = 0;
  while (*hp) {
    if (i == pos)
      while (*np)
        str[i++] = *np++;
    str[i++] = *hp++;
  }
  str[i++] = 0;

  return str;
}

char * // return result string
    strarep(const char *haystack, // string to scan
            const char *needle, // search pattern
            const char *replace // replace string
            ) {

  size_t needlelen = strlen(needle);
  size_t replacelen = strlen(replace);

  char *ptr = (char *)haystack; // haystack pointer

  // count cases of search word in haystack and length of haystack
  int i = 0, cnt = 0;
  while (*ptr) {
    char *p = strinc(ptr, needle);
    if (p) {
      cnt++;
      ptr = p;
    } else
      ptr++;
  }

  // allocate result string with enough memory
  char *res = (char *)malloc(i + cnt * (replacelen - needlelen) + 1);
  if (res == NULL)
    return NULL;

  // replace search needle string with replace string
  ptr = (char *)haystack; // haystack pointer
  i = 0;
  while (*ptr) {
    char *p = strinc(ptr, needle);
    if (p) {
      strncat(res, replace, replacelen);
      ptr = p;
    } else
      res[i++] = *ptr++;
  }
  res[i++] = 0;
  return res;
}

int strmatch(char *haystack, char *needle) {
  int len = 0;
  while (*needle) {
    len++;
    if (*needle++ != *haystack++)
      return 0;
  }
  return len;
}

void strfnda(const char *haystack, const char *needle[], void *userdata,
             int (*callback)(void *userdata, size_t *pos, int index, int len)) {
  char *hp = (char *)haystack; // pointer to haystack
  char **np = (char **)needle; // pointer to needle
  size_t pos = 0; // iterator
  while (hp[pos]) { // iterate haystack
  iterate:
    ;
    if (*hp == 32) { // if new word
      np = (char **)needle; // pointer to needle - update
    }

    int index = 0; // iterator
    while (np[index]) { // iterate search words
      char *npp = np[index];
      char *hpp = &hp[pos];
      int len = strmatch(hpp, npp);
      if (len) {
        // callback result - stop if return non zero
        if (callback)
          if (callback(userdata, &pos, index, len))
            return;
        pos++;
        goto iterate;
      }
      index++;
    }
    pos++;
  }
}

void fstrfnd(FILE *file, const char *needle, void *userdata,
             int (*callback)(void *userdata, off_t *pos, bool found, int len,
                             char *buf)) {
  int nlen = strlen(needle);
  char buf[nlen];

  int buflen = 0; // buf len
  int i = 0; // iterator

  off_t pos = 0; // file position

  // clear buf
  for (i = 0; i < nlen; i++)
    buf[i] = 0;

  while (1) {
    char ch = fgetc(file);
    if ((int)ch == EOF) {
      break;
    }

    if (ch == needle[buflen]) {
      buf[buflen++] = ch;
      if (buflen == nlen) {
        // callback founded
        if (callback)
          if (callback(userdata, &pos, true, buflen, buf))
            break;
      }
      // clear buf
      for (i = 0; i < buflen; ++i) {
        buf[i] = 0;
      }
    } else {
      if (buflen > 0) {
        // callback buff
        if (callback)
          if (callback(userdata, &pos, false, buflen, buf))
            break;
        // clear buf
        for (i = 0; i < buflen; ++i) {
          buf[i] = 0;
        }
      }
      buflen = 0;
      // callback ch
      if (callback)
        if (callback(userdata, &pos, false, 1, &ch))
          break;
    }
    pos++;
  }
}

typedef struct fstrfnda_list {
  int index;
  size_t pos;
  int len;
  struct fstrfnda_list *next;
} fstrfnda_list;

fstrfnda_list *fstrfnda_list_new() {
  fstrfnda_list *list = malloc(sizeof(fstrfnda_list));
  if (!list)
    return NULL;
  list->next = NULL;
  return list;
}

void fstrfnda_list_add(fstrfnda_list *list, size_t *pos, int *index, int *len) {
  fstrfnda_list *ptr = list;
  while (ptr->next)
    ptr++;

  fstrfnda_list *new_list = fstrfnda_list_new();
  if (!new_list)
    return;

  ptr->next = new_list;
  ptr->pos = *pos;
  ptr->index = *index;
  ptr->len = *len;
}

void fstrfnda_list_free(fstrfnda_list *list) {
  fstrfnda_list *ptr = list;
  while (ptr) {
    fstrfnda_list *next = ptr->next;
    free(ptr);
    ptr = next;
  }
}

void fstrfnda(FILE *file, const char *needle[], void *userdata,
              int (*callback)(void *userdata, off_t pos, fstrfnda_list *list,
                              char *buf)) {
  char **np = (char **)needle; // pointer to needle array

  int minlen = BUFSIZ;
  int maxlen = 0;
  int size = 0;

  // count len
  while (*np) {
    int len = strlen(*np++);
    if (len > maxlen)
      maxlen = len;
    if (len < minlen)
      minlen = len;
    size++;
  }

  char *buf = NULL; // main buffer
  char *pbuf = NULL; // previous buffer
  char *fbuf = NULL; // fix buffer - to find
  // words splitted by main buffers

  off_t pos = 0; // file position
  off_t ppos = 0; // prev buffer file position
  off_t fpos = 0; // fix  buffer file position

  int buflen = 0; // buf len
  int fbuflen = 0; // fix buf len
  int pbuflen = 0; // pbuf len
  int i = 0; // iterator

  bool eof = false;
  while (1) {
    char ch = fgetc(file);
    if ((int)ch == EOF) {
      eof = true;
      goto makesearch;
    }

    // start new buffer
    if (buflen == 0) {
      pbuf = buf;
      buf = malloc(BUFSIZ * sizeof(char));
      if (!buf)
        break;
      for (i = 0; i < BUFSIZ; i++)
        buf[i] = 0;
    }

    // fill main buffer
    if (buflen < BUFSIZ)
      buf[buflen++] = ch;

    if (buflen == BUFSIZ) {
      // start new buffer
      pbuflen = buflen;
      ppos = pos;
      buflen = 0;
    }

    // we need fix buffer to find matches if search string is splitted by main
    // buffer
    // start fix buffer at the end of main and at start on next buffer
    if (buflen == BUFSIZ - maxlen) { // start fix buffer
      // clear fbuf
      if (fbuf)
        free(fbuf);
      fbuf = malloc(2 * maxlen * sizeof(char));
      if (!fbuf)
        break;
      for (i = 0; i < 2 * maxlen; i++)
        buf[i] = 0;
      fbuflen = 0;
      fbuf[fbuflen++] = ch;
    }

    if (buflen < maxlen && buflen > BUFSIZ - maxlen) {
      if (fbuf)
        fbuf[fbuflen++] = ch;
    }

    if (buflen == maxlen) { // check if there is matches in fbuf
    makesearch:
      ;
      fpos = pos;
      if (fbuf) {
        // list of matches
        fstrfnda_list *flist = fstrfnda_list_new();
        // scan buffer
        strfnda(fbuf, needle, flist, fstrfnda_cb);

        if (flist->next) { // we have string matches
          // scan and callback prev buffer - maxlen
          if (pbuf) {
            char *ptr = pbuf;
            ptr[pbuflen - maxlen] = 0;
            fstrfnda_list *list = fstrfnda_list_new();
            strfnda(ptr, needle, list, fstrfnda_cb);
            if (callback)
              if (callback(userdata, ppos - maxlen, list, ptr))
                break;
            free(pbuf);
          }

          // callback fixbuffer
          if (callback)
            if (callback(userdata, fpos, flist, fbuf))
              break;

          // free fixbuffer
          free(fbuf);
          fbuflen = 0;

          // free main buffer
          if (!eof) {
            if (buf)
              free(buf);
            buflen = 0;
          }
        } else {
          // scan and callback prev buffer
          if (pbuf) {
            fstrfnda_list *list = fstrfnda_list_new();
            strfnda(pbuf, needle, list, fstrfnda_cb);
            if (callback)
              if (callback(userdata, ppos, list, pbuf))
                break;
            free(pbuf);
            pbuf = NULL;
          }
          // free fixbuffer
          free(fbuf);
          fbuflen = 0;
        }
      }
      if (eof) {
        // scan and callback main buffer
        fstrfnda_list *list = fstrfnda_list_new();
        strfnda(buf, needle, list, fstrfnda_cb);
        if (callback)
          callback(userdata, ppos - maxlen, list, buf);

        free(buf);
        break;
      }
    }

    pos++;
  }
}

int fstrrep(FILE *in, FILE *out, const char *needle, const char *replace) {
  int nlen = strlen(needle);
  int rlen = strlen(replace);

  char buf[nlen];

  int i, cnt = 0;

  // clear buf
  for (int i = 0; i < nlen; i++)
    buf[i] = 0;

  i = 0;

  while (1) {
    char ch = fgetc(in);
    if (ch == EOF) {
      break;
    }

    if (ch == needle[i]) {
      buf[i] = ch;
      i++;
      if (i == nlen) {
        // found string matches
        cnt++;

        // replace string
        for (i = 0; i < rlen; ++i) {
          fputc(replace[i], out);
        }
        i = 0;
      }
    } else {
      if (i > 0) {
        // print buff
        int c = i;
        for (i = 0; i < c; ++i) {
          fputc(buf[i], out);
          // clear buf
          buf[i] = 0;
        }
      }
      i = 0;
      fputc(ch, out);
    }
  }

  return cnt;
}

int fpstrrep(const char *file_in, const char *file_out, const char *needle,
             const char *replace) {
  FILE *in = fopen(file_in, "r");
  if (!in)
    return -1;

  FILE *out = fopen(file_out, "w");
  if (!out) {
    fclose(in);
    return -2;
  }

  int cnt = fstrrep(in, out, needle, replace);

  fclose(in);
  fclose(out);

  return cnt;
}

#ifdef __cplusplus
}
#endif

#endif // k_lib_strfnd_h__
