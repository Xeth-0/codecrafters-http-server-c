#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char **tokenize_string_to_array(char *some_string, char delim);
char *trim_whitespace(char *str);

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
    char **tokens = malloc((count + 1) * sizeof(char *));

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

int get_string_array_length(char *array[]){
    int count = 0;
    while (array[count] != NULL){
        count ++;
    }
    return count;
}