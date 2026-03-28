#include "policy.h"
#include "auth.h"
#include <string.h>

// --- function type ---
typedef int (*policy_fn)(struct xucred *, struct request *);

// --- rules ---

// allow all
static int allow_all(struct xucred *cred, struct request *req) {
    (void)cred;
    (void)req;
    return 1;
}

// only group netagent
static int allow_netagent(struct xucred *cred, struct request *req) {
    (void)req;

    gid_t gid;
    if (get_netagent_gid(&gid) != 0)
        return 0;

    return cred_in_group(cred, gid);
}

static int allow_root(struct xucred *cred, struct request *req) {
    (void)req;
    return cred->cr_uid == 0;
}

static int allow_root_or_group(struct xucred *cred, struct request *req) {
    if (cred->cr_uid == 0)
        return 1;

    gid_t gid;
    if (get_netagent_gid(&gid) != 0)
        return 0;

    return cred_in_group(cred, gid);
}

struct policy_rule {
    const char *cmd;
    policy_fn check;
};

static struct policy_rule rules[] = {
    { "STATUS", allow_all },
    { "LIST_INTERFACES", allow_all },
    { "CONNECT", allow_netagent },

    // конец таблицы
    { NULL, NULL }
};

int authorize(struct xucred *cred, struct request *req) {
    if (!req->cmd)
        return 0;

    for (int i = 0; rules[i].cmd != NULL; i++) {
        if (strcmp(req->cmd, rules[i].cmd) == 0) {
            return rules[i].check(cred, req);
        }
    }

    // неизвестная команда → запрещаем
    return 0;
}
