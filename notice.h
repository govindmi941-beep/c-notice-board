#ifndef NOTICE_H
#define NOTICE_H

#include <stddef.h>

#define MAX_TITLE     100
#define MAX_CONTENT   500
#define MAX_CATEGORY  30
#define MAX_PRIORITY  20
#define MAX_POSTEDBY  50
#define MAX_DATE      20
#define MAX_NOTICES   200
#define DATA_FILE     "notices.dat"

/* One notice = one fixed-size record. Storing these directly as a struct
   in a binary file (instead of parsing text) is the classic C approach:
   fwrite()/fread() copy the struct's raw bytes, so no text parsing is
   needed to save or load a notice. */
typedef struct {
    int  id;
    char title[MAX_TITLE];
    char content[MAX_CONTENT];
    char category[MAX_CATEGORY];
    char priority[MAX_PRIORITY];
    char posted_by[MAX_POSTEDBY];
    char date[MAX_DATE];
} Notice;

/* Creates notices.dat with 3 sample notices if it doesn't exist yet. */
void ensureDataFile(void);

/* Reads every notice from notices.dat into list[]. Returns how many were
   read (0 if the file is empty or missing). maxCount stops it reading
   past the end of the array the caller gave it. */
int loadNotices(Notice list[], int maxCount);

/* Appends one new notice to notices.dat. Figures out the next id itself
   by finding the current highest id and adding 1. */
int addNotice(const char *title, const char *content, const char *category,
              const char *priority, const char *posted_by);

/* Removes the notice with the given id. Internally: read every notice,
   write all EXCEPT the matching one to a temp file, then replace the
   original file with the temp file. Returns 1 if something was deleted,
   0 if no notice had that id. */
int deleteNotice(int id);

#endif
