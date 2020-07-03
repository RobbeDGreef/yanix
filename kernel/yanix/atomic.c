#include <kernel/atomic.h>

void spinlock_lock(struct spinlock *sl)
{
	while (!atomic_compare_exchange(&sl->locked, SPINLOCK_UNLOCK, SPINLOCK_LOCK));
}

void spinlock_unlock(struct spinlock *sl)
{
	atomic_store(&sl->locked, SPINLOCK_UNLOCK);
}