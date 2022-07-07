µ #ifndef __ECLIB_QUEUE_H
#define __ECLIB_QUEUE_H

#include <core.h>
#include <vector.h>

#define define_queue_type(name)
	struct queue_s_##name
{
	vec_queue##name *vec;
};
typedef struct queue_s_##name *queue_##name;

#define _define_queue_create(type, name)

#endif /* __ECLIB_QUEUE_H */