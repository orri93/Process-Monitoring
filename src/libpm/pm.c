#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#endif

#include <pm/pm.h>

#define DEFAULT_OUTPUT_FILE_NAME "pm.csv"

#define ERROR_TEXT_MEMORY \
  "Memory error\n"
#define ERROR_TEXT_OUTPUT_FILE_NOT_OPEN \
  "The output file is not open\n"
#define ERROR_TEXT_FAILED_FLUSH_OUTPUT_FILE \
  "Failed to flush the output file\n"
#define ERROR_TEXT_NOT_INITIALIZED \
  "Process Monitoring has not been initialized\n"
#define ERROR_TEXT_ALREADY_INITIALIZED \
  "Process Monitoring has already been initialized\n"
#define ERROR_TEXT_OUTPUT_ALREADY_SET \
  "Process Monitoring output file name has already been set\n"

#define PM_TEXT_BUFFER_SIZE 256

#ifdef _WIN32
#define PM_PROCESS_ARRAY_SIZE 1024
#define PM_PROCESS_NAME_SIZE MAX_PATH
#else
#endif

enum {
  PM_MODE_UNDEFINED,
  PM_MODE_PID,
  PM_MODE_NAME,
  PM_MODE_CONFIGURATION
};

static int mode = PM_MODE_UNDEFINED;
static char* configuration = NULL;
static char* outputfilename = NULL;
static FILE* outputfile = NULL;

static int* monitoringid = NULL;
static char** monitoringname = NULL;
static unsigned long long* monitoring = NULL;

static int monitoringidcount = 0;
static int monitoringnamecount = 0;

static size_t length;

#ifdef _WIN32
static ULONGLONG inittime, current;
static DWORD pids[PM_PROCESS_ARRAY_SIZE];
static DWORD penums;
static DWORD menums;
PROCESS_MEMORY_COUNTERS pmc;
HANDLE hprocess;
HANDLE hmodule;
#else
struct timespec begining, conclusion;
#endif

static int j;
static int i;
static int pcount;
static unsigned long long elapsed;
static char processname[PM_PROCESS_NAME_SIZE];
static time_t currtime;

static char pm_text_buffer[PM_TEXT_BUFFER_SIZE];

static bool pm_is_monitored_id(const int id);
static bool pm_is_monitored_name(const char* name);
static int pm_write_header();
static int pm_init();

int pm_init_id(const int id) {
  if (mode == PM_MODE_UNDEFINED) {
    monitoringidcount = 1;
    monitoringid = malloc(monitoringidcount * sizeof(int));
    if (monitoringid) {
      monitoringid[0] = id;
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
    mode = PM_MODE_PID;
    return pm_init();
  } else {
    fprintf(stderr, ERROR_TEXT_ALREADY_INITIALIZED);
    return EXIT_FAILURE;
  }
}

int pm_init_name(const char* name) {
  if (mode == PM_MODE_UNDEFINED) {
    monitoringnamecount = 1;
    monitoringname = (char**)(malloc(monitoringnamecount * sizeof(char*)));
    if (monitoringname != NULL) {
      length = (size_t)(strlen(name)) + 1;
      monitoringname[0] = (char*)(malloc(length));
      if (monitoringname[0] != NULL) {
        strncpy(monitoringname[0], name, length);
        printf("Process name is %s\n", name);
      } else {
        fprintf(stderr, ERROR_TEXT_MEMORY);
        return EXIT_FAILURE;
      }
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
    mode = PM_MODE_NAME;
    return pm_init();
  } else {
    fprintf(stderr, ERROR_TEXT_ALREADY_INITIALIZED);
    return EXIT_FAILURE;
  }
}

int pm_init_configuration(const char* filename) {
  if (mode == PM_MODE_UNDEFINED) {
    length = (size_t)(strlen(filename)) + 1;
    configuration = malloc(length);
    if (configuration) {
      strncpy(configuration, filename, length);
      printf("Configuration file name is %s\n", configuration);
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
    mode = PM_MODE_CONFIGURATION;
    return pm_init();
  } else {
    fprintf(stderr, ERROR_TEXT_ALREADY_INITIALIZED);
    return EXIT_FAILURE;
  }
}

int pm_set_output(const char* filename) {
  if (!outputfilename) {
    length = (size_t)(strlen(filename)) + 1;
    outputfilename = malloc(length);
    if (outputfilename) {
      strncpy(outputfilename, filename, length);
      printf("Output file name is %s\n", outputfilename);
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, ERROR_TEXT_OUTPUT_ALREADY_SET);
    return EXIT_FAILURE;
  }
}

int pm_is_initialized() {
  if (mode != PM_MODE_UNDEFINED) {
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, ERROR_TEXT_NOT_INITIALIZED);
    return EXIT_FAILURE;
  }
}

int pm_loop() {
  struct tm tsr;
  if (outputfile) {
    currtime = time(NULL);
    gmtime_s(&tsr, &currtime);
    strftime(pm_text_buffer, PM_TEXT_BUFFER_SIZE, "%y-%m-%d %H:%M:%S", &tsr);
    fprintf(outputfile, "%s", pm_text_buffer);
#ifdef _WIN32
    current = GetTickCount64();
    elapsed = current - inittime;
    fprintf(outputfile, ",%llu", elapsed);
    if (EnumProcesses(pids, sizeof(pids), &penums)) {
      pcount = penums / sizeof(DWORD);
      for (i = pcount - 1; i >= 0; --i) {
        hprocess = OpenProcess(
          PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
          FALSE,
          pids[i]);
        if (hprocess) {
          if (pm_is_monitored_id(pids[i])) {
            if (GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc))) {
              fprintf(outputfile, ",%llu", pmc.WorkingSetSize);
            }
          } else {
            if (EnumProcessModules(
              hprocess,
              &hmodule,
              sizeof(hmodule),
              &menums)) {
              if (GetModuleBaseNameA(
                hprocess,
                hmodule,
                processname,
                sizeof(processname) / sizeof(CHAR)) > 0) {
                if (pm_is_monitored_name(processname)) {
                  if (GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc))) {
                    fprintf(outputfile, ",%llu", pmc.WorkingSetSize);
                  }
                }
              }
            }
          }
          CloseHandle(hprocess);
        } else {
          if (pm_is_monitored_id(pids[i])) {
            fprintf(stderr, "Failed to open process %d\n", pids[i]);
          }
        }
      }
    } else {
      for (j = 0; j < monitoringidcount; ++j) {
        fprintf(outputfile, ",-1");
      }
      for (j = 0; j < monitoringnamecount; ++j) {
        fprintf(outputfile, ",-1");
      }
      pcount = -1;
      fprintf(stderr, "Failed to enumerate processes\n");
    }
#else
#endif

    fprintf(outputfile, ",%d\n", pcount);

    if (fflush(outputfile) != 0) {
      fprintf(stderr, ERROR_TEXT_FAILED_FLUSH_OUTPUT_FILE);
    }
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, ERROR_TEXT_OUTPUT_FILE_NOT_OPEN);
    return EXIT_FAILURE;
  }
}

void pm_shutdown() {
  if (outputfile) {
    if (fclose(outputfile) == 0) {
      printf("Closed the output file\n");
    } else {
      fprintf(stderr, "Failed to closed output file\n");
    }
    outputfile = NULL;
  }

  if (monitoringname) {
    for (j = 0; j < monitoringnamecount; ++j) {
      free(monitoringname[j]);
    }
    free(monitoringname);
    monitoringname = NULL;
  }

  if (monitoringid) {
    free(monitoringid);
    monitoringid = NULL;
  }

  if (monitoring) {
    free(monitoring);
    monitoring = NULL;
  }

  if (configuration) {
    free(configuration);
    configuration = NULL;
  }

  if (outputfilename) {
    free(outputfilename);
    outputfilename = NULL;
  }

  mode = PM_MODE_UNDEFINED;
}

bool pm_is_monitored_id(const int id) {
  for (j = 0; j < monitoringidcount; ++j) {
    if (monitoringid[j] == id) {
      return true;
    }
  }
  return false;
}

bool pm_is_monitored_name(const char* name) {
  for (j = 0; j < monitoringnamecount; ++j) {
    if (strncmp(name, monitoringname[j], PM_PROCESS_NAME_SIZE) == 0) {
      return true;
    }
  }
  return false;
}

int pm_write_header() {
  if (outputfile) {
    fprintf(outputfile, "time,elapsed");
    for (j = 0; j < monitoringidcount; ++j) {
      fprintf(outputfile, ",%d", monitoringid[j]);
    }
    for (j = 0; j < monitoringnamecount; ++j) {
      fprintf(outputfile, ",%s", monitoringname[j]);
    }
    fprintf(outputfile, ",count\n");
    if (fflush(outputfile) != 0) {
      fprintf(stderr, ERROR_TEXT_FAILED_FLUSH_OUTPUT_FILE);
    }
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, ERROR_TEXT_OUTPUT_FILE_NOT_OPEN);
    return EXIT_FAILURE;
  }
}

int pm_init() {
  
  int result;

  if (outputfilename == NULL) {
    length = sizeof(DEFAULT_OUTPUT_FILE_NAME);
    outputfilename = malloc(length);
    if (outputfilename != NULL) {
      memcpy(
        outputfilename,
        DEFAULT_OUTPUT_FILE_NAME,
        sizeof(DEFAULT_OUTPUT_FILE_NAME));
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
  }

  if (outputfilename != NULL) {
    outputfile = fopen(outputfilename, "w+");
    if (outputfile) {
      printf(
        "Output file '%s' has been opened\n",
        outputfilename);
    } else {
      fprintf(
        stderr,
        "Failed to open output file '%s'\n",
        outputfilename);
      return EXIT_FAILURE;
    }
  }

#ifdef _WIN32
  inittime = GetTickCount64();
#else
#endif

  if ((result = pm_write_header()) != EXIT_SUCCESS) {
    return result;
  }

  return EXIT_SUCCESS;
}
