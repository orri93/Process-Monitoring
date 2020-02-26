#ifndef PM_H_
#define PM_H_

int pm_init_id(const int id);
int pm_init_name(const char* name);
int pm_init_configuration(const char* filename);

int pm_set_output(const char* filename);

int pm_is_initialized();

int pm_loop();

void pm_shutdown();

#endif
