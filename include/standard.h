#pragma once

#include <stdlib.h>
#include <stdio.h>

static inline char* loadToRampChar(const char* path, size_t* size_out) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Couldn't open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, size, file);
    buffer[read_bytes] = '\0';

    *size_out = read_bytes;

    fclose(file);
    return buffer;
}
