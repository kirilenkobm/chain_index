/*
 * Bogdan Kirilenko
 * CSB Dresden, 2019
 * 
 * Shared library for chain_index python library
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


typedef struct{
    uint64_t chain_id;
    uint64_t start_byte;
    uint64_t offset;
    bool is_null;
    bool is_terminal;
} Point;

// empty
Point NOTHING = {};


// comparator for Points
int compare_structs
(const void *a, const void *b)
{
    Point *ia = (Point*)a;
    Point *ib = (Point*)b;
    if (ia->chain_id > ib->chain_id){
        return 1;
    } else {
        return -1;
    }
}   


void print_point
(Point *point)
{
    printf("Chain ID %llu;", point->chain_id);
    printf("Start byte %llu;", point->start_byte);
    printf("Offset %llu\n", point->offset);
}


void copy_point
(Point *from, Point *to)
{
    to->chain_id = from->chain_id;
    to->start_byte = from->start_byte;
    to->offset = from->offset;
    to->is_null = false;
    to->is_terminal = false;
}


void add_node
(Point *points, Point *bst, uint64_t bst_size, 
uint64_t low, uint64_t high, uint64_t i)
{
    if (low > high)
    {
        bst[i].is_null = true;
        bst[i].is_terminal = true;
        return;
    }

    uint64_t mid = low + (high - low) / 2;
    copy_point(&points[mid], &bst[i]);
    // print_point(&points[mid]);

    if (2 * i > bst_size)
    {
        bst[i].is_terminal = true;
        return;
    }
    // left
    add_node(points, bst, bst_size, low, mid - 1, 2 * i);
    // right
    add_node(points, bst, bst_size, mid + 1, high, (2 * i) + 1);
    return;
}

Point *make_bst
(Point *array, uint64_t arr_size, uint64_t *bst_size)
{
    *bst_size = 0;
    uint64_t i = 0;
    while (*bst_size < arr_size){*bst_size = pow(2, i); ++i;}
    *bst_size += 1;
    Point *bst = (Point*)malloc(sizeof(Point) * *bst_size);
    // printf("Arr size %llu; BST: %llu\n", arr_size, *bst_size);
    uint64_t init_i = 1;
    add_node(array, bst, *bst_size, 0, arr_size - 1, init_i);
    return bst;
}


// shared library entry point
int make_index
(uint64_t *chain_ids, uint64_t *start_bytes, uint64_t *offsets,
uint64_t arr_size, char *table_path)
{
    // struct of arrays to array of structs
    Point *array = (Point *)malloc(sizeof(Point) * arr_size);

    for (uint64_t i = 0; i < arr_size; ++i)
    {
        array[i].chain_id = chain_ids[i];
        array[i].start_byte = start_bytes[i];
        array[i].offset = offsets[i];
        array[i].is_null = false;
        array[i].is_terminal = false;
    }

    // we need chains to be sorted
    qsort(array, arr_size, sizeof(Point), compare_structs);

    // make bst
    uint64_t bst_size = 0;
    Point *bst = make_bst(array, arr_size, &bst_size);
    bst[0].chain_id = 0;
    bst[0].start_byte = 0;
    bst[0].offset = 0;
    bst[0].is_terminal = false;
    bst[0].is_null = false;

    // and now save
    FILE *fp = NULL;
    fp = fopen(table_path, "wb");
    fwrite(bst, sizeof(Point), bst_size, fp);
    fclose(fp);
    return 0;
}


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
            break;
        }
        ++n;
    }
        return 0;
}
