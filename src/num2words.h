#pragma once

#include "string.h"
#define BUFFER_SIZE 43

void fuzzy_time_to_words(int hours, int minutes, char* words, size_t length);

void fuzzy_hours_to_words(struct tm *t, char* words);
void fuzzy_sminutes_to_words(struct tm *t, char* words);
void fuzzy_minutes_to_words(struct tm *t, char* words);
void fuzzy_dates_to_words(struct tm *t, char* words);

