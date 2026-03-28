#pragma once

#include <sys/types.h>
#include <sys/ucred.h>

// получить credentials клиента
int get_peer_cred(int fd, struct xucred *cred);

// проверка: пользователь в группе
int cred_in_group(struct xucred *cred, gid_t gid);

// получить gid группы netagent (кэшируется)
int get_netagent_gid(gid_t *gid);
