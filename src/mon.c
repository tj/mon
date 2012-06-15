
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
#include "commander.h"
#include "ms.h"

/*
 * Program version.
 */

#define VERSION "0.0.3"

/*
 * Log prefix.
 */

static const char *prefix = NULL;

/*
 * Child process PID.
 */

static pid_t pid;

/*
 * Monitor.
 */

typedef struct {
  const char *pidfile;
  const char *mon_pidfile;
  const char *logfile;
  int daemon;
  int sleepsec;
} monitor_t;

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
write_pidfile(const char *file, pid_t pid) {
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
show_status_of(const char *pidfile) {
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
redirect_stdio_to(const char *file) {
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
start(const char *cmd, int sleepsec, const char *pidfile) {
exec: {
  pid = fork();
  int status;

  switch (pid) {
    case -1:
      perror("fork()");
      exit(1);
    case 0:
      log("sh -c \"%s\"", cmd);
      execl("/bin/sh", "sh", "-c", cmd, 0);
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
 * --log <path>
 */

static void
on_log(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  monitor->logfile = self->arg;
}

/*
 * --sleep <sec>
 */

static void
on_sleep(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  monitor->sleepsec = atoi(self->arg);
}

/*
 * --daemonize
 */

static void
on_daemonize(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  monitor->daemon = 1;
}

/*
 * --pidfile <path>
 */

static void
on_pidfile(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  monitor->pidfile = self->arg;
}

/*
 * --mon-pidfile <path>
 */

static void
on_mon_pidfile(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  monitor->mon_pidfile = self->arg;
}

/*
 * --status
 */

static void
on_status(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  if (!monitor->pidfile) error("--pidfile required");
  show_status_of(monitor->pidfile);
  exit(0);
}

/*
 * --prefix
 */

static void
on_prefix(command_t *self) {
  monitor_t *monitor = (monitor_t *) self->data;
  prefix = self->arg;
}

/*
 * [options] <cmd>
 */

int
main(int argc, char **argv){
  monitor_t monitor;
  monitor.pidfile = NULL;
  monitor.mon_pidfile = NULL;
  monitor.logfile = "mon.log";
  monitor.daemon = 0;
  monitor.sleepsec = 1;

  command_t program;
  program.data = &monitor;
  command_init(&program, "mon", VERSION);
  command_option(&program, "-l", "--log <path>", "specify logfile [mon.log]", on_log);
  command_option(&program, "-s", "--sleep <sec>", "sleep seconds before re-executing [1]", on_sleep);
  command_option(&program, "-S", "--status", "check status of --pidfile", on_status);
  command_option(&program, "-p", "--pidfile <path>", "write pid to <path>", on_pidfile);
  command_option(&program, "-m", "--mon-pidfile <path>", "write mon(1) pid to <path>", on_mon_pidfile);
  command_option(&program, "-P", "--prefix <str>", "add a log prefix", on_prefix);
  command_option(&program, "-d", "--daemonize", "daemonize the program", on_daemonize);
  command_parse(&program, argc, argv);

  // command required
  if (!program.argc) error("<cmd> required");
  const char *cmd = program.argv[0];
  
  // signals
  signal(SIGTERM, graceful_exit);
  signal(SIGQUIT, graceful_exit);
  
  // daemonize
  if (monitor.daemon) {
    daemonize();
    redirect_stdio_to(monitor.logfile);
  }
  
  // write mon pidfile
  if (monitor.mon_pidfile) {
    log("write mon pid to %s", monitor.mon_pidfile);
    write_pidfile(monitor.mon_pidfile, getpid());
  }
  
  start(cmd, monitor.sleepsec, monitor.pidfile);

  return 0;
}