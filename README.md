# Watch Dog Timer

This project provides a watch dog mechanism that can monitor a client process and revive it if it stops responding.
The library exposes two main functions:
- `MMI()` – start monitoring the process.
- `DNR()` – notify the watch dog to stop.

## Prerequisites

The code depends on a Data Structures library that supplies several utilities, including `scheduler_heap.h` and `uid.h`.
The Makefile expects that library under `/home/ido/git/ds`.  Adjust the following variables if your copy resides elsewhere:

```
SRC_DIR    := /home/ido/git/ds/src
INCLUDE_DIR:= /home/ido/git/ds/include
```

## Building

Run `make` from the repository root.  This builds the shared library `libwd.so`, the watch dog process `wd_process`, and an example client `wd_client`.

```
make
```

`make` invokes the following targets:

```
all:
$(MAKE) wd_shared
$(MAKE) wd_process
$(MAKE) wd
```

Refer to the Makefile for additional compilation flags.

## Example Usage

The sample client `wd_client` (built from `client_test.c`) demonstrates how to start and stop the watch dog.  It calls `MMI()` and later invokes `DNR()` before exiting:

```
int main(int argc, char *argv[])
{
    ...
    MMI(2, 3, argv);
    ...
    DNR();
}
```

Run the example after building:

```
./wd_client arg1 arg2
```

`wd_client` will start a watch dog process in the background and continue running.  After roughly 30 seconds the program calls `DNR()` which terminates the watch dog and exits cleanly.

