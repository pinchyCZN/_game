#pragma once
#include "entity.h"
void exit_cursor();
void center_cursor();
void do_exit();
void abort_msg(const char *msg);
int is_key_down(int key);
int are_keys_down(int *list,int count);
int are_keys_up(int *list,int count);

int get_entity(int id,ENTITY **e);