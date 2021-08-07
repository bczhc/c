//
// Created by bczhc on 7/25/21.
//

#ifndef C_PCRE_RESULT_ARRAY_LIST_H
#define C_PCRE_RESULT_ARRAY_LIST_H

#include "stdlib.h"
#include "pcre_result.h"

struct ArrayList {
    struct Group *data;
    size_t dataLen;
    size_t length;
};

void array_list_init_vp(struct ArrayList *list);

void array_list_free_vp(struct ArrayList *list);

size_t array_list_length_vp(struct ArrayList *list);

struct Group array_list_get_vp(struct ArrayList *list, size_t index);

void array_list_resize_vp(struct ArrayList *list, size_t newLen);

void array_list_add_vp(struct ArrayList *list, struct Group a);

void array_list_remove_vp(struct ArrayList *list);

#endif //C_PCRE_RESULT_ARRAY_LIST_H
