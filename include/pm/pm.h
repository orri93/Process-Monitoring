#ifndef PM_H_
#define PM_H_

int pm_add_ids(char* ids);
int pm_add_names(char* names);

int pm_set_output(char* filename);

int pm_init();

void pm_start();

int pm_loop();

void pm_shutdown();

#endif
