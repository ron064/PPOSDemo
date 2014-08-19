#include "pebble.h"
#include "num2words.h"

static const char const ONES[][6] = {
  "",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine"
};

static const char const TEENS[][10] ={
  "",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seven",
  "eighteen",
  "nineteen"
};

static const char const FUZZYTEENS[][10] ={
  "",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen"
};

static const char const TENS[][7] = {
  "",
  "ten",
  "twenty",
  "thirty",
  "forty",
  "fifty"
  //"sixty",
  //"seventy",
  //"eighty",
  //"ninety"
};

static const char* STR_OH_CLOCK = "o'clock";
static const char* STR_NOON = "noon";
static const char* STR_MIDNIGHT = "midnight";
static const char* STR_QUARTER = "quarter";
static const char* STR_TO = "to";
static const char* STR_PAST = "past";
static const char* STR_HALF = "half";
static const char* STR_AFTER = "after";

static const char* STR_MID = "mid";
static const char* STR_NIGHT = "night";
static const char* STR_OH = "oh";
static const char* STR_TEEN = "teen";

static size_t append_fuzzy_number(char* words, int num) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1 && num != 10) {
      strcat(words, FUZZYTEENS[ones_val]);
      return strlen(FUZZYTEENS[ones_val]);
    }
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
    if (ones_val > 0) {
      strcat(words, " ");
      len += 1;
    }
  }

  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
  }
  return len;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}

void fuzzy_time_to_words(int hours, int minutes, char* words, size_t length) {
  int fuzzy_hours = hours;
  int fuzzy_minutes = ((minutes + 2) / 5) * 5;

  // Handle hour & minute roll-over.
  if (fuzzy_minutes > 55) {
    fuzzy_minutes = 0;
    fuzzy_hours += 1;
    if (fuzzy_hours > 23) {
      fuzzy_hours = 0;
    }
  }

  size_t remaining = length;
  memset(words, 0, length);

  if (fuzzy_minutes != 0 && (fuzzy_minutes >= 10 || fuzzy_minutes == 5 || fuzzy_hours == 0 || fuzzy_hours == 12)) {
    if (fuzzy_minutes == 15) {
      remaining -= append_string(words, remaining, STR_QUARTER);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_AFTER);
      remaining -= append_string(words, remaining, " ");
    } else if (fuzzy_minutes == 45) {
      remaining -= append_string(words, remaining, STR_QUARTER);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_TO);
      remaining -= append_string(words, remaining, " ");

      fuzzy_hours = (fuzzy_hours + 1) % 24;
    } else if (fuzzy_minutes == 30) {
      remaining -= append_string(words, remaining, STR_HALF);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_PAST);
      remaining -= append_string(words, remaining, " ");
    } else if (fuzzy_minutes < 30) {
      remaining -= append_fuzzy_number(words, fuzzy_minutes);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_AFTER);
      remaining -= append_string(words, remaining, " ");
    } else {
      remaining -= append_fuzzy_number(words, 60 - fuzzy_minutes);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_TO);
      remaining -= append_string(words, remaining, " ");

      fuzzy_hours = (fuzzy_hours + 1) % 24;
    }
  }

  if (fuzzy_hours == 0) {
    remaining -= append_string(words, remaining, STR_MIDNIGHT);
  } else if (fuzzy_hours == 12) {
    remaining -= append_string(words, remaining, STR_NOON);
  } else {
    remaining -= append_fuzzy_number(words, fuzzy_hours % 12);
  }

  if (fuzzy_minutes == 0 && !(fuzzy_hours == 0 || fuzzy_hours == 12)) {
    remaining -= append_string(words, remaining, " ");
    remaining -= append_string(words, remaining, STR_OH_CLOCK);
  }
}

static size_t append_number(char* words, int num) {
  int tens_val = num / 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val == 1 && num != 10) {
    strcat(words, TEENS[ones_val]);
    return strlen(TEENS[ones_val]);
  }
  strcat(words, TENS[tens_val]);
  len += strlen(TENS[tens_val]);
  if (tens_val < 1) {
    strcat(words, ONES[ones_val]);
    return strlen(ONES[ones_val]);
  }
return len;
}

static size_t append_minutes_number(char* words, int num) {
  int ones_val = num % 10;

  size_t len = 0;
  strcat(words, ONES[ones_val]);
  len += strlen(ONES[ones_val]);
  return len;
}

void fuzzy_minutes_to_words(struct tm *t, char* words) {
  int fuzzy_hours = t->tm_hour;
  int fuzzy_minutes = t->tm_min;

  size_t remaining = BUFFER_SIZE;
  memset(words, 0, BUFFER_SIZE);

  //Is it midnight? or noon
  if (fuzzy_minutes != 0 || (fuzzy_hours != 12 && fuzzy_hours != 0)) {
    //is it the top of the hour?
    if(fuzzy_minutes == 0){
      remaining -= append_string(words, remaining, STR_OH_CLOCK);
    } else if(fuzzy_minutes < 10){
      //is it before ten minutes into the hour
      remaining -= append_string(words, remaining, STR_OH);
    } else {
      remaining -= append_number(words, fuzzy_minutes);
    }
  } else if (fuzzy_hours == 0) {
    remaining -= append_string(words, remaining, STR_NIGHT);
  }
}

void fuzzy_sminutes_to_words(struct tm *t, char* words) {
  int fuzzy_hours = t->tm_hour;
  int fuzzy_minutes = t->tm_min;

  size_t remaining = BUFFER_SIZE;
  memset(words, 0, BUFFER_SIZE);

  if (10 < fuzzy_minutes && fuzzy_minutes < 20) {
    if (fuzzy_minutes == 17) {
        strcat(words, STR_TEEN);
      }
  } else if (fuzzy_minutes != 0 || (fuzzy_hours != 12 && fuzzy_hours != 0)) {
      remaining -= append_minutes_number(words, fuzzy_minutes);
  }
}

void fuzzy_hours_to_words(struct tm *t, char* words) {
  int fuzzy_hours = t->tm_hour;
  int fuzzy_minutes = t->tm_min;

  size_t remaining = BUFFER_SIZE;
  memset(words, 0, BUFFER_SIZE);

  //Is it midnight?
  if (fuzzy_hours == 0 && fuzzy_minutes == 0) {
    remaining -= append_string(words, remaining, STR_MID);
  //is it noon?
  } else if (fuzzy_hours == 12 && fuzzy_minutes == 0) {
    remaining -= append_string(words, remaining, STR_NOON);
  } else if (fuzzy_hours == 0 || fuzzy_hours == 12 || fuzzy_hours == 24){
    remaining -= append_number(words, 12);
  } else {
    //get hour
    remaining -= append_number(words, fuzzy_hours % 12);
  }
}

void fuzzy_dates_to_words(struct tm *t, char* words) {
	strftime(words, BUFFER_SIZE, "%a %e", t);
}
