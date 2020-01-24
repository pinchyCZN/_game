#pragma once
#include <Windows.h>
#include <stdio.h>

HANDLE get_console_handle();
void open_console();
int do_wait();
int get_key();
int getkey_wait();
int get_flen(FILE *f);
int key_state(int key);