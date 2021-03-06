
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

#ifdef __APPLE__

#include <mach/clock.h>
#include <mach/mach.h>

#endif

#define UNUSED __attribute__ ((unused))

/* Get current wall-clock time and return it in microseconds since the Unix
 * epoch.
 *
 * CLOCK_REALTIME should be used to get the system's best guess at real time.
 * CLOCK_MONOTONIC should be used when jumps in time would cause trouble.
 */
#ifdef __APPLE__
uint64_t clock_gettime_us(clock_t clock_id UNUSED)
{
    clock_serv_t cclock;
    mach_timespec_t ts;

    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &ts);
    mach_port_deallocate(mach_task_self(), cclock);

    return (uint64_t)ts.tv_sec * 1000000ULL + (ts.tv_nsec + 500) / 1000;
}
#else
uint64_t clock_gettime_us(clock_t clock_id)
{
    struct timespec ts;

    clock_gettime(clock_id, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL + (ts.tv_nsec + 500) / 1000;
}
#endif

/* Set clock.
 * This is usually used with CLOCK_REALTIME.
 */
#ifdef __APPLE__
void clock_settime_us(clock_t clock_id UNUSED, uint64_t t_us)
{
    mach_timespec_t ts;

    ts.tv_sec = t_us / 1000000;
    ts.tv_nsec = (t_us % 1000000) * 1000;

    clock_serv_t cclock;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_set_time(cclock, ts);
    mach_port_deallocate(mach_task_self(), cclock);
}
#else
void clock_settime_us(clock_t clock_id, uint64_t t_us)
{
    struct timespec ts;

    ts.tv_sec = t_us / 1000000;
    ts.tv_nsec = (t_us % 1000000) * 1000;

    clock_settime(clock_id, &ts);
}
#endif


#ifndef _WIN32

/* buf must be at least 24 characters */
const char *clock_tostr_r(uint64_t t_us, char *buf)
{
    time_t s = t_us / 1000000;
    unsigned us = t_us % 1000000;
    struct tm now_tm;
    size_t num_chars;

    localtime_r(&s, &now_tm);

    /* %F is %Y-%m-%d (4+1+2+1+2=10 chars)
     * %T is %H:%M:%S (2+1+2+1+2=8 chars)
     * total is then 10+1+8+1=20 chars, 21 with EOS */
    num_chars = strftime(buf, 21, "%F %T,", &now_tm);

    /* append 3 chars, rounding to nearest millisecond */
    sprintf(buf + num_chars, "%03d", (us + 500) / 1000);

    return buf;
}

/* Get a clock and return as human-readable string. */
const char *clock_gettime_str_r(clock_t clock_id, char *buf)
{
    return clock_tostr_r(clock_gettime_us(clock_id), buf);
}

#endif

/* convert hex digit to integer */
int hex_aton(char a)
{
    if (a >= '0' && a <= '9')
        return a - '0';
    else if (a >= 'a' && a <= 'f')
        return 10 + a - 'a';
    else if (a >= 'A' && a <= 'F')
        return 10 + a - 'A';
    else
        return -1;
}


/* convert mac from string ("04:8d:38:7f:1e:20") to array of six bytes */
uint8_t* mac_aton(const char* mac_string, uint8_t* mac_bytes)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        uint8_t b = 0;
        int v = hex_aton(*mac_string);
        if (v == -1)
            return NULL;
        b = v * 16;
        mac_string++;
        v = hex_aton(*mac_string);
        if (v == -1)
            return NULL;
        b += v;
        mac_string++;
        if (i < 5 && *mac_string == '\0')
            return NULL;
        mac_string++;
        mac_bytes[i] = b;
    }

    return mac_bytes;

} /* mac_aton */


/* convert array of six bytes to mac as string (e.g. "04:8d:38:7f:1e:20") */
char* mac_ntoa(uint8_t* mac_bytes, char* mac_string)
{

    sprintf(mac_string, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_bytes[0], mac_bytes[1], mac_bytes[2], mac_bytes[3], mac_bytes[4], mac_bytes[5]);

    return mac_string;

} /* mac_ntoa */
