#include "csv.h"

#define MAX_LINES 5000

Line* init_line() {
    Line* l = malloc(sizeof(Line));
    return l;
}

Line** read_csv(char* path)
{
    Line** lines = malloc(MAX_LINES*sizeof(Line*));
    FILE* fp = fopen(path, "r");
    uint32_t i = 0;

    if (fp == NULL) {
        printf("File not found!\n");
        return NULL;
    }

    char buf[BUF_SIZE];

    while (fgets(buf, BUF_SIZE, fp)) {
        if (i == MAX_LINES)
            goto cleanup;

        // remove newline
        buf[strlen(buf)-1] = '\0';

        char* tok;
        tok = strtok(buf, ",");

        lines[i] = init_line();

        //printf("------------------------\n");
        if (tok == NULL)
            goto cleanup;

        if ((tok = strtok(NULL, ",")) == NULL)
            goto cleanup;
        strcpy(lines[i]->f0, tok);

        if ((tok = strtok(NULL, ",")) == NULL)
            goto cleanup;
        strcpy(lines[i]->f1, tok);

        if ((tok = strtok(NULL, ",")) == NULL)
            goto cleanup;
        strcpy(lines[i]->f2, tok);

        i++;
    }

    cleanup:

    printf("cleanup!");
        fclose(fp);

    return lines;

}
