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

static char* outputfilename = NULL;
static FILE* outputfile = NULL;

static int* monitoringid = NULL;
static char** monitoringname = NULL;
static unsigned long long* monitoring = NULL;

static size_t monitoringidcount = 0;
static size_t monitoringnamecount = 0;
static size_t monitoringcount = 0;

static size_t length;

#ifdef _WIN32
static ULONGLONG inittime, current;
static DWORD pids[PM_PROCESS_ARRAY_SIZE];
static DWORD penums;
static DWORD menums;
PROCESS_MEMORY_COUNTERS pmc;
HMODULE hmodule;
HANDLE hprocess;
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

static int pm_is_monitored_id(const int id);
static int pm_is_monitored_name(const char* name);
static int pm_write_header();
static size_t pm_count_delimiters(char* s, char ch);

int pm_add_ids(char* ids) {
  int id;
  char* token;
  monitoringidcount = pm_count_delimiters(ids, ',');
  if (monitoringidcount > 0) {
    monitoringid = (int*)(malloc(monitoringidcount * sizeof(int)));
    if (monitoringid) {
      j = 0;
      token = strtok(ids, ",");
      while (token != NULL) {
        length = strlen(token);
        if (length > 0) {
          id = atoi(token);
          if(id > 0) {
            monitoringid[j++] = id;
            printf("Adding ID %d for monitoring\n", id);
            if (j > monitoringidcount) {
              fprintf(stderr, "To many ids!\n");
              return EXIT_FAILURE;
            }
          } else {
            fprintf(stderr, "The id '%s' is not a number\n", token);
            return EXIT_FAILURE;
          }
        } else {
          fprintf(stderr, "Error: Empty process ID number\n");
          return EXIT_FAILURE;
        }
        token = strtok(NULL, ",");
      }
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int pm_add_names(char* names) {
  char* token;
  monitoringnamecount = pm_count_delimiters(names, ';');
  if (monitoringnamecount > 0) {
    monitoringname = (char**)(malloc(monitoringnamecount * sizeof(char*)));
    if (monitoringname) {
      j = 0;
      token = strtok(names, ";");
      while (token != NULL) {
        length = strlen(token);
        if (length > 0) {
          monitoringname[j] = (char*)(malloc(++length));
          if (monitoringname[j] != NULL) {
            strncpy(monitoringname[j], token, length);
            printf("Adding Process '%s' for monitoring\n", monitoringname[j]);
            j++;
            if (j > monitoringnamecount) {
              fprintf(stderr, "To many names!\n");
              return EXIT_FAILURE;
            }
          }
        } else {
          fprintf(stderr, "Error: Empty process name\n");
          return EXIT_FAILURE;
        }
        token = strtok(NULL, ";");
      }
    } else {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }
  } else {
    fprintf(stderr, "Name string is empty\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int pm_set_output(char* filename) {
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

int pm_init() {
  int result;

  monitoringcount = monitoringidcount + monitoringnamecount;

  if (monitoringcount > 0) {
    monitoring = (unsigned long long*)malloc(
      monitoringcount * sizeof(unsigned long long));
    if (monitoring == NULL) {
      fprintf(stderr, ERROR_TEXT_MEMORY);
      return EXIT_FAILURE;
    }

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

    if ((result = pm_write_header()) != EXIT_SUCCESS) {
      return result;
    }

    return EXIT_SUCCESS;
  } else {
#ifdef _WIN32
    current = GetTickCount64();
    elapsed = current - inittime;
    if (EnumProcesses(pids, sizeof(pids), &penums)) {
      pcount = penums / sizeof(DWORD);
      for (i = 0; i < pcount; ++i) {
        processname[0] = '\0';
        hprocess = OpenProcess(
          PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
          FALSE,
          pids[i]);
        if (hprocess) {
          if (EnumProcessModules(
            hprocess,
            &hmodule,
            sizeof(hmodule),
            &menums)) {
            if (GetModuleBaseNameA(
              hprocess,
              hmodule,
              processname,
              sizeof(processname) / sizeof(CHAR)) == 0) {
              processname[0] = '\0';
            }
          }
          CloseHandle(hprocess);
        }
        if (processname[0] != '\0') {
          printf("%d - %s\n", pids[i], processname);
        } else {
          printf("%d\n", pids[i]);
        }
      }
    }
#else
#endif
    printf("\nNothing to monitor. "
      "Please select a process from the list above to monitor.\n");
    return EXIT_FAILURE;
  }
}

void pm_start() {
#ifdef _WIN32
  inittime = GetTickCount64();
#else
#endif
}

int pm_loop() {
  int monitoring_index;
  struct tm tsr;
  memset(monitoring, 0x00, monitoringcount * sizeof(unsigned long long));
  if (outputfile) {
#ifdef _WIN32
    current = GetTickCount64();
    elapsed = current - inittime;
    if (EnumProcesses(pids, sizeof(pids), &penums)) {
      pcount = penums / sizeof(DWORD);
      for (i = pcount - 1; i >= 0; --i) {
        hprocess = OpenProcess(
          PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
          FALSE,
          pids[i]);
        if (hprocess) {
          if ((monitoring_index = pm_is_monitored_id(pids[i])) >= 0) {
            if (GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc))) {
              monitoring[monitoring_index] = pmc.WorkingSetSize;
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
                if ((monitoring_index = pm_is_monitored_name(processname)) >= 0) {
                  if (GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc))) {
                    monitoring[monitoring_index] = pmc.WorkingSetSize;
                  }
                }
              }
            }
          }
          CloseHandle(hprocess);
        } else {
          if (pm_is_monitored_id(pids[i]) >= 0) {
            fprintf(stderr, "Failed to open process %d\n", pids[i]);
          }
        }
      }
    } else {
      pcount = -1;
      fprintf(stderr, "Failed to enumerate processes\n");
    }
#else
#endif

    currtime = time(NULL);
    gmtime_s(&tsr, &currtime);
    strftime(pm_text_buffer, PM_TEXT_BUFFER_SIZE, "%y-%m-%d,%H:%M:%S", &tsr);
    fprintf(outputfile, "%s", pm_text_buffer);
    fprintf(outputfile, ",%llu", elapsed);
    for (j = 0; j < monitoringcount; ++j) {
      fprintf(outputfile, ",%llu", monitoring[j]);
    }
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

  if (outputfilename) {
    free(outputfilename);
    outputfilename = NULL;
  }
}

int pm_is_monitored_id(const int id) {
  for (j = 0; j < monitoringidcount; ++j) {
    if (monitoringid[j] == id) {
      return j;
    }
  }
  return -1;
}

int pm_is_monitored_name(const char* name) {
  for (j = 0; j < monitoringnamecount; ++j) {
    if (strncmp(name, monitoringname[j], PM_PROCESS_NAME_SIZE) == 0) {
      return ((int)(monitoringidcount)) + j ;
    }
  }
  return -1;
}

int pm_write_header() {
  if (outputfile) {
    fprintf(outputfile, "date,time,elapsed");
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

size_t pm_count_delimiters(char* s, char ch) {
  size_t count = 0;
  length = strlen(s);
  if (length > 0) {
    for (count = 0; s[count]; s[count] == ch ? (count++) : (*(s++)));
    ++count;
  }
  return count;
}