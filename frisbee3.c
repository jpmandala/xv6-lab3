#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "x86.h"

int passes = 0;
int players = 0;
int threads = 0;
int current_pass = 0;
int chance = 0;


struct lock_t
{
	uint chance;
}x86_lock;

void lock_init(struct lock_t *lock)
{
	lock->chance = 0;
}

void lock_acquire(struct lock_t *lock, int current_thread)
{
	while(xchg(&lock->chance, lock->chance) != current_thread);
}

void lock_release(struct lock_t *lock)
{
	xchg(&lock->chance, (lock->chance+1)%players);
}

struct thread_arr {
	int index;
};

void*
frisbee(void *arg)
{
	int current_thread = ((struct thread_arr*)arg)->index;
	while(current_pass+(players-1) < passes)
	{
		lock_acquire(&x86_lock, current_thread);
		printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", current_pass, current_thread, (current_thread + 1)%players);
		current_pass++;
		lock_release(&x86_lock);
	}
	return arg;
}

int
thread_create(void * (*start_routine)(void *), void * arg)
{
	void * stack = (void *)malloc(4096);
	int thread_id = clone(stack, 4096);
	if(thread_id == 0)
	{
		start_routine(arg);
		free(stack);
	}
	return 0;
}

int
main(int argc, char **argv)
{
	players = atoi(argv[1]);
	passes = atoi(argv[2]);

	lock_init(&x86_lock);

	struct thread_arr* args[threads];

	for(threads = 0; threads<players; threads++)
	{
		struct thread_arr* arg = (struct thread_arr*)malloc(sizeof(struct thread_arr));
		arg->index = threads;
		args[threads] = arg;
		thread_create(frisbee, (void*)arg);
	}

	for (threads = 0; threads < players; threads++)
	{
		free(args[threads]);
	}
    printf(1, "Exit\n");
	exit();
} 
