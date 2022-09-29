#ifndef __KMT_H

#include <common.h>
#include <common/lock.h>

void kmt_context_save(Event ev, Context *ctx);
Context *kmt_context_schedule(Event ev,Context *ctx);
#endif 