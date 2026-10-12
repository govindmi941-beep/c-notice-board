#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"
#include "notice.h"

#define DEFAULT_PORT 8080
#define ADMIN_USER "admin"
#define DEFAULT_ADMIN_PASS "admin123"
/* Set from the ADMIN_PASS environment variable in main(), so a cloud
   deployment can use a private password instead of the default. */
static const char *ADMIN_PASS = DEFAULT_ADMIN_PASS;

#define BUF_SIZE 16384
#define PAGE_SIZE 262144

/* --------- small helper: writes HTML-safe text (so a notice containing
   < > & doesn't break the page) into a growing buffer --------- */
static void appendEscaped(char *out, size_t outSize, size_t *pos, const char *text) {
    for (size_t i = 0; text[i] != '\0' && *pos < outSize - 1; i++) {
        const char *rep = NULL;
        switch (text[i]) {
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '&': rep = "&amp;"; break;
        }
        if (rep) {
            size_t len = strlen(rep);
            if (*pos + len >= outSize - 1) break;
            memcpy(out + *pos, rep, len);
            *pos += len;
        } else {
            out[(*pos)++] = text[i];
        }
    }
    out[*pos] = '\0';
}

/* --------- builds the page every student sees --------- */
static void renderBoard(char *out, size_t outSize, const char *categoryFilter) {
    Notice list[MAX_NOTICES];
    int count = loadNotices(list, MAX_NOTICES);
    size_t pos = 0;

    pos += snprintf(out + pos, outSize - pos,
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Notice Board (C backend)</title>"
        "<link rel='stylesheet' href='/style.css'></head><body>"
        "<header><h1>College Notice Board</h1>"
        "<p class='tagline'>Served entirely by a C program &mdash; no JavaScript, no framework.</p>"
        "<a class='admin-link' href='/admin'>Admin sign in</a></header>"
        "<main><nav class='chips'>"
        "<a class='chip%s' href='/'>All</a>"
        "<a class='chip%s' href='/?category=Academic'>Academic</a>"
        "<a class='chip%s' href='/?category=Exam'>Exam</a>"
        "<a class='chip%s' href='/?category=Event'>Event</a>"
        "<a class='chip%s' href='/?category=Scholarship'>Scholarship</a>"
        "<a class='chip%s' href='/?category=General'>General</a>"
        "</nav><section class='board'>",
        (categoryFilter[0] == '\0') ? " active" : "",
        (strcmp(categoryFilter, "Academic") == 0) ? " active" : "",
        (strcmp(categoryFilter, "Exam") == 0) ? " active" : "",
        (strcmp(categoryFilter, "Event") == 0) ? " active" : "",
        (strcmp(categoryFilter, "Scholarship") == 0) ? " active" : "",
        (strcmp(categoryFilter, "General") == 0) ? " active" : "");

    int shown = 0;
    for (int i = count - 1; i >= 0; i--) { /* newest first */
        if (categoryFilter[0] != '\0' && strcmp(list[i].category, categoryFilter) != 0) continue;
        shown++;

        pos += snprintf(out + pos, outSize - pos,
            "<article class='notice%s'><div class='notice-head'>"
            "<span class='tag'>%s</span>%s</div><h2>",
            (strcmp(list[i].priority, "Urgent") == 0) ? " urgent" : "",
            list[i].category,
            (strcmp(list[i].priority, "Urgent") == 0) ? "<span class='urgent-badge'>Urgent</span>" : "");
        appendEscaped(out, outSize, &pos, list[i].title);
        pos += snprintf(out + pos, outSize - pos, "</h2><p>");
        appendEscaped(out, outSize, &pos, list[i].content);
        pos += snprintf(out + pos, outSize - pos,
            "</p><div class='notice-foot'><span>%s</span><span>%s</span></div></article>",
            list[i].posted_by, list[i].date);
    }

    if (shown == 0) {
        pos += snprintf(out + pos, outSize - pos, "<p class='empty'>No notices in this category yet.</p>");
    }

    pos += snprintf(out + pos, outSize - pos,
        "</section></main>"
        "<footer><p>Cloud Computing project &mdash; backend written in C (raw sockets, struct-based binary file storage, no external libraries).</p></footer>"
        "</body></html>");
}

/* --------- builds the admin page: login form, or dashboard if logged in --------- */
static void renderAdmin(char *out, size_t outSize, int loggedIn, int showError) {
    size_t pos = 0;
    pos += snprintf(out + pos, outSize - pos,
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Admin - Notice Board</title><link rel='stylesheet' href='/style.css'></head><body>"
        "<header><h1>College Notice Board</h1><p class='tagline'>Admin panel</p>"
        "<a class='admin-link' href='/'>View board</a></header><main>");

    if (!loggedIn) {
        pos += snprintf(out + pos, outSize - pos,
            "<section class='panel'><h2>Sign in</h2>"
            "%s"
            "<form method='POST' action='/login'>"
            "<label>Username</label><input name='username' required>"
            "<label>Password</label><input name='password' type='password' required>"
            "<button type='submit'>Sign in</button>"
            "</form>%s</section>",
            showError ? "<p class='error'>Incorrect username or password.</p>" : "",
            (strcmp(ADMIN_PASS, DEFAULT_ADMIN_PASS) == 0)
                ? "<p class='hint'>Default login: <code>admin</code> / <code>admin123</code></p>" : "");
    } else {
        pos += snprintf(out + pos, outSize - pos,
            "<section class='panel'><h2>Post a notice</h2>"
            "<form method='POST' action='/add?key=%s'>"
            "<label>Title</label><input name='title' required maxlength='99'>"
            "<label>Content</label><textarea name='content' rows='3' required></textarea>"
            "<label>Category</label><select name='category'>"
            "<option>Academic</option><option>Exam</option><option>Event</option>"
            "<option>Scholarship</option><option>General</option></select>"
            "<label>Priority</label><select name='priority'><option>Normal</option><option>Urgent</option></select>"
            "<label>Posted by</label><input name='posted_by' value='Administration'>"
            "<button type='submit'>Publish notice</button>"
            "</form></section><section class='panel'><h2>Existing notices</h2>",
            ADMIN_PASS);

        Notice list[MAX_NOTICES];
        int count = loadNotices(list, MAX_NOTICES);
        for (int i = count - 1; i >= 0; i--) {
            pos += snprintf(out + pos, outSize - pos, "<div class='admin-row'><span class='tag'>%s</span><strong>",
                list[i].category);
            appendEscaped(out, outSize, &pos, list[i].title);
            pos += snprintf(out + pos, outSize - pos,
                "</strong><form method='POST' action='/delete?key=%s&amp;id=%d'>"
                "<button type='submit' class='btn-delete'>Delete</button></form></div>",
                ADMIN_PASS, list[i].id);
        }
        pos += snprintf(out + pos, outSize - pos, "</section>");
    }

    pos += snprintf(out + pos, outSize - pos, "</main></body></html>");
}

/* --------- parses "GET /path?query HTTP/1.1" into method/path/query --------- */
static void parseRequestLine(const char *line, char *method, char *path, char *query) {
    char fullPath[512];
    sscanf(line, "%15s %511s", method, fullPath);

    char *q = strchr(fullPath, '?');
    if (q) {
        *q = '\0';
        strncpy(query, q + 1, 1023);
        query[1023] = '\0';
    } else {
        query[0] = '\0';
    }
    strncpy(path, fullPath, 255);
    path[255] = '\0';
}

static void handleClient(SOCK_T client) {
    char request[BUF_SIZE];
    int received = recv(client, request, BUF_SIZE - 1, 0);
    if (received <= 0) { CLOSESOCK(client); return; }
    request[received] = '\0';

    /* A browser can send the headers and the form body in separate network
       packets, so one recv() may only give us the headers. Keep reading until
       the whole body has arrived. The Content-Length header tells us how many
       body bytes to expect. */
    char *headerEnd = strstr(request, "\r\n\r\n");
    while (headerEnd == NULL && received < BUF_SIZE - 1) {
        int more = recv(client, request + received, BUF_SIZE - 1 - received, 0);
        if (more <= 0) break;
        received += more;
        request[received] = '\0';
        headerEnd = strstr(request, "\r\n\r\n");
    }
    if (headerEnd != NULL) {
        int expected = 0;
        char *cl = strstr(request, "Content-Length:");
        if (cl == NULL) cl = strstr(request, "content-length:");
        if (cl != NULL) expected = atoi(cl + 15);

        int haveBody = received - (int)((headerEnd + 4) - request);
        while (haveBody < expected && received < BUF_SIZE - 1) {
            int more = recv(client, request + received, BUF_SIZE - 1 - received, 0);
            if (more <= 0) break;
            received += more;
            haveBody += more;
            request[received] = '\0';
        }
    }

    char method[16] = "", path[256] = "", query[1024] = "";
    parseRequestLine(request, method, path, query);

    /* The request body (for POST) comes after the blank line "\r\n\r\n" */
    char *body = strstr(request, "\r\n\r\n");
    body = body ? body + 4 : "";

    printf("%s %s\n", method, path);

    char *page = malloc(PAGE_SIZE);

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        char category[64] = "";
        getParam(query, "category", category, sizeof(category));
        renderBoard(page, PAGE_SIZE, category);
        sendResponse(client, "200 OK", "text/html", page);

    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/style.css") == 0) {
        FILE *fp = fopen("public/style.css", "rb");
        if (fp) {
            size_t len = fread(page, 1, PAGE_SIZE - 1, fp);
            page[len] = '\0';
            fclose(fp);
            sendResponse(client, "200 OK", "text/css", page);
        } else {
            sendResponse(client, "404 Not Found", "text/plain", "style.css not found");
        }

    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/admin") == 0) {
        char key[64] = "", err[8] = "";
        getParam(query, "key", key, sizeof(key));
        getParam(query, "error", err, sizeof(err));
        int loggedIn = (strcmp(key, ADMIN_PASS) == 0);
        renderAdmin(page, PAGE_SIZE, loggedIn, err[0] != '\0');
        sendResponse(client, "200 OK", "text/html", page);

    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/login") == 0) {
        char user[64] = "", pass[64] = "";
        getParam(body, "username", user, sizeof(user));
        getParam(body, "password", pass, sizeof(pass));
        if (strcmp(user, ADMIN_USER) == 0 && strcmp(pass, ADMIN_PASS) == 0) {
            char loc[128];
            snprintf(loc, sizeof(loc), "/admin?key=%s", ADMIN_PASS);
            sendRedirect(client, loc);
        } else {
            sendRedirect(client, "/admin?error=1");
        }

    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/add") == 0) {
        char key[64] = "";
        getParam(query, "key", key, sizeof(key));
        if (strcmp(key, ADMIN_PASS) != 0) {
            sendResponse(client, "403 Forbidden", "text/plain", "Admin login required.");
        } else {
            char title[MAX_TITLE] = "", content[MAX_CONTENT] = "", category[MAX_CATEGORY] = "General",
                 priority[MAX_PRIORITY] = "Normal", postedBy[MAX_POSTEDBY] = "Administration";
            getParam(body, "title", title, sizeof(title));
            getParam(body, "content", content, sizeof(content));
            getParam(body, "category", category, sizeof(category));
            getParam(body, "priority", priority, sizeof(priority));
            getParam(body, "posted_by", postedBy, sizeof(postedBy));
            addNotice(title, content, category, priority, postedBy);
            char loc[128];
            snprintf(loc, sizeof(loc), "/admin?key=%s", ADMIN_PASS);
            sendRedirect(client, loc);
        }

    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/delete") == 0) {
        char key[64] = "", idStr[16] = "";
        getParam(query, "key", key, sizeof(key));
        getParam(query, "id", idStr, sizeof(idStr));
        if (strcmp(key, ADMIN_PASS) != 0) {
            sendResponse(client, "403 Forbidden", "text/plain", "Admin login required.");
        } else {
            deleteNotice(atoi(idStr));
            char loc[128];
            snprintf(loc, sizeof(loc), "/admin?key=%s", ADMIN_PASS);
            sendRedirect(client, loc);
        }

    } else {
        sendResponse(client, "404 Not Found", "text/plain", "Not found");
    }

    free(page);
    CLOSESOCK(client);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0); /* print log lines immediately */
    platformSocketInit();
    ensureDataFile();

    /* Cloud platforms tell the program which port to use via the PORT
       environment variable. On your laptop it falls back to 8080. */
    int port = DEFAULT_PORT;
    const char *envPort = getenv("PORT");
    if (envPort != NULL && atoi(envPort) > 0) port = atoi(envPort);
    const char *envPass = getenv("ADMIN_PASS");
    if (envPass != NULL && envPass[0] != '\0') ADMIN_PASS = envPass;

    SOCK_T server = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)port);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Could not bind to port %d - is it already in use?\n", port);
        return 1;
    }
    listen(server, 10);

    printf("College Notice Board (C backend) running at http://localhost:%d\n", port);
    printf("Admin panel at http://localhost:%d/admin\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCK_T client = accept(server, (struct sockaddr *)&clientAddr, &clientLen);
        if (client == BAD_SOCKET) continue;
        handleClient(client);
    }

    CLOSESOCK(server);
    platformSocketCleanup();
    return 0;
}
