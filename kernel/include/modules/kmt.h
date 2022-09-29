#ifndef __KMT_H

#include <os.h>

__always_inline void kmt_context_save(Event ev, Context *ctx);
__always_inline Context *kmt_context_switch(Event ev,Context *ctx);
#endif 