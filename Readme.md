
# mon(1)

  Super-simple monitoring program.

  Effectively all it does is daemonize some programs,
  re-executing the program on failure, write pidfiles,
  and provide status checks.

## Installation

```
$ make install
```

## Usage

```

Usage: mon [options] <cmd>

Options:

  -s, --sleep <sec>    sleep seconds before re-executing [1]
  -S, --status         check status of --pidfile
  -l, --log <file>     specify logfile [mon.log]
  -d, --daemonize      daemonize the program
  -p, --pidfile <path> write pid to <path>
  -P, --prefix <str>   add a log prefix <str>
  -v, --version        output program version
  -h, --help           output help information

```

## Example

  `mon(1)` is designed to monitor a single program only, this means a few things,
  firstly that a single `mon(1)` may crash and it will not influence other programs,
  secondly that the "configuration" for `mon(1)` is simply a shell script,
  no need for funky weird inflexible DSLs.

```bash
#!/usr/bin/env bash

pids="/var/run"
app="/www/example.com"

mon -d redis-server -p $pids/redis.pid
mon -d "node $app/app" -p $pids/app-0.pid
mon -d "node $app/jobs" -p $pids/jobs-0.pid
mon -d "node $app/jobs" -p $pids/jobs-1.pid
mon -d "node $app/jobs" -p $pids/jobs-2.pid
mon -d "node $app/image" -p $pids/image-0.pid
mon -d "node $app/image" -p $pids/image-1.pid
mon -d "node $app/image-broker" -p $pids/image-broker.pid
```

## Restarting processes

  Restarting a process is simple, `kill(1)` it. For example if your app is written
  to gracefully exit on __SIGQUIT__ you might want to do:
  
    $ kill -s SIGQUIT $(cat /var/run/app-0.pid)

  Or if you just want things done quick and dirty:

    $ kill -s SIGKILL $(cat /var/run/app-0.pid)
