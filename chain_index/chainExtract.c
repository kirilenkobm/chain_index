#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHUNK 5

typedef struct{
    uint64_t chain_id;
    uint64_t start_byte;
    uint64_t offset;
    bool is_null;
    bool is_terminal;
} Point;


uint64_t get_s_byte
(char *index_path, uint64_t chain_id,
uint64_t *start_byte, uint64_t *offset)
{
    FILE *fp = NULL;
    fp = fopen(index_path, "rb");
    Point reader;
    uint64_t cur = 0;
    uint64_t n = 0;

    while (fread(&reader, sizeof(Point), 1, fp))
    {
        if (cur != n){
            ++n;
            continue;
        }
        if (chain_id > reader.chain_id){cur = (2 * n + 1);}
        else if (chain_id < reader.chain_id){cur = (2 * n);}
        else
        {
            *start_byte = reader.start_byte;
            *offset = reader.offset;
            fclose(fp);
            break;
        }
        ++n;
    }
        return 0;
}


int main
(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s [chain file] [index file] [chain_id]\n", argv[0]);
        return 0;
    }
    uint64_t start_byte = 0;
    uint64_t offset = 0;
    uint64_t chain_id = strtol(argv[3], NULL, 10);
    uint64_t _ = get_s_byte(argv[2], chain_id, &start_byte, &offset);

    FILE *fp = fopen(argv[1], "r");
    char *chain_body = (char*)malloc(sizeof(char) * offset + 1);

    fseek(fp, start_byte, SEEK_SET);
    size_t len = fread(chain_body, 1, offset, fp);
    *(chain_body + offset) = '\0';

    fclose(fp);
    printf("%s\n", chain_body);
}
