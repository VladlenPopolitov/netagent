#pragma once

#include <sys/ucred.h>
#include "request.h"   // struct request

// returns 1 if allowed, 0 if not allowed
int authorize(struct xucred *cred, struct request *req);
