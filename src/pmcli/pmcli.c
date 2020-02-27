#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

#include <optparse.h>

#include <pm/pm.h>
#include <pm/version.h>

#define PM_DEFAULT_INTERVAL 60000
#define LONG_OPTIONS_COUNT 7
#define LONG_OPTIONS_HELP_SPACE 38
#define TEXT_BUFFER_SIZE 256

#define OPTION_DESCRIPTION_H "produce help message"
#define OPTION_DESCRIPTION_V "print version string"
#define OPTION_DESCRIPTION_O "output file name"
#define OPTION_DESCRIPTION_I "interval in ms (default 60000)"
#define OPTION_DESCRIPTION_P "monitoring process id (multiple separated by ,)"
#define OPTION_DESCRIPTION_N "monitoring process name (multiple separated by ;)"
#define OPTION_DESCRIPTION_T "memory type (see list below)"
#define OPTION_DESCRIPTION_C "configuration file name"

#ifdef _WIN32
#define SLEEPER_NAME "Sleeper"
#define MUTEX_NAME "BreakMutex"
#define MUTEX_WAIT_TIME 10000
#else
#ifdef CLOCK_MONOTONIC
#define PM_CLOCK CLOCK_MONOTONIC
#else
#define PM_CLOCK CLOCK_REALTIME
#endif
#endif

#ifdef _WIN32
HANDLE ghMutex;
HANDLE ghSleeper;
#else
#endif

static char text_buffer[TEXT_BUFFER_SIZE];

struct optparse_description {
  const char* description;
  size_t length;
};

static struct optparse_long longopts[LONG_OPTIONS_COUNT] = {
    {"help", 'h', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},
    {"output", 'o', OPTPARSE_REQUIRED},
    {"interval", 'i', OPTPARSE_REQUIRED},
    {"process-id", 'p', OPTPARSE_REQUIRED},
    {"process-name", 'n', OPTPARSE_REQUIRED},
    {"type", 't', OPTPARSE_REQUIRED}
};

static struct optparse_description longoptsdesc[LONG_OPTIONS_COUNT] = {
  { OPTION_DESCRIPTION_H, sizeof(OPTION_DESCRIPTION_H) },
  { OPTION_DESCRIPTION_V, sizeof(OPTION_DESCRIPTION_V) },
  { OPTION_DESCRIPTION_O, sizeof(OPTION_DESCRIPTION_O) },
  { OPTION_DESCRIPTION_I, sizeof(OPTION_DESCRIPTION_I) },
  { OPTION_DESCRIPTION_P, sizeof(OPTION_DESCRIPTION_P) },
  { OPTION_DESCRIPTION_N, sizeof(OPTION_DESCRIPTION_N) },
  { OPTION_DESCRIPTION_T, sizeof(OPTION_DESCRIPTION_T) }
};

static bool _go;

static void stop_go();
static void show_types();
static void show_help_item(const int index);
static void show_help(char* name);
static void show_version();

#ifdef _WIN32
static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
#else
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
  DWORD wait, sleep;
  ULONGLONG begining, conclusion, elapsed;
  DWORD interval = PM_DEFAULT_INTERVAL;
#else
  struct timespec begining, conclusion;
#endif
  struct optparse options;

  int option, longindex, result = EXIT_SUCCESS;
  bool go;

  optparse_init(&options, argv);
  while ((option = optparse_long(&options, longopts, &longindex)) != -1) {
    switch (option) {

    case '?':
    case 'h':
      show_help(argv[0]);
      goto pm_cli_exit_failure;

    case 'v':
      show_version();
      goto pm_cli_exit_failure;

    case 'o':
      if (options.optarg) {
        if ((result = pm_set_output(options.optarg)) != EXIT_SUCCESS) {
          goto pm_cli_exit_cleanup;
        }
        break;
      } else {
        fprintf(stderr, "Output File Name not specified. "
          "Use --help for usage.\n");
        goto pm_cli_exit_failure;
      }

    case 'i':
      if (options.optarg) {
        interval = atoi(options.optarg);
        if (interval > 0) {
          printf("Interval is set to %d\n", interval);
        } else {
          fprintf(stderr, "Interval must be a number. "
            "Use --help for usage.\n");
          goto pm_cli_exit_failure;
        }
      } else {
        fprintf(stderr, "Interval not specified. "
          "Use --help for usage.\n");
        goto pm_cli_exit_failure;
      }
      break;

    case 'p':
      if (options.optarg) {
        if ((result = pm_add_ids(options.optarg)) != EXIT_SUCCESS) {
          goto pm_cli_exit_cleanup;
        }
      } else {
        fprintf(stderr, "Process IDs not specified. "
          "Use --help for usage.\n");
        goto pm_cli_exit_failure;
      }
      break;

    case 'n':
      if (options.optarg) {
        if ((result = pm_add_names(options.optarg)) != EXIT_SUCCESS) {
          goto pm_cli_exit_cleanup;
        }
      } else {
        fprintf(stderr, "Process Name not specified. "
          "Use --help for usage.\n");
        goto pm_cli_exit_failure;
      }
      break;

    case 't':
      if (options.optarg) {
        if ((result = pm_set_types(options.optarg)) != EXIT_SUCCESS) {
          goto pm_cli_exit_cleanup;
        }
      } else {
        fprintf(stderr, "Type not specified. "
          "Use --help for usage.\n");
        goto pm_cli_exit_failure;
      }
      break;
    }
  }

  if ((result = pm_init()) != EXIT_SUCCESS) {
    goto pm_cli_exit_cleanup;
  }

#ifdef _WIN32
  ghSleeper = CreateEvent(
    NULL,               // Default security attributes
    TRUE,               // Manual-reset event
    FALSE,              // Initial state is non-signaled
    SLEEPER_NAME        // Object name
  );
  if (ghSleeper == NULL) {
    fprintf(stderr, "Failed to create sleeper event\n");
    goto pm_cli_exit_failure;
  }
  ghMutex = CreateMutex(
    NULL,              // Default security attributes
    FALSE,             // Initially not owned
    MUTEX_NAME);       // Mutex name
  if (ghMutex == NULL) {
    fprintf(stderr, "Failed to create mutex\n");
    goto pm_cli_exit_failure;
  }
  if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
    fprintf(stderr, "Failed to set control handler\n");
    goto pm_cli_exit_failure;
  }
#else
#endif

  _go = go = true;

  printf("Press Ctrl-C or Ctrl-Break to stop!\n");

  pm_start();
  while (go) {
#ifdef _WIN32
    begining = GetTickCount64();
#else
    clock_gettime(PM_CLOCK, &begining);
#endif
    printf(".");

    if ((result = pm_loop()) != EXIT_SUCCESS) {
      goto pm_cli_exit_cleanup;
    }

#ifdef _WIN32
    conclusion = GetTickCount64();
    elapsed = conclusion - begining;
    if (elapsed < interval) {
      sleep = interval - (DWORD)(elapsed);
      assert(sleep <= interval);
      wait = WaitForSingleObject(ghSleeper, sleep);
      switch (wait) {
      case WAIT_OBJECT_0:
      case WAIT_TIMEOUT:
        break;
      case WAIT_ABANDONED:
      case WAIT_FAILED:
      default:
        fprintf(stderr, "Waiting for the go lock failed\n");
        goto pm_cli_exit_failure;
      }
    }
#else
    clock_gettime(PM_CLOCK, &begining);
#endif

#ifdef _WIN32
    wait = WaitForSingleObject(ghMutex, MUTEX_WAIT_TIME);
    switch (wait) {
    case WAIT_OBJECT_0:
      break;
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
    default:
      fprintf(stderr, "Waiting for the go lock failed\n");
      goto pm_cli_exit_failure;
    }
    go = _go;
    if (!ReleaseMutex(ghMutex)) {
      fprintf(stderr, "Failed to release the go lock\n");
      goto pm_cli_exit_failure;
    }
#else
#endif
  }
  printf("\n");

  printf("Exiting and cleanup\n");


  /*
   * Exiting
   */

  /* Go to cleanup */
  goto pm_cli_exit_cleanup;

  /* Error */
pm_cli_exit_failure:
  result = EXIT_FAILURE;

  /* Cleanup */
pm_cli_exit_cleanup:

#ifdef _WIN32
  if (ghSleeper) {
    CloseHandle(ghSleeper);
  }
  if (ghMutex) {
    CloseHandle(ghMutex);
  }
#else

#endif
  pm_shutdown();

  return result;
}

void stop_go() {
  DWORD wait;
#ifdef _WIN32
  if (!SetEvent(ghSleeper)) {
    fprintf(stderr, "Setting the sleeper event failed\n");
  }
  wait = WaitForSingleObject(ghMutex, MUTEX_WAIT_TIME);
  switch (wait) {
  case WAIT_OBJECT_0:
    break;
  case WAIT_ABANDONED:
  case WAIT_TIMEOUT:
  case WAIT_FAILED:
  default:
    fprintf(stderr, "Waiting for the go lock failed\n");
  }
  printf("Stopping...");
  _go = false;
  if (!ReleaseMutex(ghMutex)) {
    fprintf(stderr, "Failed to release the go lock\n");
  }
#else
#endif
}

void show_types() {
  int i, length;
  printf("\nTypes\n\n");
  for (i = 0; i < PM_TYPE_COUNT; ++i) {
    if (i != PM_TYPE_DEFAULT_INDEX) {
      printf("  %s: %s\n", pm_type_arr[i].st, pm_type_arr[i].lt);
    } else {
      printf("  %s: %s (default type)\n", pm_type_arr[i].st, pm_type_arr[i].lt);
    }
  }
}

void show_help_item(const int index) {
  const char* description;
  char* text;
  int length;
  description = longoptsdesc[index].description;
  memset(text_buffer, 0x20, TEXT_BUFFER_SIZE);
  length = snprintf(
    text_buffer,
    TEXT_BUFFER_SIZE,
    "-%c [ --%s ]",
    longopts[index].shortname,
    longopts[index].longname);
  text = text_buffer;
  if (length < 200) {
    text += length;
    *text = (char)(0x20);
    text += (size_t)(LONG_OPTIONS_HELP_SPACE - (size_t)(length));
    memcpy(text, description, longoptsdesc[index].length);
  }
  printf("  %s\n", text_buffer);
}

void show_help(char* n) {
  int i;
#ifdef _WIN32
  char* lastslash;
  lastslash = strrchr(n, '\\');
  if (lastslash != NULL) {
    ++lastslash;
    n = lastslash;
  }
#endif
  printf("%s usage:\n\n", n);
  for (i = 0; i < LONG_OPTIONS_COUNT; ++i) {
    show_help_item(i);
  }
  show_types();
  printf("\nExamples:\n\n");
  printf("  %s --process-id 1234,5678\n", n);
#ifdef _WIN32
  printf("  %s --process-name a.exe;b.exe;%s\n", n, n);
  printf("  %s  --process-id 1234,5678 --process-name a.exe;b.exe;%s\n", n, n);
#else
  printf("  %s --process-name a;b;%s\n", n, n);
  printf("  %s  --process-id 1234,5678 --process-name a;b;%s\n", n, n);
#endif

#ifdef PM_SHOW_SHORT_EXAMPLES
  printf("  %s -p 1234\n", n);
  printf("  %s -n %s\n", n, n);
#endif
}

void show_version() {
  printf("%s\n", PM_VERSION_TEXT_WITH_ALL);
}

#ifdef _WIN32
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
  switch (fdwCtrlType) {
  case CTRL_C_EVENT:
    printf("Stop by Ctrl-C event...");
    stop_go();
    return TRUE;
  case CTRL_CLOSE_EVENT:
    printf("Stop by Ctrl-Close event...");
    stop_go();
    return TRUE;
  case CTRL_BREAK_EVENT:
    printf("Stop by Ctrl-Break event...");
    stop_go();
    return FALSE;
  case CTRL_LOGOFF_EVENT:
    printf("Stop by Ctrl-Logoff event...");
    stop_go();
    return FALSE;
  case CTRL_SHUTDOWN_EVENT:
    printf("Stop by Ctrl-Shutdown event...");
    stop_go();
    return FALSE;
  default:
    return FALSE;
  }
}
#else
#endif
