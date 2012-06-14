
# commander.c

  Commander option parser ported to C.

## Example

```c
#include <stdio.h>
#include "commander.h"

static void
verbose(command_t *self) {
  printf("verbose: enabled\n");
}

static void
required(command_t *self) {
  printf("required: %s\n", self->arg);
}

static void
optional(command_t *self) {
  printf("optional: %s\n", self->arg);
}

int
main(int argc, const char **argv){
  command_t cmd;
  command_init(&cmd, argv[0], "0.0.1");
  command_option(&cmd, "-v", "--verbose", "enable verbose stuff", verbose);
  command_option(&cmd, "-r", "--required <arg>", "required arg", required);
  command_option(&cmd, "-o", "--optional [arg]", "optional arg", optional);
  command_parse(&cmd, argc, argv);
  printf("additional args:\n");
  for (int i = 0; i < cmd.argc; ++i) {
    printf("  - '%s'\n", cmd.argv[i]);
  }
  return 0;
}
```

## Automated --help

  The previous example would produce the following `--help`:

```

Usage: example [options]

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -v, --verbose                 enable verbose stuff
  -r, --required <arg>          required arg
  -o, --optional [arg]          optional arg

```

## Closure

  `cmd.data` is a `void *` so pass along a struct to the callbacks if you want.

## Todo

  - unrecognized flags (suggestions?)
  - specify "Usage: " string
