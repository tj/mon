
//
// ms.c
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ms.h"

// milliseconds

#define MS_SEC 1000
#define MS_MIN 60000
#define MS_HOUR 3600000
#define MS_DAY 86400000
#define MS_WEEK 604800000
#define MS_YEAR 31557600000

/*
 * Convert the given `str` representation to microseconds,
 * for example "10ms", "5s", "2m", "1h" etc.
 */

long long
string_to_microseconds(const char *str) {
  size_t len = strlen(str);
  long long val = strtoll(str, NULL, 10);
  if (!val) return -1;
  switch (str[len - 1]) {
    case 's': return  'm' == str[len - 2] ? val * 1000 : val * 1000000;
    case 'm': return val * 60000000;
    case 'h': return val * 3600000000;
    case 'd': return val * 86400000000;
    default:  return val;
  }
}

/*
 * Convert the given `str` representation to milliseconds,
 * for example "10ms", "5s", "2m", "1h" etc.
 */

long long
string_to_milliseconds(const char *str) {
  size_t len = strlen(str);
  long long val = strtoll(str, NULL, 10);
  if (!val) return -1;
  switch (str[len - 1]) {
    case 's': return  'm' == str[len - 2] ? val : val * 1000;
    case 'm': return val * 60 * 1000;
    case 'h': return val * 60 * 60 * 1000;
    case 'd': return val * 24 * 60 * 60 * 1000;
    default:  return val;
  }
}

/*
 * Convert the given `ms` to a string. This
 * value must be `free()`d by the developer.
 */

char *
milliseconds_to_string(long long ms) {
  char *str = malloc(MS_MAX);
  if (!str) return NULL;
  long div = 1;
  char *fmt;

  if (ms < MS_SEC) fmt = "%lldms";
  else if (ms < MS_MIN) { fmt = "%llds"; div = MS_SEC; }
  else if (ms < MS_HOUR) { fmt = "%lldm"; div = MS_MIN; }
  else if (ms < MS_DAY) { fmt = "%lldh"; div = MS_HOUR; }
  else if (ms < MS_WEEK) { fmt = "%lldd"; div = MS_DAY; }
  else if (ms < MS_YEAR) { fmt = "%lldw"; div = MS_WEEK; }
  else { fmt = "%lldy"; div = MS_YEAR; }
  snprintf(str, MS_MAX, fmt, ms / div);

  return str;
}

/*
 * Convert the given `ms` to a long string. This
 * value must be `free()`d by the developer.
 */

char *
milliseconds_to_long_string(long long ms) {
  long div;
  char *name;

  char *str = malloc(MS_MAX);
  if (!str) return NULL;

  if (ms < MS_SEC) {
    sprintf(str, "less than one second");
    return str;
  }

  if (ms < MS_MIN) { name = "second"; div = MS_SEC; }
  else if (ms < MS_HOUR) { name = "minute"; div = MS_MIN; }
  else if (ms < MS_DAY) { name = "hour"; div = MS_HOUR; }
  else if (ms < MS_WEEK) { name = "day"; div = MS_DAY; }
  else if (ms < MS_YEAR) { name = "week"; div = MS_WEEK; }
  else { name = "year"; div = MS_YEAR; }

  long long val = ms / div;
  char *fmt = 1 == val
    ? "%lld %s"
    : "%lld %ss";

  snprintf(str, MS_MAX, fmt, val, name);
  return str;
}

// tests

#ifdef TEST_MS

#include <assert.h>

void
equal(char *a, char *b) {
  if (strcmp(a, b)) {
    printf("expected: %s\n", a);
    printf("actual: %s\n", b);
    exit(1);
  }
}

void
test_string_to_microseconds() {
  assert(string_to_microseconds("") == -1);
  assert(string_to_microseconds("s") == -1);
  assert(string_to_microseconds("hey") == -1);
  assert(string_to_microseconds("5000") == 5000);
  assert(string_to_microseconds("1ms") == 1000);
  assert(string_to_microseconds("5ms") == 5000);
  assert(string_to_microseconds("1s") == 1000000);
  assert(string_to_microseconds("5s") == 5000000);
  assert(string_to_microseconds("1m") == 60000000);
  assert(string_to_microseconds("1h") == 3600000000);
  assert(string_to_microseconds("2d") == 2 * 24 * 3600000000);
  assert(strtous("1ms") == 1000);
}

void
test_string_to_milliseconds() {
  assert(string_to_milliseconds("") == -1);
  assert(string_to_milliseconds("s") == -1);
  assert(string_to_milliseconds("hey") == -1);
  assert(string_to_milliseconds("5000") == 5000);
  assert(string_to_milliseconds("1ms") == 1);
  assert(string_to_milliseconds("5ms") == 5);
  assert(string_to_milliseconds("1s") == 1000);
  assert(string_to_milliseconds("5s") == 5000);
  assert(string_to_milliseconds("1m") == 60 * 1000);
  assert(string_to_milliseconds("1h") == 60 * 60 * 1000);
  assert(string_to_milliseconds("1d") == 24 * 60 * 60 * 1000);
  assert(strtoms("50s") == 50000);
}

void
test_milliseconds_to_string() {
  equal("500ms", mstostr(500));
  equal("500ms", milliseconds_to_string(500));
  equal("5s", milliseconds_to_string(5000));
  equal("2s", milliseconds_to_string(2500));
  equal("1m", milliseconds_to_string(MS_MIN));
  equal("5m", milliseconds_to_string(5 * MS_MIN));
  equal("1h", milliseconds_to_string(MS_HOUR));
  equal("2d", milliseconds_to_string(2 * MS_DAY));
  equal("2w", milliseconds_to_string(15 * MS_DAY));
  equal("3y", milliseconds_to_string(3 * MS_YEAR));
}

void
test_milliseconds_to_long_string() {
  equal("less than one second", mstolstr(500));
  equal("less than one second", milliseconds_to_long_string(500));
  equal("5 seconds", milliseconds_to_long_string(5000));
  equal("2 seconds", milliseconds_to_long_string(2500));
  equal("1 minute", milliseconds_to_long_string(MS_MIN));
  equal("5 minutes", milliseconds_to_long_string(5 * MS_MIN));
  equal("1 hour", milliseconds_to_long_string(MS_HOUR));
  equal("2 days", milliseconds_to_long_string(2 * MS_DAY));
  equal("2 weeks", milliseconds_to_long_string(15 * MS_DAY));
  equal("1 year", milliseconds_to_long_string(MS_YEAR));
  equal("3 years", milliseconds_to_long_string(3 * MS_YEAR));
}

int
main(){
  test_string_to_microseconds();
  test_string_to_milliseconds();
  test_milliseconds_to_string();
  test_milliseconds_to_long_string();
  printf("\n  \e[32m\u2713 \e[90mok\e[0m\n\n");
  return 0;
}

#endif