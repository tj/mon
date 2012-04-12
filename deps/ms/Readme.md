
# ms.c

```c
#define strtous string_to_microseconds
#define strtoms string_to_milliseconds
#define mstostr milliseconds_to_string
#define mstolstr milliseconds_to_long_string

long long
string_to_microseconds(const char *str);

long long
string_to_milliseconds(const char *str);

char *
milliseconds_to_string(long long ms);
```

For usage view the [tests](https://github.com/visionmedia/ms.c/blob/master/ms.c#L52)