#include "legend.h"

Legend* legend_init(Yaxis* ya)
{
    Legend* le = malloc(sizeof(Legend));
    le->ysize = 1;
    le->yaxis = ya;
    return le;
}

void legend_draw(Legend* le)
{
    /* draw left or right legend */
    // TODO is writing outside of window area but is dificult to fix because UTF8 chars are wider than ascii
    Line* l = le->yaxis->line;
    uint32_t xpos = 0;

    while (l != NULL) {
        char buf[LEGEND_MAX_SIZE] = {'\0'};
        sprintf(buf, "%s = %s", l->icon, l->name);

        if (l->next != NULL)
            strcat(buf, ", ");

        add_str(le->win, 0, xpos, l->color, buf);

        // can't do strlen on buf because the icon is a multi byte unicode character
        xpos += strlen(l->name) + 6;
        l = l->next;
    }
}

void legend_destroy(Legend* le)
{
    free(le);
}

