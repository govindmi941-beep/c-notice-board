#include <stdio.h>
#include <string.h>
#include <time.h>
#include "notice.h"

static void todayDate(char *out, size_t outSize) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(out, outSize, "%Y-%m-%d", t);
}

void ensureDataFile(void) {
    FILE *check = fopen(DATA_FILE, "rb");
    if (check != NULL) {
        fclose(check);
        return; /* file already exists, don't overwrite it */
    }

    /* First run: seed with 3 sample notices so the board isn't empty. */
    addNotice(
        "Internal Assessment 1 Schedule Released",
        "IA-1 for all AIML branches will be conducted from 22nd to 27th September. Timetable has been shared on the department notice board and email.",
        "Exam", "Urgent", "Examination Cell");
    addNotice(
        "Cloud Computing Lab Submission Deadline",
        "All students must submit their Cloud Computing lab file along with the deployed project link by end of this week.",
        "Academic", "Normal", "Dept. of AIML");
    addNotice(
        "Merit Scholarship Form Submission",
        "Students eligible for the merit-cum-means scholarship must submit the completed form to the accounts office before the 30th.",
        "Scholarship", "Normal", "Accounts Office");
}

int loadNotices(Notice list[], int maxCount) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) return 0; /* no file yet = no notices */

    int count = 0;
    while (count < maxCount && fread(&list[count], sizeof(Notice), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

static int nextId(void) {
    Notice list[MAX_NOTICES];
    int count = loadNotices(list, MAX_NOTICES);
    int maxId = 0;
    for (int i = 0; i < count; i++) {
        if (list[i].id > maxId) maxId = list[i].id;
    }
    return maxId + 1;
}

int addNotice(const char *title, const char *content, const char *category,
              const char *priority, const char *posted_by) {
    Notice n;
    n.id = nextId();
    strncpy(n.title, title, MAX_TITLE - 1);         n.title[MAX_TITLE - 1] = '\0';
    strncpy(n.content, content, MAX_CONTENT - 1);   n.content[MAX_CONTENT - 1] = '\0';
    strncpy(n.category, category, MAX_CATEGORY - 1); n.category[MAX_CATEGORY - 1] = '\0';
    strncpy(n.priority, priority, MAX_PRIORITY - 1); n.priority[MAX_PRIORITY - 1] = '\0';
    strncpy(n.posted_by, posted_by, MAX_POSTEDBY - 1); n.posted_by[MAX_POSTEDBY - 1] = '\0';
    todayDate(n.date, MAX_DATE);

    FILE *fp = fopen(DATA_FILE, "ab"); /* "ab" = append, binary mode */
    if (fp == NULL) return 0;
    fwrite(&n, sizeof(Notice), 1, fp);
    fclose(fp);
    return n.id;
}

int deleteNotice(int id) {
    Notice list[MAX_NOTICES];
    int count = loadNotices(list, MAX_NOTICES);

    FILE *out = fopen("notices.tmp", "wb");
    if (out == NULL) return 0;

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (list[i].id == id) {
            found = 1;       /* skip writing this one -> effectively deletes it */
            continue;
        }
        fwrite(&list[i], sizeof(Notice), 1, out);
    }
    fclose(out);

    if (found) {
        remove(DATA_FILE);         /* delete the old file          */
        rename("notices.tmp", DATA_FILE); /* temp file becomes the real one */
    } else {
        remove("notices.tmp");     /* nothing matched, discard temp file */
    }
    return found;
}
