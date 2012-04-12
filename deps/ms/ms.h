
//
// ms.h
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#ifndef MS
#define MS

// max buffer length

#ifndef MS_MAX
#define MS_MAX 256
#endif

// aliases

#define strtous string_to_microseconds
#define strtoms string_to_milliseconds
#define mstostr milliseconds_to_string
#define mstolstr milliseconds_to_long_string

// prototypes

long long
string_to_microseconds(const char *str);

long long
string_to_milliseconds(const char *str);

char *
milliseconds_to_string(long long ms);

#endif