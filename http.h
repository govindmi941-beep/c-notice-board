#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

/* Windows and Linux have slightly different socket APIs. This block picks
   the right headers and gives both platforms the same type/function names
   (SOCK_T, CLOSESOCK) so the rest of the program doesn't need to care
   which OS it's running on. */
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SOCK_T;
    #define CLOSESOCK closesocket
    #define BAD_SOCKET INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    typedef int SOCK_T;
    #define CLOSESOCK close
    #define BAD_SOCKET (-1)
#endif

void platformSocketInit(void);
void platformSocketCleanup(void);

/* Decodes application/x-www-form-urlencoded text: '+' becomes a space,
   and %XX becomes the character with that hex code (e.g. %40 -> '@').
   This is how browsers encode form data and query strings. */
void urlDecode(const char *src, char *dst, size_t dstSize);

/* Finds "key=value" inside a query string or form body like
   "title=Hello&category=Exam" and copies the decoded value into out.
   Returns 1 if found, 0 if the key wasn't present. */
int getParam(const char *data, const char *key, char *out, size_t outSize);

/* Sends a complete HTTP response (status line + headers + body) down the socket. */
void sendResponse(SOCK_T client, const char *status, const char *contentType, const char *body);

/* Sends a 302 redirect to the given path. */
void sendRedirect(SOCK_T client, const char *location);

#endif
