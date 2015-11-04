Zoneinfo Dump
===

`zid` reads binary zoneinfo data from the file named on the
command line and outputs raw values contained within the
data to `stdout`.  The filename must have been compiled using
the `zic` command and be in the correct `tzfile` format.

### To build:

Prerequisites:

    * GCC or other C99 compiler

UNIX (GCC):

    make

UNIX (Other):

    make CC=c99 CFLAGS= LDFLAGS=

Solaris (Studio):

    make native

AIX (VisualAge C):

    make native


### Example output:

```
$ ./zid /usr/share/zoneinfo/Etc/GMT
32: tzh: magic 'TZif'
32: tzh: version 2
32: tzh: ttisgmtcnt 1
32: tzh: ttisstdcnt 1
32: tzh: leapcnt 0
32: tzh: timecnt 0
32: tzh: typecnt 1
32: tzh: charcnt 4
32: gmtoff 0 isdst 0 abbrind 0 (GMT)
32: ttisstdcnt[0]: 0
32: ttisgmtcnt[0]: 0
64: tzh: magic 'TZif'
64: tzh: version 2
64: tzh: ttisgmtcnt 1
64: tzh: ttisstdcnt 1
64: tzh: leapcnt 0
64: tzh: timecnt 0
64: tzh: typecnt 1
64: tzh: charcnt 4
64: offset 0 isdst 0 abbrind 0 (GMT)
64: ttisstdcnt[0]: 0
64: ttisgmtcnt[0]: 0
TZ: 'GMT0'
```

### License

`zid` by Andrew Paprocki

To the extent possible under law, the person who associated CC0 with
`zid` has waived all copyright and related or neighboring rights to
`zid`.

You should have received a copy of the CC0 legalcode along with this work.
If not, see [Creative Commons CC0 1.0 Universal License](http://creativecommons.org/publicdomain/zero/1.0).
