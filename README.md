# College Notice Board — C Backend (HTML + CSS + C)

A notice board where the **backend is a C program**. It opens a network
socket, understands HTTP itself, and builds the web pages in C. No
JavaScript, no framework, no external libraries.

## Files

| File | What it does |
|---|---|
| `notice.h` / `notice.c` | The **data layer**: the `Notice` struct, and functions to load, add and delete notices in a binary file (`notices.dat`) |
| `http.h` / `http.c` | The **HTTP helpers**: socket setup, decoding form data, sending responses |
| `server.c` | The **main program**: accepts connections, reads each request, routes it, builds the HTML page |
| `public/style.css` | The look of the pages (CSS) |
| `build.bat` / `Makefile` | Build scripts (Windows / Linux) |

## Build and run — Windows

You need `gcc`. Easiest: download **w64devkit** (a zip, no installer) from
github.com/skeeto/w64devkit/releases, extract it, and run `w64devkit.exe`
to get a terminal with gcc. Then, in that terminal:

```
cd path\to\c-noticeboard
gcc server.c notice.c http.c -o noticeboard.exe -lws2_32
noticeboard.exe
```
(`build.bat` runs the gcc line for you.) Open **http://localhost:8080**.

## Build and run — Linux / AWS EC2

```
sudo apt-get install -y build-essential
make
./noticeboard
```

Admin login: `admin` / `admin123` at `/admin`. Run the program from inside
the project folder (it reads `public/style.css` and writes `notices.dat`
relative to where it runs).

## How a request travels (explain this in your viva)

1. `main()` in `server.c` calls `socket()`, `bind()`, `listen()` — this
   makes the program wait for browsers on port 8080.
2. `accept()` blocks until a browser connects, then `handleClient()` runs.
3. `recv()` reads the raw HTTP text, e.g. `GET /?category=Exam HTTP/1.1`.
4. `parseRequestLine()` splits that into **method** (`GET`), **path** (`/`)
   and **query** (`category=Exam`).
5. A chain of `if / else if` with `strcmp()` is the **router**: it picks
   what to do based on method + path.
6. The handler calls `loadNotices()` (reads structs from the file),
   loops through them, and uses `snprintf()` to write HTML into a buffer.
7. `sendResponse()` writes the HTTP header (`200 OK`, `Content-Length`)
   and the HTML body back with `send()`, then the socket is closed.

## Key concepts and where they are in the code

- **Structs** — `Notice` in `notice.h`: one record with id, title, etc.
- **Binary file handling** — `fwrite()` / `fread()` of whole structs in
  `notice.c` (`addNotice`, `loadNotices`).
- **Deleting from a file** — `deleteNotice()` copies every record except
  the one to delete into `notices.tmp`, then `remove()` + `rename()` swap it
  in. (Files can't delete the middle of themselves, so you rewrite.)
- **Strings** — `strncpy` with manual `'\0'` termination (prevents buffer
  overflow); `strcmp` for routing; `strchr`/`strstr` for parsing.
- **URL decoding** — `urlDecode()` in `http.c`: browsers send `+` for
  spaces and `%40` for `@`; this converts them back.
- **Preventing broken pages** — `appendEscaped()` turns `<`, `>`, `&` into
  `&lt;`, `&gt;`, `&amp;` so a notice can't inject HTML.
- **Conditional compilation** — `#ifdef _WIN32` in `http.h` picks Winsock
  on Windows and POSIX sockets on Linux, so the same code builds on both.
- **Dynamic memory** — `malloc()`/`free()` for the page buffer in
  `handleClient()` (too big for the stack).

## Cloud computing angle

- It is a **client–server, three-tier style** system: browser
  (presentation) → C server (application logic) → `notices.dat` (data).
- It is **deployable on AWS EC2**: copy the folder, `make`, run. Because
  it listens on `INADDR_ANY`, it accepts connections from the internet
  once the EC2 security group allows the port (open TCP 8080).
- Config note: the port (`PORT`) and admin login are `#define`s at the top
  of `server.c`.
- See `CLOUD-COMPUTING.md` and `AWS-DEPLOYMENT.md` from your earlier Node
  project — the cloud concepts (IaaS/PaaS/SaaS, NIST characteristics,
  security groups, scalability) apply equally here; only the run command
  changes (`./noticeboard` instead of `npm start`).

## Honest limitations (good to mention before your teacher asks)

- **Simple admin auth**: after login the password is passed in the URL
  (`?key=...`). Fine for a classroom demo; a real system would use
  hashed passwords and session cookies/tokens.
- **Handles one request at a time** (no threads), and reads each request
  with a single `recv()` — fine for small forms and demos.
- **Single data file**: two servers couldn't share it — a managed database
  (e.g. AWS RDS) would be the production answer.
- Max 200 notices (`MAX_NOTICES`), fixed field sizes (e.g. title 100 chars).
