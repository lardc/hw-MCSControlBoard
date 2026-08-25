#ifndef __SELFTEST_H
#define __SELFTEST_H
// Includes
#include "stdinc.h"
// Defines
#define SELFTEST_FAIL_OSH1		0x01
#define SELFTEST_FAIL_OSH2		0x02
#define SELFTEST_IN_PROGRESS	0xFFFF

Int16U SELFTEST_Run();

#endif // __SELFTEST_H
