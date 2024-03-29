#include "status.h"


StatusBar* status_init()
{
    StatusBar* sb = malloc(sizeof(StatusBar));
    sb->kv = malloc(sizeof(StatusItem*));
    sb->length = 0;
    sb->ysize = 1;
    sb->is_changed = false;
    sb->fgcol = CMAGENTA;
    sb->bgcol = CDEFAULT;
    return sb;
}

StatusItem* status_set(StatusBar* sb, char* k, char* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);

    // find key
    StatusItem** pkv = sb->kv;
    for (uint32_t i=0 ; i<sb->length ; i++, pkv++) {
        if (strcmp((*pkv)->key, k) == 0) {
            // TODO only if value is changed
            vsprintf((*pkv)->value, fmt, ptr);
            va_end(ptr);
            sb->is_changed = true;
            return *pkv;
        }
    }

    // item doesn't exist yet => extend, allocate and initialize!
    sb->kv = realloc(sb->kv, (sb->length+1) * sizeof(StatusItem*));
    StatusItem* si = malloc(sizeof(StatusItem));
    strcpy(si->key, k);
    vsprintf(si->value, fmt, ptr);
    sb->kv[sb->length] = si;
    sb->length++;
    va_end(ptr);
    sb->is_changed = true;
    return si;
}

void status_draw(StatusBar* sb)
{
    if (!sb->is_changed)
        return;

    uint32_t xpos = 0;

    StatusItem** pkv = sb->kv;
    for (uint32_t i=0 ; i<sb->length ; i++, pkv++) {
        char buf[1000] = {'\0'};
        sprintf(buf, "%s:%s", (*pkv)->key, (*pkv)->value);

        if (i != sb->length-1)
            strcat(buf, ", ");

        for (uint32_t ix=0 ; ix<strlen(buf) ; ix++, xpos++) {
            if (xpos >= sb->xsize) {
                add_str(sb->win, 0, sb->xsize-3, sb->fgcol, sb->bgcol, "...");
                break;
            }
            //mvwaddch(sb->win, 0, xpos, buf[ix]);
            add_chr(sb->win, 0, xpos, sb->fgcol, sb->bgcol, buf[ix]);
        }
    }
}

void status_destroy(StatusBar* sb)
{
    for (uint32_t ix=0 ; ix<sb->length ; ix++) {
        free(sb->kv[ix]);
    }
    free(sb->kv);
    free(sb);
}
