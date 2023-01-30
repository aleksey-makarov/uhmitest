# uhmitest

> A library and a test application for Unified HMI

## libuhmigl

### GLES contexts

`libuhmigl` is an Android library that allows to create applications that
use GLES to draw on native or UHMI surfaces.  It uses `glad` library to
keep current pointers to EGL/GLES symbols.

Clients should include `linuhmigl.h` header.  It redefines all the EGL/GLES symbols.
To initialize drawing on UHMI function

    int  libuhmigl_init(uint16_t *h, uint16_t *v);

should be called.  It returns the size of the screen.  To free the resources, call

    libuhmigl_done(void);

Between the calls to these functions all GLES symbols are available to the client.
Function

    libuhmigl_update(void);

should be used to flip buffer.

Functions with suffixes `_android_` are used to draw on the native surface:

    int  libuhmigl_android_init(struct ANativeWindow *window, uint16_t *h, uint16_t *v);
    void libuhmigl_android_done(void);
    int  libuhmigl_android_update(void);

Client can have initialized both native and UHMI context simultaniously.
To switch between the available symbols use functions

    int  libuhmigl_load(void);
    int  libuhmigl_android_load(void);

that load UHMI or native GLES symbols.

See directory `app` for an example.

### Restarting `rproxy`

`libuhmigl` provides functions to set arguments of `rproxy` daemon and to restart it:

    int libuhmigl_set_scanout(const char *scanout);
    int libuhmigl_set_remote_address(const char *remote_address);

The first function sets the scanout parameter, the second one sets the IP address
of the remote `rvdds` server and the port to connect to and restarts `rproxy`.

### Logging functions

In a separate header file `pr.h` there are helper functions to use log facilities
uniformly on Android and Linux:

    void pr_info( format, ... );
    void pr_err( format, ... );

As these functions are implemented as macroses, the argument `format` should be
an explicit string literal.  On Linux, they print to `stderr`, on Android -- to
the standard log facility.  By redefining the weak symbol `pr_use_stderr` and/or setting
it to `1` Android client can print to `stderr`.  By redefining the weak symbol
`pr_disable_info` and/or setting it to `1` client can switch off printing info messages.

## es2gears

The directory `es2gears` contains the code from `glgears` ported to GLES:

    https://gitlab.freedesktop.org/mesa/demos/-/blob/main/src/egl/opengles2/es2gears.c

## uhmitest

The directory `uhmitest` contains the sources for Android command line utility
that uses `libuhmigl` to draw on UHMI device.  Also, when called with a command line parameter it sets up the `rproxy` address to connect to and restarts it.

## app

The directory `app` contains an android application that draws `glgears` on native and UHMI contexts.  Right after being started it uses the native context, after receiving
a tap event it switches to UHMI and so on.
