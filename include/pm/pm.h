#ifndef PM_H_
#define PM_H_

#define PM_TYPE_COUNT 9
#define PM_TYPE_DEFAULT_INDEX 2

enum Pm_Type {
  PM_TYPE_UNDEFINED,
  PM_TYPE_PAGE_FAULT_COUNT,
  PM_TYPE_PEAK_WORKING_SET_SIZE,
  PM_TYPE_WORKING_SET_SIZE,
  PM_TYPE_QUOTA_PEAK_PAGED_POOL_USAGE,
  PM_TYPE_QUOTA_PAGED_POOL_USAGE,
  PM_TYPE_QUOTA_PEAK_NON_PAGED_POOL_USAGE,
  PM_TYPE_QUOTA_NON_PAGED_POOL_USAGE,
  PM_TYPE_PAGEFILE_USAGE,
  PM_TYPE_PEAK_PAGEFILE_USAGE,
  PM_TYPE_UNKNOWN
};

struct pm_type {
  const char* lt;
  const char* st;
  int type;
};

extern struct pm_type pm_type_arr[PM_TYPE_COUNT];

 int pm_add_ids(char* ids);
 int pm_add_names(char* names);

 int pm_set_types(char* types);
 int pm_set_output(char* filename);

 int pm_init();
void pm_start();

 int pm_loop();
void pm_shutdown();

#endif
