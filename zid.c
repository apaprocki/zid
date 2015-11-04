// zid - zoneinfo dump
//
// zid  reads  binary  zoneinfo data from the file named on the
// command line and outputs the raw values contained within the
// data  to stdout . The filename must have been compiled using
// the zic command and be in the correct tzfile format.

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
# error "C99 compiler required"
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
# define zid_assert(x, y) _Static_assert(x, y)
#else
# define zid_assert(x, y) assert(x && y)
#endif

#if defined(__linux__)
# define _BSD_SOURCE
# include <endian.h>
# if __GLIBC_PREREQ(2, 9)
#  define ntohll(x) be64toh(x)
# else
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ntohll(x) (((uint64_t)ntohl(x)) << 32 | ntohl(x >> 32))
#  else
#   define ntohll(x) x
#  endif
# endif
#elif defined(__FreeBSD__)
# include <sys/endian.h>
# define ntohll(x) be64toh(x)
#elif defined(__OpenBSD__)
# include <sys/types.h>
# define ntohll(x) betoh64(x)
#elif defined(_AIX)
# ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 600
# endif
# ifndef _ALL_SOURCE
#  define _ALL_SOURCE
# endif
#endif

#include <alloca.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netinet/in.h>
#include <sys/types.h>

#define TZ_MAGIC "TZif"

struct tzhead {
    char tzh_magic[4];
    char tzh_version[1];
    char tzh_reserved[15];
    char tzh_ttisgmtcnt[4];
    char tzh_ttisstdcnt[4];
    char tzh_leapcnt[4];
    char tzh_timecnt[4];
    char tzh_typecnt[4];
    char tzh_charcnt[4];
};

int read32(FILE *fp, int *version)
{
    struct tzhead tzh;
    if (fread(&tzh, sizeof(tzh), 1, fp) < 1) {
        perror("zid: 32: header");
        return -1;
    }
    if (0 != memcmp(tzh.tzh_magic, TZ_MAGIC, 4)) {
        fprintf(stderr, "zid: 32: incorrect TZ_MAGIC, not a zoneinfo file\n");
        return -1;
    }
    printf("32: tzh: magic '%c%c%c%c'\n",
           tzh.tzh_magic[0], tzh.tzh_magic[1],
           tzh.tzh_magic[2], tzh.tzh_magic[3]);
    *version = tzh.tzh_version[0] - 0x30;
    printf("32: tzh: version %d\n", *version);

    uint32_t ttisgmtcnt = ntohl(*(uint32_t *)tzh.tzh_ttisgmtcnt);
    printf("32: tzh: ttisgmtcnt %u\n", ttisgmtcnt);
    uint32_t ttisstdcnt = ntohl(*(uint32_t *)tzh.tzh_ttisstdcnt);
    printf("32: tzh: ttisstdcnt %u\n", ttisstdcnt);
    uint32_t leapcnt = ntohl(*(uint32_t *)tzh.tzh_leapcnt);
    printf("32: tzh: leapcnt %u\n", leapcnt);
    uint32_t timecnt = ntohl(*(uint32_t *)tzh.tzh_timecnt);
    printf("32: tzh: timecnt %u\n", timecnt);
    uint32_t typecnt = ntohl(*(uint32_t *)tzh.tzh_typecnt);
    printf("32: tzh: typecnt %u\n", typecnt);
    uint32_t charcnt = ntohl(*(uint32_t *)tzh.tzh_charcnt);
    printf("32: tzh: charcnt %u\n", charcnt);

    int32_t *ttimes = (int32_t *)alloca(4 * timecnt);
    if (fread(ttimes, sizeof(int32_t), timecnt, fp) < timecnt) {
        perror("zid: 32: ttimes");
        return -1;
    }
    uint8_t *ttypes = (uint8_t *)alloca(4 * timecnt);
    if (fread(ttypes, sizeof(uint8_t), timecnt, fp) < timecnt) {
        perror("zid: 32: ttypes");
        return -1;
    }
    for (uint32_t i = 0; i < timecnt; ++i) {
        int32_t ttime = ntohl(ttimes[i]);
        printf("32: ttimes[%02d]: %d %u ", i, ttime, ttypes[i]);
        time_t t = (time_t)ttime;
        const struct tm *gmt = gmtime(&t);
        printf("%04d-%02d-%02d %02d:%02d:%02d\n",
               gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
               gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
    }

#pragma pack(1)
    typedef struct {
        int32_t gmtoff;
        uint8_t isdst;
        uint8_t abbrind;
    } offsets_t;
#pragma pack()
    zid_assert(sizeof(offsets_t) == 6, "offsets_t is the wrong size");

    offsets_t *offsets = (offsets_t *)alloca(sizeof(offsets_t) * typecnt);
    if (fread(offsets, sizeof(offsets_t), typecnt, fp) < typecnt) {
        perror("zid: 32: offsets");
        return -1;
    }

    char *abbrevs = (char *)alloca(charcnt);
    if (fread(abbrevs, sizeof(char), charcnt, fp) < charcnt) {
        perror("zid: 32: abbrevs");
        return -1;
    }

    for (uint32_t i = 0; i < typecnt; ++i) {
        printf("32: gmtoff %d isdst %d abbrind %d (%s)\n",
               ntohl(offsets[i].gmtoff),
               offsets[i].isdst, offsets[i].abbrind,
               &abbrevs[offsets[i].abbrind]);
    }

#pragma pack(1)
    typedef struct {
        int32_t leap_transition;
        int32_t total_correction;
    } leap_t;
#pragma pack()
    zid_assert(sizeof(leap_t) == 8, "leap_t is the wrong size");

    leap_t *leaps = (leap_t *)alloca(sizeof(leap_t) * leapcnt);
    if (fread(leaps, sizeof(leap_t), leapcnt, fp) < leapcnt) {
        perror("zid: 32: leaps");
        return -1;
    }

    for (uint32_t i = 0; i < leapcnt; ++i) {
        printf("32: leap transition %d correction %d\n",
               ntohl(leaps[i].leap_transition),
               ntohl(leaps[i].total_correction));
    }

    uint8_t *ttisstd = (uint8_t *)alloca(ttisstdcnt);
    if (fread(ttisstd, ttisstdcnt, 1, fp) < 1) {
        perror("zid: 32: ttisstd");
        return -1;
    }
    for (uint32_t i = 0; i < ttisstdcnt; ++i) {
        printf("32: ttisstdcnt[%d]: %u\n", i, ttisstd[i]);
    }

    uint8_t *ttisgmt = (uint8_t *)alloca(ttisgmtcnt);
    if (fread(ttisgmt, ttisgmtcnt, 1, fp) < 1) {
        perror("zid: 32: ttisgmt");
        return -1;
    }
    for (uint32_t i = 0; i < ttisgmtcnt; ++i) {
        printf("32: ttisgmtcnt[%d]: %u\n", i, ttisgmt[i]);
    }

    return 0;
}

int read64(FILE *fp)
{
    struct tzhead tzh;
    if (fread(&tzh, sizeof(tzh), 1, fp) < 1) {
        perror("zid: 64: header");
        return -1;
    }
    if (0 != memcmp(tzh.tzh_magic, TZ_MAGIC, 4)) {
        fprintf(stderr, "zid: 64: incorrect TZ_MAGIC, not a zoneinfo file\n");
        return -1;
    }
    printf("64: tzh: magic '%c%c%c%c'\n",
           tzh.tzh_magic[0], tzh.tzh_magic[1],
           tzh.tzh_magic[2], tzh.tzh_magic[3]);
    printf("64: tzh: version %d\n", tzh.tzh_version[0] - 0x30);

    uint32_t ttisgmtcnt = ntohl(*(uint32_t *)tzh.tzh_ttisgmtcnt);
    printf("64: tzh: ttisgmtcnt %u\n", ttisgmtcnt);
    uint32_t ttisstdcnt = ntohl(*(uint32_t *)tzh.tzh_ttisstdcnt);
    printf("64: tzh: ttisstdcnt %u\n", ttisstdcnt);
    uint32_t leapcnt = ntohl(*(uint32_t *)tzh.tzh_leapcnt);
    printf("64: tzh: leapcnt %u\n", leapcnt);
    uint32_t timecnt = ntohl(*(uint32_t *)tzh.tzh_timecnt);
    printf("64: tzh: timecnt %u\n", timecnt);
    uint32_t typecnt = ntohl(*(uint32_t *)tzh.tzh_typecnt);
    printf("64: tzh: typecnt %u\n", typecnt);
    uint32_t charcnt = ntohl(*(uint32_t *)tzh.tzh_charcnt);
    printf("64: tzh: charcnt %u\n", charcnt);

    int64_t *ttimes = (int64_t *)alloca(sizeof(int64_t) * timecnt);
    if (fread(ttimes, sizeof(int64_t), timecnt, fp) < timecnt) {
        perror("zid: 64: ttimes");
        return -1;
    }
    uint8_t *ttypes = (uint8_t *)alloca(sizeof(uint8_t) * timecnt);
    if (fread(ttypes, sizeof(uint8_t), timecnt, fp) < timecnt) {
        perror("zid: 64: ttypes");
        return -1;
    }
    for (uint32_t i = 0; i < timecnt; ++i) {
        int64_t ttime = ntohll(ttimes[i]);
        printf("64: ttimes[%02d]: %lld %u ", i, ttime, ttypes[i]);
        if (ttime >= INT_MIN && ttime <= INT_MAX) {
            time_t t = (time_t)ttime;
            const struct tm *gmt = gmtime(&t);
            printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                   gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
                   gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
        } else {
            printf("\n");
        }
    }

#pragma pack(1)
    typedef struct {
        int32_t gmtoff;
        uint8_t isdst;
        uint8_t abbrind;
    } offsets_t;
#pragma pack()
    zid_assert(sizeof(offsets_t) == 6, "offsets_t is the wrong size");

    offsets_t *offsets = (offsets_t *)alloca(sizeof(offsets_t) * typecnt);
    if (fread(offsets, sizeof(offsets_t), typecnt, fp) < typecnt) {
        perror("zid: 64: offsets");
        return -1;
    }

    char *abbrevs = (char *)alloca(charcnt);
    if (fread(abbrevs, sizeof(char), charcnt, fp) < charcnt) {
        perror("zid: 64: abbrevs");
        return -1;
    }

    for (uint32_t i = 0; i < typecnt; ++i) {
        printf("64: offset %d isdst %d abbrind %d (%s)\n",
               ntohl(offsets[i].gmtoff),
               offsets[i].isdst, offsets[i].abbrind,
               &abbrevs[offsets[i].abbrind]);
    }

#pragma pack(1)
    typedef struct {
        int64_t leap_transition;
        int64_t total_correction;
    } leap_t;
#pragma pack()
    zid_assert(sizeof(leap_t) == 16, "leap_t is the wrong size");

    leap_t *leaps = (leap_t *)alloca(sizeof(leap_t) * leapcnt);
    if (fread(leaps, sizeof(leap_t), leapcnt, fp) < leapcnt) {
        perror("zid: 64: leaps");
        return -1;
    }

    for (uint32_t i = 0; i < leapcnt; ++i) {
        printf("64: leap transition %lld correction %lld\n",
               ntohll(leaps[i].leap_transition),
               ntohll(leaps[i].total_correction));
    }

    uint8_t *ttisstd = (uint8_t *)alloca(ttisstdcnt);
    if (fread(ttisstd, ttisstdcnt, 1, fp) < 1) {
        perror("zid: 64: ttisstd");
        return -1;
    }
    for (uint32_t i = 0; i < ttisstdcnt; ++i) {
        printf("64: ttisstdcnt[%d]: %u\n", i, ttisstd[i]);
    }

    uint8_t *ttisgmt = (uint8_t *)alloca(ttisgmtcnt);
    if (fread(ttisgmt, ttisgmtcnt, 1, fp) < 1) {
        perror("zid: 64: ttisgmt");
        return -1;
    }
    for (uint32_t i = 0; i < ttisgmtcnt; ++i) {
        printf("64: ttisgmtcnt[%d]: %u\n", i, ttisgmt[i]);
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (2 != argc) {
        fprintf(stderr, "usage: zid <zonefile>\n");
        exit(-1);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("zid: Opening zoneinfo file");
        exit(-1);
    }

    // All files contain a 32-bit section.
    int version;
    if (0 != read32(fp, &version)) {
        exit(-2);
    }

    // `zic` compiled files >= version 2 contain a 64-bit section, followed
    // by a Posix(-like) TZ environment string for encoding far-future times.
    if (version >= 2) {
        if (0 != read64(fp)) {
            exit(-3);
        }

        // The TZ string is encoded between two newline characters.  If there
        // are two newline characters with nothing inbetween, no string is
        // specified inside the file.  Version 3 simply allows a wider range
        // of string encoded values and does not change the binary file format.
        char posix_tz[128];
        if (fgets(posix_tz, sizeof(posix_tz), fp) &&
            fgets(posix_tz, sizeof(posix_tz), fp) &&
            ('\n' != posix_tz[0] || '\r' != posix_tz[0])) {

            // Truncate the string at the trailing newline.
            for (size_t i = 0; i < sizeof(posix_tz); ++i) {
                if ('\n' == posix_tz[i] || '\r' == posix_tz[i]) {
                    posix_tz[i] = '\0';
                    break;
                }
            }
            printf("TZ: '%s'\n", posix_tz);
        }
    }

    if (fclose(fp)) {
        perror("zid: Closing zoneinfo file");
        exit(-1);
    }
    return 0;
}

//
// `zid` by Andrew Paprocki
//
// To the extent possible under law, the person who associated CC0 with
// `zid` has waived all copyright and related or neighboring rights to
// `zid`.
//
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0>.
//
