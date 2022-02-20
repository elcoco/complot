#include "line.h"

uint32_t lineid_counter = 0;

Line* line_init(char* name)
{
    Line* l = malloc(sizeof(Line));
    l->name = strdup(name);
    l->next = NULL;
    l->lineid = lineid_counter++;
    return l;
}

void line_destroy(Line* l)
{
    free(l->name);
    free(l);
}

void  line_print_lines(Line* l)
{
    while (l != NULL) {
        printf("l: %s\n", l->name);
        l = l->next;
    }
}
