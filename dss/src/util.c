#include "util.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void *memmem_case(const void *haystack, size_t hlen,
                  const void *needle, size_t nlen) {
    if (nlen == 0 || hlen < nlen) return NULL;
    const unsigned char *h = haystack;
    const unsigned char *n = needle;
    for (size_t i = 0; i <= hlen - nlen; i++) {
        size_t j;
        for (j = 0; j < nlen; j++) {
            unsigned char a = h[i+j];
            unsigned char b = n[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) break;
        }
        if (j == nlen) return (void*)(h + i);
    }
    return NULL;
}

void fmt_time(time_t t, char out[32]) {
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(out, 32, "%d.%m.%Y %H:%M:%S", &tm);
}

const char *path_basename(const char *path) {
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
}

void fmt_size(long long bytes, char out[32]) {
    double v;
    const char *u;
    if (bytes < 1024)              { v = (double)bytes;              u = "B";   }
    else if (bytes < 1024LL*1024)  { v = bytes / 1024.0;             u = "KiB"; }
    else if (bytes < 1024LL*1024*1024) { v = bytes / 1048576.0;      u = "MiB"; }
    else                           { v = bytes / 1073741824.0;       u = "GiB"; }
    snprintf(out, 32, "%lld bajtova (%.2f %s)", bytes, v, u);
}

int extract_pdf_string(const unsigned char *buf, size_t buflen,
                       const char *marker,
                       char *out, size_t outlen) {
    void *p = memmem_case(buf, buflen, marker, strlen(marker));
    if (!p) return 0;
    unsigned char *q = (unsigned char*)p + strlen(marker);
    /* Preskoči separator */
    while (q < buf + buflen && (*q == ' ' || *q == '(' || *q == '/' || *q == '['))
        q++;
    size_t i = 0;
    while (q < buf + buflen && i < outlen - 1 &&
           *q > 32 && *q != '/' && *q != '<' && *q != '>' &&
           *q != ')' && *q != ']' && *q != '\n') {
        /* PDF #XX escape */
        if (*q == '#' && q + 2 < buf + buflen &&
            isxdigit(q[1]) && isxdigit(q[2])) {
            int hi = isdigit(q[1]) ? q[1]-'0' : (q[1]|32)-'a'+10;
            int lo = isdigit(q[2]) ? q[2]-'0' : (q[2]|32)-'a'+10;
            out[i++] = (char)((hi<<4) | lo);
            q += 3;
        } else {
            out[i++] = *q++;
        }
    }
    out[i] = '\0';
    return (int)i;
}
