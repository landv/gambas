/***************************************************************************

  CProcess.c

  The process management class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBX_C_PROCESS_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_common_buffer.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "gb_replace.h"
#include "gbx_api.h"
#include "gambas.h"
#include "gbx_stream.h"
#include "gb_array.h"
#include "gbx_exec.h"
#include "gbx_class.h"
#include "gbx_watch.h"
#include "gbx_project.h"
#include "gbx_c_array.h"

#include "gbx_c_process.h"

//#define DEBUG_ME

extern char **environ;

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Error);
DECLARE_EVENT(EVENT_Kill);

static CPROCESS *RunningProcessList = NULL;
static int _pipe_child[2];
static bool _init = FALSE;

static int _last_status = 0;

static void init_child(void);
static void exit_child(void);

static void callback_write(int fd, int type, CPROCESS *process)
{
  if (process->to_string)
  {
    int n = read(fd, COMMON_buffer, 256);
    if (n > 0)
    {
      STRING_add(&process->result, COMMON_buffer, n);
      CSTREAM_stream(process)->process.read_something = TRUE;
      #ifdef DEBUG_ME
      fprintf(stderr, "callback_write: result = '%s'\n", process->result);
      #endif
    }
  }
  else if (!STREAM_eof(CSTREAM_stream(process))) //process->running &&
    GB_Raise(process, EVENT_Read, 0);
}


static int callback_error(int fd, int type, CPROCESS *process)
{
  /*fprintf(stderr, ">> Write\n"); fflush(stderr);*/
  char buffer[1024];
  int n;

  n = read(fd, buffer, sizeof(buffer));

  if (n <= 0) /* || !process->running)*/
  {
    /*close(process->err);*/
    return TRUE;
  }

  /*printf("callback_error: (%d) %.*s\n", n, n, buffer);*/

  GB_Raise(process, EVENT_Error, 1,
    GB_T_STRING, buffer, n);

  return FALSE;

  /*fprintf(stderr, "<< Write\n"); fflush(stderr);*/
}


static void update_stream(CPROCESS *process)
{
  STREAM *stream = &process->ob.stream;

  stream->type = &STREAM_process;
  (*stream->type->open)(stream, NULL, 0, process);
}


static void init_process(CPROCESS *process)
{
  process->watch = GB_WATCH_NONE;
  process->in = process->out = process->err = -1;
  update_stream(process);
  process->tag.type = GB_T_NULL;
}

static void exit_process(CPROCESS *_object)
{
  /* Normalement impossible */
  /*
  if (THIS->running)
  {
    fprintf(stderr, "WARNING: CPROCESS_free: running ?\n");
    stop_process(THIS);
  }
  */

  #ifdef DEBUG_ME
  fprintf(stderr, "exit_process: %p\n", _object);
  #endif

  if (THIS->in >= 0)
  {
    if (THIS->in != THIS->out)
      close(THIS->in);
    THIS->in = -1;
  }

  if (THIS->out >= 0)
  {
    GB_Watch(THIS->out, GB_WATCH_NONE, NULL, 0);
    close(THIS->out);
    THIS->out = -1;
  }

  if (THIS->err >= 0)
  {
    GB_Watch(THIS->err, GB_WATCH_NONE, NULL, 0);
    close(THIS->err);
    THIS->err = -1;
  }

  update_stream(THIS);
  GB_StoreVariant(NULL, &THIS->tag);
}

static void stop_process_after(CPROCESS *_object)
{
  STREAM *stream;
  //long long len;

  #ifdef DEBUG_ME
  fprintf(stderr, "stop_process_after: %p\n", _object);
  #endif

  /* Vidage du tampon d'erreur */
  if (THIS->err >= 0)
    while (callback_error(THIS->err, 0, THIS) == 0);

  /* Vidage du tampon de sortie */
  if (THIS->out >= 0)
  {
    stream = CSTREAM_stream(THIS);
    while (!STREAM_eof(stream))
    {
      stream->process.read_something = FALSE;
      callback_write(THIS->out, 0, THIS);
      //GB_Raise(THIS, EVENT_Read, 0);
      if (!stream->process.read_something)
        break;
    }
  }

  GB_Raise(THIS, EVENT_Kill, 0);

  exit_process(THIS);

  GB_Detach(THIS);

  /*printf("** stop_process_after\n");*/
  //GB_Unref((void **)&_object); /* Ref du post */
}


static void stop_process(CPROCESS *process)
{
  if (!process->running)
    return;

  #ifdef DEBUG_ME
  fprintf(stderr, "stop_process: %p\n", process);
  #endif

  /*process->pid = -1;*/

  /* Remove from running process list */

  if (process->prev)
    process->prev->next = process->next;

  if (process->next)
    process->next->prev = process->prev;

  if (process == RunningProcessList)
    RunningProcessList = process->next;

  process->running = FALSE;

  /*printf("** stop_process\n");*/

  stop_process_after(process);

  OBJECT_UNREF(&process, "stop_process"); /* Le processus ne tourne plus */

  if (!RunningProcessList)
    exit_child();
}

static void run_process(CPROCESS *process, int mode, void *cmd)
{
  static const char *shell[] = { "sh", "-c", NULL, NULL };

  int fdin[2], fdout[2], fderr[2];
  pid_t pid;
  char **argv;
  CARRAY *array;
  int n;
  sigset_t sig, old;

  /* for terminal */
  int fd_master = -1;
  int fd_slave;
  char *slave = NULL;
  struct termios termios_stdin;
  struct termios termios_master;

  init_child();

  if (mode & PM_SHELL)
  {
    #ifdef DEBUG_ME
    fprintf(stderr, "run_process %p: %s\n", process, (char *)cmd);
    #endif

    argv = (char **)shell;

    argv[2] = (char *)cmd;

    if (argv[2] == NULL || *argv[2] == 0)
    {
      /*stop_process(process, FALSE);*/
      return;
    }
  }
  else
  {
    array = (CARRAY *)cmd;
    n = ARRAY_count(array->data);

    if (n == 0)
    {
      /*stop_process(process, FALSE);*/
      return;
    }

    ALLOC(&argv, sizeof(*argv) * (n + 1), "run_process");
    memcpy(argv, array->data, sizeof(*argv) * n);
    argv[n] = NULL;

    #ifdef DEBUG_ME
    {
      int i;

      fprintf(stderr, "run_process %p: ", process);
      for (i = 0; i < n; i++)
        fprintf(stderr, "%s ", argv[i]);
      fprintf(stderr, "\n");
    }
    #endif
  }

  /*printf("** run_process\n");*/
  GB_Ref(process); /* Process is running */

  /* Adding to the running process list */

  if (RunningProcessList)
    RunningProcessList->prev = process;

  process->next = RunningProcessList;
  process->prev = NULL;

  RunningProcessList = process;

  process->running = TRUE;

  if (mode & PM_STRING)
  {
    process->to_string = TRUE;
    process->result = NULL;
    mode |= PM_READ;
  }

  if (mode & PM_TERM)
  {
    fd_master = getpt();
    if (fd_master < 0)
      THROW_SYSTEM(errno, NULL);

    grantpt(fd_master);
    unlockpt(fd_master);

    slave = ptsname(fd_master);
    tcgetattr(STDIN_FILENO, &termios_stdin);
  }
  else
  {
    /* Create pipes */

    if ((mode & PM_WRITE) && pipe(fdin) != 0)
      THROW_SYSTEM(errno, NULL);

    if ((mode & PM_READ) && (pipe(fdout) != 0 || pipe(fderr) != 0))
      THROW_SYSTEM(errno, NULL);
  }

  /* Block SIGCHLD */

  sigemptyset(&sig);

  sigaddset(&sig, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sig, &old);

  pid = fork();
  if (pid == (-1))
  {
    stop_process(process);
    sigprocmask(SIG_SETMASK, &old, &sig);
    THROW_SYSTEM(errno, NULL);
  }

  if (pid)
  {
    process->pid = pid;

    #ifdef DEBUG_ME
    fprintf(stderr, "fork: pid = %d\n", pid);
    #endif

    /*printf("Running process %d\n", pid);
    fflush(NULL);*/

    if (mode & PM_TERM)
    {
      tcgetattr(fd_master, &termios_master);
      cfmakeraw(&termios_master);
      //termios_master.c_lflag &= ~ECHO;
      tcsetattr(fd_master, TCSANOW, &termios_master);
    }

    if (mode & PM_WRITE)
    {
      if (mode & PM_TERM)
      {
        process->in = fd_master;
      }
      else
      {
        close(fdin[0]);
        process->in = fdin[1];
      }
    }

    if (mode & PM_READ)
    {
      if (mode & PM_TERM)
      {
        process->out = fd_master;
        process->err = -1;
      }
      else
      {
        close(fdout[1]);
        close(fderr[1]);

        process->out = fdout[0];
        process->err = fderr[0];
      }

      fcntl(process->err, F_SETFL, fcntl(process->err, F_GETFL) | O_NONBLOCK);

      GB_Watch(process->out, GB_WATCH_READ, (void *)callback_write, (long)process);
      if (process->err >= 0)
        GB_Watch(process->err, GB_WATCH_READ, (void *)callback_error, (long)process);
    }

    if ((mode & PM_SHELL) == 0)
    {
      FREE(&argv, "run_process");
    }

    sigprocmask(SIG_SETMASK, &old, &sig);
  }
  else /* child */
  {
    sigprocmask(SIG_SETMASK, &old, &sig);

    if (mode & PM_TERM)
    {
      close(fd_master);
      setsid();
      fd_slave = open(slave, O_RDWR);
      if (fd_slave < 0)
        exit(127);
      tcsetattr(fd_slave, TCSANOW, &termios_stdin);

      if (mode & PM_WRITE)
      {
        if (dup2(fd_slave, STDIN_FILENO) == -1)
          exit(127);
      }

      if (mode & PM_READ)
      {
        if ((dup2(fd_slave, STDOUT_FILENO) == -1)
            || (dup2(fd_slave, STDERR_FILENO) == -1))
          exit(127);
      }
    }
    else
    {
      if (mode & PM_WRITE)
      {
        close(fdin[1]);
      /*else
        fdin[0] = open("/dev/null", O_RDONLY);*/

        if (dup2(fdin[0], STDIN_FILENO) == -1)
          exit(127);
      }

      if (mode & PM_READ)
      {
        close(fdout[0]);
        close(fderr[0]);
      /*
      else
      {
        fdout[1] = open("/dev/null", O_WRONLY);
        fderr[1] = fdout[1];
      }
      */

        if ((dup2(fdout[1], STDOUT_FILENO) == -1)
            || (dup2(fderr[1], STDERR_FILENO) == -1))
          exit(127);
      }
    }

		// Return to the parent working directory
		chdir(PROJECT_oldcwd);
		
    execvp(argv[0], (char **)argv);
    //execve(argv[0], (char **)argv, environ);
    exit(127);
  }

  update_stream(process);
}


static void callback_child(int fd, int type, void *data)
{
  int status;
  CPROCESS *process, *next;
  long buffer;

  /*old = signal(SIGCHLD, signal_child);*/

  read(fd, (char *)&buffer, 1);

  #ifdef DEBUG_ME
  fprintf(stderr, "<< callback_child\n");
  #endif

  for (process = RunningProcessList; process; )
  {
    next = process->next;

    if (wait4(process->pid, &status, WNOHANG, NULL) == process->pid)
    {
      process->status = status;
      _last_status = status;

      #ifdef DEBUG_ME
      fprintf(stderr, "Process %d has returned %d\n", process->pid, status);
      #endif

      /*printf("** signal_child\n");*/

      //GB_Ref(process);
      stop_process(process);
      //stop_process_after(process);
      //break; // one exit at a time // why ?
    }

    process = next;
  }

  #ifdef DEBUG_ME
  fprintf(stderr, ">> callback_child\n");
  #endif
}


static void signal_child(int sig)
{
  #ifdef DEBUG_ME
  fprintf(stderr, "SIGCHLD: _pipe_child[0] = %d\n", _pipe_child[0]);
  #endif
  char buffer;

	if (!_init)
		return;

  buffer = 42;
  write(_pipe_child[1], &buffer, 1);
}


static void init_child(void)
{
  if (_init)
    return;

  #ifdef DEBUG_ME
  fprintf(stderr, "init_child()\n");
  #endif

  if (pipe(_pipe_child) != 0)
    ERROR_panic("Cannot create SIGCHLD pipes");

	fcntl(_pipe_child[0], F_SETFD, FD_CLOEXEC);
	fcntl(_pipe_child[1], F_SETFD, FD_CLOEXEC);

  #ifdef DEBUG_ME
  fprintf(stderr, "_pipe_child[0] = %d\n", _pipe_child[0]);
  #endif

  /* This may work only on Linux. Should use sigaction() instead! */
  signal(SIGCHLD, signal_child);
  GB_Watch(_pipe_child[0], GB_WATCH_READ, (void *)callback_child, 0);
  _init = TRUE;
}

static void exit_child(void)
{
  if (!_init)
    return;

  #ifdef DEBUG_ME
  fprintf(stderr, "exit_child()\n");
  #endif

  GB_Watch(_pipe_child[0], GB_WATCH_NONE, NULL, 0);
  close(_pipe_child[0]);
  close(_pipe_child[1]);
  _init = FALSE;
}


PUBLIC CPROCESS *CPROCESS_create(int mode, void *cmd, char *name)
{
  CPROCESS *process;

  /*printf("** CPROCESS_create <<<< \n");*/

	if (!name || !*name)
		name = "Process";

  OBJECT_new((void **)(void *)&process, CLASS_Process, name, OP  ? (OBJECT *)OP : (OBJECT *)CP);

  init_process(process);
  run_process(process, mode, cmd);

  OBJECT_UNREF_KEEP(&process, "CPROCESS_create");

  /*printf("** CPROCESS_create >>>> \n");*/

  return process;
}


/* Attention, l'objet Process est d���enc� une fois la fonction termin� ! */

PUBLIC void CPROCESS_wait_for(CPROCESS *process)
{
  //sigset_t sig, old;
  //bool have_sigchld;
  int ret;

  #ifdef DEBUG_ME
  printf("Waiting for %d\n", process->pid);
  #endif

  /*sigemptyset(&sig);
  sigaddset(&sig, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sig, &old);

  have_sigchld = sigismember(&old, SIGCHLD);
  if (have_sigchld)
    sigdelset(&old, SIGCHLD);*/

  /*if (process->to_string)
  {
    while (process->running)
      WATCH_loop();
  }
  else
  {
    while (process->running)
      WATCH_loop();
      //sigsuspend(&old);
  }*/
  OBJECT_REF(process, "CPROCESS_wait_for");
  while (process->running)
  {
    //HOOK_DEFAULT(wait, WATCH_wait)(0);
    #ifdef DEBUG_ME
    fprintf(stderr, "watching _pipe_child[0] = %d\n", _pipe_child[0]);
    #endif
    ret = WATCH_process(_pipe_child[0], process->out);
    if (ret == _pipe_child[0])
      callback_child(_pipe_child[0], GB_WATCH_READ, 0);
    else if (ret == process->out)
      callback_write(process->out, GB_WATCH_READ, process);
  }
  OBJECT_UNREF(&process, "CPROCESS_wait_for");

  #if 0
  {
    sigsuspend(&old);
    if (!process->running)
      break;

    #ifdef DEBUG_ME
    fprintf(stderr, "Waiting: %d\n", process->running);
    #endif

    sleep(10);
  }
  #endif

  /*if (have_sigchld)
    sigaddset(&old, SIGCHLD);
  sigprocmask(SIG_SETMASK, &old, NULL);*/

  #ifdef DEBUG_ME
  printf("Got it !\n");
  #endif
}



BEGIN_METHOD_VOID(CPROCESS_exit)

  //fprintf(stderr, "CPROCESS_exit\n");

  while (RunningProcessList)
    stop_process(RunningProcessList);

  exit_child();

END_METHOD


BEGIN_METHOD_VOID(CPROCESS_new)

  init_process(THIS);

END_METHOD


BEGIN_METHOD_VOID(CPROCESS_free)

  #ifdef DEBUG_ME
  printf("CPROCESS_free %p\n", THIS);
  #endif

  exit_process(THIS);

END_METHOD


BEGIN_PROPERTY(CPROCESS_id)

  GB_ReturnInt(THIS->pid);

END_PROPERTY


BEGIN_METHOD_VOID(CPROCESS_kill)

  if (!THIS->running)
    return;

  kill(THIS->pid, SIGKILL);
  CPROCESS_wait_for(THIS);

END_METHOD

BEGIN_METHOD_VOID(CPROCESS_signal)

  if (!THIS->running)
    return;

  /*
  printf("Send SIGUSR1 to process %d\n", THIS->pid);
  fflush(NULL);*/
  kill(THIS->pid, SIGUSR1);

END_METHOD


#if 0
BEGIN_METHOD(CPROCESS_send, GB_STRING text)

  if (!THIS->running || THIS->in < 0)
    return;

  STREAM_write(&THIS->ob.stream, STRING(text), LENGTH(text));

END_METHOD
#endif

BEGIN_PROPERTY(CPROCESS_state)

  if (THIS->running)
    GB_ReturnInteger(1);
  else
  {
    if (WIFEXITED(THIS->status))
      GB_ReturnInteger(0);
    else
      GB_ReturnInteger(2);
  }

END_PROPERTY


BEGIN_PROPERTY(CPROCESS_value)

  int status;

  if (THIS->running)
  {
    GB_ReturnInteger(0);
    return;
  }

  status = THIS->status;

  if (WIFEXITED(status))
    GB_ReturnInteger(WEXITSTATUS(status));
  else if (WIFSIGNALED(status))
    GB_ReturnInteger(WTERMSIG(status));
  else
    GB_ReturnInteger(-1);

END_PROPERTY

BEGIN_PROPERTY(CPROCESS_last_state)

	if (WIFEXITED(_last_status))
		GB_ReturnInteger(0);
	else
		GB_ReturnInteger(2);

END_PROPERTY


BEGIN_PROPERTY(CPROCESS_last_value)

  int status;

  status = _last_status;

  if (WIFEXITED(status))
    GB_ReturnInteger(WEXITSTATUS(status));
  else if (WIFSIGNALED(status))
    GB_ReturnInteger(WTERMSIG(status));
  else
    GB_ReturnInteger(-1);

END_PROPERTY

BEGIN_PROPERTY(CPROCESS_tag)

  if (READ_PROPERTY)
    GB_ReturnPtr(GB_T_VARIANT, &THIS->tag);
  else
    GB_StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD

BEGIN_METHOD_VOID(CPROCESS_wait)

	CPROCESS_wait_for(THIS);

END_METHOD

#endif

PUBLIC GB_DESC NATIVE_Process[] =
{
  GB_DECLARE("Process", sizeof(CPROCESS)), GB_NOT_CREATABLE(),
  GB_INHERITS("Stream"),

  GB_CONSTANT("Stopped", "i", 0),
  GB_CONSTANT("Running", "i", 1),
  GB_CONSTANT("Crashed", "i", 2),
  GB_CONSTANT("Signaled", "i", 2),

  GB_STATIC_PROPERTY_READ("LastState", "i", CPROCESS_last_state),
  GB_STATIC_PROPERTY_READ("LastValue", "i", CPROCESS_last_value),
  
  GB_PROPERTY_READ("Id", "i", CPROCESS_id),
  GB_PROPERTY_READ("Handle", "i", CPROCESS_id),
  GB_PROPERTY_READ("State", "i", CPROCESS_state),
  GB_PROPERTY_READ("Value", "i", CPROCESS_value),
  GB_PROPERTY("Tag", "v", CPROCESS_tag),

  //GB_STATIC_METHOD("_init", NULL, CPROCESS_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CPROCESS_exit, NULL),
  GB_METHOD("_new", NULL, CPROCESS_new, NULL),
  GB_METHOD("_free", NULL, CPROCESS_free, NULL),

  GB_METHOD("Kill", NULL, CPROCESS_kill, NULL),
  GB_METHOD("Signal", NULL, CPROCESS_signal, NULL),
  GB_METHOD("Wait", NULL, CPROCESS_wait, NULL),

  //GB_METHOD("Send", NULL, CPROCESS_send, "(Input)s"),
  //GB_METHOD("_print", NULL, CPROCESS_send, "(Input)s"),

  GB_EVENT("Read", NULL, NULL, &EVENT_Read),
  GB_EVENT("Error", NULL, "(Error)s", &EVENT_Error),
  GB_EVENT("Kill", NULL, NULL, &EVENT_Kill),

  GB_END_DECLARE
};

