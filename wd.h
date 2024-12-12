#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__

#include <stddef.h> /*  size_t */

int MMI(size_t interval_in_seconds, size_t repetitions, char **argv); /* argv 0 argument is path*/

void DNR(void);

#endif /*__WATCH_DOG_H__*/