#include "connect.h"
#include "protocol.h"
#include "exec_safe.h"
#include "util.h"   // позже
#include <string.h>

// system log output 
#include <syslog.h>

static const char *get_arg(struct request *req, const char *key) {
    for (int i = 0; i < req->argc; i++) {
        if (strcmp(req->args[i].key, key) == 0)
            return req->args[i].val;
    }
    return NULL;
}


void handle_connect(int fd, struct request *req) {
    const char *iface = get_arg(req, "iface");
    const char *ssid  = get_arg(req, "ssid");
    const char *psk   = get_arg(req, "psk");

    if (!iface) {
        send_error(fd, "EINVAL", "missing_iface");
        return;
    }

    if (!is_valid_iface(iface)) {
        send_error(fd, "EINVAL", "bad_iface");
        return;
    }

    // --- Wi-Fi path (ssid required) ---
    if (ssid) {
        if (!is_valid_ssid(ssid)) {
            send_error(fd, "EINVAL", "bad_ssid");
            return;
        }

        if (!psk) {
            send_error(fd, "EINVAL", "missing_psk");
            return;
        }

        // 1. ifconfig iface up
        char *if_up[] = { "ifconfig", (char *)iface, "up", NULL };
        if (run_exec("/sbin/ifconfig", if_up) != 0) {
            send_error(fd, "EIO", "ifconfig_up_failed");
            return;
        }

        // 2. wpa_supplicant -B -i iface -c /etc/wpa_supplicant.conf
        char *wpa[] = {
            "wpa_supplicant",
            "-B",
            "-i", (char *)iface,
            "-c", "/etc/wpa_supplicant.conf",
            NULL
        };

        if (run_exec("/usr/sbin/wpa_supplicant", wpa) != 0) {
            send_error(fd, "EIO", "wpa_failed");
            return;
        }

        // 3. dhclient iface
        char *dhcp[] = { "dhclient", (char *)iface, NULL };
        if (run_exec("/sbin/dhclient", dhcp) != 0) {
            send_error(fd, "EIO", "dhcp_failed");
            return;
        }

        if (send_ok(fd) < 0) {
            syslog(LOG_WARNING, "write failed");
        }
        return;
    }

    // --- Ethernet (DHCP) ---
    char *if_up[] = { "ifconfig", (char *)iface, "up", NULL };
    if (run_exec("/sbin/ifconfig", if_up) != 0) {
        send_error(fd, "EIO", "ifconfig_up_failed");
        return;
    }

    char *dhcp[] = { "dhclient", (char *)iface, NULL };
    if (run_exec("/sbin/dhclient", dhcp) != 0) {
        send_error(fd, "EIO", "dhcp_failed");
        return;
    }

    if (send_ok(fd) < 0) {
        syslog(LOG_WARNING, "write failed");
    }
}

