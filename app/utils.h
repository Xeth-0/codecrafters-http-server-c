#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <zlib.h>
#include <zconf.h>

char **tokenize_string_to_array(char *some_string, char delim);
char *trim_whitespace(char *str);
char *compress_string(char *source_string, size_t *output_length);

char **tokenize_string_to_array(char *some_string, char delim)
{
    // Count how many there are to assign space for the array.
    int count = 0;
    for (int i = 0; some_string[i] != '\0'; i++)
    {
        if (some_string[i] == delim)
            count++;
    }

    char *token_rest;
    char *token = strtok_r(some_string, ",", &token_rest);
    char **tokens = malloc((count + 2) * sizeof(char *));

    int idx = 0;
    while (token != NULL)
    {
        tokens[idx] = token;
        token = strtok_r(NULL, ",", &token_rest);
        idx++;
    }

    // Trim whitespace from tokens
    // Leading whitespace
    for (int i = 0; i < count + 1; i++)
    {
        char *token = tokens[i];
        char *trimmed_token = trim_whitespace(token);
        // *(trimmed_token + strlen(trimmed_token)) = '\0';
        tokens[i] = strdup(trimmed_token);
    }
    // Null terminator for the array
    tokens[count + 1] = NULL;
    return tokens;
}

char *trim_whitespace(char *str)
{
    char *end;

    // Leading whitespace
    while (isspace((unsigned char)*str))
        str++;

    // If we're already at the null terminator, the string is all whitespace.
    if (*str == '0')
        return str;

    // Trailing whitespace
    end = str + strlen(str) + 1;

    while (end > str && isspace((unsigned char)*end))
        end--;

    // Add the null terminator
    *end = '\0';

    return str;
}

int get_string_array_length(char *array[])
{
    int count = 0;
    while (array[count] != NULL)
    {
        count++;
    }
    return count;
}

char *compress_string(char *source_string, size_t *output_length)
{
    int err;

    printf("source string: %s\n", source_string);

    z_stream zs = {0};
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 0x1F, 8, Z_DEFAULT_STRATEGY);

    uLong source_len = (uLong)strlen(source_string);

    size_t dest_max_len = deflateBound(&zs, source_len);
    char *dest_string = malloc(dest_max_len);

    memset(dest_string, 0, dest_max_len);

    printf("source len: %lu\nmax_len: %lu\n", source_len, dest_max_len);

    printf("Compressing....\n");

    zs.next_in = (Bytef *)source_string;;
    zs.avail_in = source_len;
    zs.next_out = (Bytef *)dest_string;
    zs.avail_out = dest_max_len;

    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    *output_length = zs.total_out;

    printf("dest string: %s\noutput len: %ln\n", dest_string, output_length);
    return dest_string;
}