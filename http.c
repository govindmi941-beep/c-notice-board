#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "http.h"

void platformSocketInit(void) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void platformSocketCleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static int hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void urlDecode(const char *src, char *dst, size_t dstSize) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dstSize - 1; i++) {
        if (src[i] == '+') {
            dst[j++] = ' ';
        } else if (src[i] == '%' && src[i + 1] && src[i + 2]) {
            dst[j++] = (char)(hexToInt(src[i + 1]) * 16 + hexToInt(src[i + 2]));
            i += 2;
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

int getParam(const char *data, const char *key, char *out, size_t outSize) {
    out[0] = '\0';
    if (data == NULL) return 0;

    size_t keyLen = strlen(key);
    const char *p = data;

    while (*p) {
        /* p now points at the start of a "key=value" pair */
        if (strncmp(p, key, keyLen) == 0 && p[keyLen] == '=') {
            const char *valueStart = p + keyLen + 1;
            const char *valueEnd = strchr(valueStart, '&');
            size_t len = valueEnd ? (size_t)(valueEnd - valueStart) : strlen(valueStart);

            char raw[1024];
            if (len >= sizeof(raw)) len = sizeof(raw) - 1;
            strncpy(raw, valueStart, len);
            raw[len] = '\0';

            urlDecode(raw, out, outSize);
            return 1;
        }
        /* skip to the next "&key=value" pair */
        const char *amp = strchr(p, '&');
        if (!amp) break;
        p = amp + 1;
    }
    return 0;
}

void sendResponse(SOCK_T client, const char *status, const char *contentType, const char *body) {
    char header[512];
    size_t bodyLen = strlen(body);
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, contentType, (unsigned long)bodyLen);

    send(client, header, (int)strlen(header), 0);
    send(client, body, (int)bodyLen, 0);
}

void sendRedirect(SOCK_T client, const char *location) {
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 302 Found\r\n"
        "Location: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        location);
    send(client, header, (int)strlen(header), 0);
}
