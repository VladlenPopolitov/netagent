#include "auth.h"
#include <sys/socket.h>
#include <grp.h>
//#include <sys/types.h>
#include <sys/un.h>     
#include <sys/ucred.h>

int get_peer_cred(int fd, struct xucred *cred) {
    socklen_t len = sizeof(*cred);

    if (getsockopt(fd, SOL_LOCAL, LOCAL_PEERCRED, cred, &len) < 0) {
        return -1;
    }

    return 0;
}

int cred_in_group(struct xucred *cred, gid_t gid) {
    for (int i = 0; i < cred->cr_ngroups; i++) {
        if (cred->cr_groups[i] == gid)
            return 1;
    }
    return 0;
}

int get_netagent_gid(gid_t *gid) {
    static int initialized = 0;
    static gid_t cached_gid;

    if (!initialized) {
        struct group *grp = getgrnam("netagent");
        if (!grp) return -1;

        cached_gid = grp->gr_gid;
        initialized = 1;
    }

    *gid = cached_gid;
    return 0;
}
