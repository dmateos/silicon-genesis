#include <stdio.h>
#include <stdlib.h>

/**
 * Get the size of a file.
 * @param file Pre fopen'd POSIX file type.
 * @return file size on success, 0 on fail.
 */
static long fsize(FILE *file) {
    long size, saved;

    if (file == NULL)
        return 0;

    /* Do some ugly seek foo to find out how many
       * how many bytes the file is. */
    saved = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, saved, SEEK_SET);

    return size;
}

char *read_config(const char *path) {
    FILE *cfile;
    char *buff;
    int bsize;

    /* Do some init stuff. */
    if (!(cfile = fopen(path, "r")))
        return NULL;
    if (!(bsize = fsize(cfile)))
        return NULL;
    if (!(buff = malloc(bsize)))
        return NULL;

    return buff;
}

