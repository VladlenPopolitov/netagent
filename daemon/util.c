#include <ctype.h>
#include "util.h"

// allow only safe interface names: wlan0, em0, etc.
int is_valid_iface(const char *s) {
    if (!s || !*s) return 0;

    for (; *s; s++) {
        if (!isalnum((unsigned char)*s))
            return 0;
    }
    return 1;
}

// minimal SSID validation (no quotes/newlines)
int is_valid_ssid(const char *s) {
    if (!s || !*s) return 0;

    for (; *s; s++) {
        if (*s == '"' || *s == '\'' || *s == '\n')
            return 0;
    }
    return 1;
}
