
//
// mon.c
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "ms.h"

/*
 * Program version.
 */

#define VERSION "0.0.1"

/*
 * Log prefix.
 */

static char *prefix = NULL;

/*
 * Child process PID.
 */

static pid_t pid;

/*
 * Logger.
 */

#define log(fmt, args...) \
  if (prefix) { \
    printf("mon : %s : " fmt "\n", prefix, ##args); \
  } else { \
    printf("mon : " fmt "\n", ##args); \
  }

/*
 * Output usage information.
 */

void
usage() {
  printf(
    "\n"
    "  Usage: mon [options] <cmd>\n"
    "\n"
    "  Options:\n"
    "\n"
    "    -s, --sleep <sec>    sleep seconds before re-executing [1]\n"
    "    -S, --status         check status of --pidfile\n"
    "    -l, --log <file>     specify logfile [mon.log]\n"
    "    -d, --daemonize      daemonize the program\n"
    "    -p, --pidfile <path> write pid to <path>\n"
    "    -P, --prefix <str>   add a log prefix <str>\n"
    "    -v, --version        output program version\n"
    "    -h, --help           output help information\n"
    "\n"
  );
}

/*
 * Output error `msg`.
 */

void
error(char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

/*
 * Check if process of `pid` is alive.
 */

int
alive(pid_t pid) {
  return 0 == kill(pid, 0);
}

/*
 * Kill everythanggg.
 */

void
graceful_exit(int sig) {
  log("shutting down");
  kill(pid, SIGTERM);
  exit(1);
}

/*
 * Write `pid` to `file`.
 */

void
write_pidfile(char *file, pid_t pid) {
  char buf[32] = {0};
  snprintf(buf, 32, "%d", pid);
  int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd < 0) perror("open()");
  write(fd, buf, 32);
  close(fd);
}

/*
 * Output status of `pidfile`.
 */

void
show_status_of(char *pidfile) {
  off_t size;
  struct stat s;

  // stat
  if (stat(pidfile, &s) < 0) {
    perror("stat()");
    exit(1);
  }

  size = s.st_size;

  // opens
  int fd = open(pidfile, O_RDONLY, 0);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }

  // read
  char buf[size];
  if (size != read(fd, buf, size)) {
    perror("read()");
    exit(1);
  }

  // updtime
  time_t modified = s.st_mtime;

  struct timeval t;
  gettimeofday(&t, NULL);
  time_t now = t.tv_sec;
  time_t secs = now - modified;

  // status
  pid_t pid = atoi(buf);

  if (alive(pid)) {
    char *str = milliseconds_to_long_string(secs * 1000);
    printf("\e[90m%d\e[0m : \e[32malive\e[0m : uptime %s\e[m\n", pid, str);
    free(str);
  } else {
    printf("\e[90m%d\e[0m : \e[31mdead\e[0m\n", pid);
  }

  close(fd);
}

/*
 * Redirect stdio to `file`.
 */

void
redirect_stdio_to(char *file) {
  int logfd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0755);
  int nullfd = open("/dev/null", O_RDONLY, 0);

  if (-1 == logfd) {
    perror("open()");
    exit(1);
  }

  if (-1 == nullfd) {
    perror("open()");
    exit(1);
  }

  dup2(nullfd, 0);
  dup2(logfd, 1);
  dup2(logfd, 2);
}

/*
 * Daemonize the program.
 */

void
daemonize() {
  if (fork()) exit(0);

  if (setsid() < 0) {
    perror("setsid()");
    exit(1);
  }
}

/*
 * Monitor the given `cmd`, `sleepsec` specifying
 * the number of seconds before restart.
 */

void
monitor(char *cmd, int sleepsec, char *pidfile) {
exec: {
  pid = fork();
  int status;

  switch (pid) {
    case -1:
      perror("fork()");
      exit(1);
    case 0:
      log("bash -c \"%s\"", cmd);
      execl("/bin/bash", "bash", "-c", cmd, 0);
      perror("execl()");
      exit(1);
    default:
      log("pid %d", pid);

      // write pidfile
      if (pidfile) {
        log("write pid to %s", pidfile);
        write_pidfile(pidfile, pid);
      }

      // wait for exit
      waitpid(pid, &status, 0);

      // signalled
      if (WIFSIGNALED(status)) {
        log("signal(%s)", strsignal(WTERMSIG(status)));
        log("sleep(%d)", sleepsec);
        sleep(sleepsec);
        goto exec;
      }

      // check status
      if (WEXITSTATUS(status)) {
        log("exit(%d)", WEXITSTATUS(status));
        log("sleep(%d)", sleepsec);
        sleep(sleepsec);
        goto exec;
      }
  }
}
}

/*
 * [options] <cmd>
 */

int
main(int argc, char **argv){
  char *cmd = NULL;
  char *pidfile = NULL;
  char *logfile = "mon.log";
  int daemon = 0;
  int sleepsec = 1;

  // parse args
  for (int i = 1; i < argc; ++i) {
    char *arg = argv[i];

    // -l, --log <file>
    if (!strcmp("-l", arg) || !strcmp("--log", arg)) {
      logfile = argv[++i];
      continue;
    }

    // -d, --daemonize
    if (!strcmp("-d", arg) || !strcmp("--daemonize", arg)) {
      daemon = 1;
      continue;
    }

    // -s, --sleep <sec>
    if (!strcmp("-s", arg) || !strcmp("--sleep", arg)) {
      sleepsec = atoi(argv[++i]);
      continue;
    }

    // -p, --pidfile <path>
    if (!strcmp("-p", arg) || !strcmp("--pidfile", arg)) {
      pidfile = argv[++i];
      continue;
    }

    // -S, --status
    if (!strcmp("-S", arg) || !strcmp("--status", arg)) {
      if (!pidfile) error("--pidfile required");
      show_status_of(pidfile);
      exit(0);
    }

    // -P, --prefix <str>
    if (!strcmp("-P", arg) || !strcmp("--prefix", arg)) {
      prefix = argv[++i];
      continue;
    }

    // -v, --version
    if (!strcmp("-v", arg) || !strcmp("--version", arg)) {
      printf("%s\n", VERSION);
      exit(0);
    }

    // -h, --help
    if (!strcmp("-h", arg) || !strcmp("--help", arg)) {
      usage();
      exit(0);
    }

    cmd = arg;
  }

  // command required
  if (!cmd) error("<cmd> required");

  // signals
  signal(SIGTERM, graceful_exit);
  signal(SIGQUIT, graceful_exit);

  // daemonize
  if (daemon) {
    daemonize();
    redirect_stdio_to(logfile);
  }

  monitor(cmd, sleepsec, pidfile);

  return 0;
}
