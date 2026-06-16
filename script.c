#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define BUFFER_SIZE (1 << 16)

int main(const int argc, const char *const argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Incorrect usage! Usage: \"script <input file> <output file>\"\n");
        return 1;
    }

    const char *const from_file_name = argv[1], *const to_file_name = argv[2];
    if ((strlen(from_file_name) == strlen(to_file_name)) && !strcmp(from_file_name, to_file_name)) {
        fprintf(stderr, "Can't read from and write to the one single file at once!\n");
        return 1;
    }

    int return_code = 0;
    FILE *from_file = NULL, *to_file = NULL;
    char *buffer = NULL;

    from_file = fopen(from_file_name, "r");
    if (!from_file) {
        fprintf(stderr, "Error opening file \"%s\": %s\n", from_file_name, strerror(errno));
        return_code = 1;
        goto cleanup;
    }

    to_file = fopen(to_file_name, "w");
    if (!to_file) {
        fprintf(stderr, "Error opening file \"%s\": %s\n", to_file_name, strerror(errno));
        return_code = 1;
        goto cleanup;
    }

    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error allocating %zu bytes of memory: %s\n", (size_t) BUFFER_SIZE, strerror(errno));
        return_code = 1;
        goto cleanup;
    }

    size_t bytes_read = 0;
    bool prev_was_newline = false;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, from_file)) > 0) {
        size_t l = 0;

        for (size_t i = 0; i < bytes_read; ) {
            if (buffer[i] == '\n') {
                size_t j = i;
                if (!prev_was_newline) ++j;
                while (i < bytes_read && (buffer[i] == '\n')) ++i;

                fwrite(buffer + l, 1, j - l, to_file);
                l = i;
                prev_was_newline = false;
            } else ++i;
        }

        if (buffer[bytes_read - 1] == '\n') prev_was_newline = true;
        if (l < bytes_read) fwrite(buffer + l, 1, bytes_read - l, to_file);
    }

    if (ferror(from_file)) {
        fprintf(stderr, "Error reading from file \"%s\": %s\n", from_file_name, strerror(errno));
        return_code = 1;
    }

    cleanup:
    if (from_file) fclose(from_file);
    if (to_file) fclose(to_file);
    if (buffer) free(buffer);

    return return_code;
}
