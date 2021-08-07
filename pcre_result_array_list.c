//
// Created by bczhc on 7/25/21.
//

#include "pcre_result_array_list.h"

void array_list_init_vp(struct ArrayList *list) {
    list->data = (struct Group *) malloc(15 * sizeof(struct Group));
    list->dataLen = 15;
    list->length = 0;
}

void array_list_free_vp(struct ArrayList *list) {
    free(list->data);
    list->length = 0, list->dataLen = 0;
}

size_t array_list_length_vp(struct ArrayList *list) {
    return list->length;
}

struct Group array_list_get_vp(struct ArrayList *list, size_t index) {
    return list->data[index];
}

void array_list_resize_vp(struct ArrayList *list, size_t newLen) {
    list->data = (struct Group *) realloc(list->data, newLen * sizeof(struct Group));
}

void array_list_add_vp(struct ArrayList *list, struct Group a) {
    size_t add_len = list->length + 1;
    if (add_len > list->dataLen) {
        size_t newLen = add_len * 2;
        array_list_resize_vp(list, newLen);
        list->dataLen = newLen;
    }
    list->data[list->length] = a;
    ++list->length;
}

void array_list_remove_vp(struct ArrayList *list) {
    --list->length;
}