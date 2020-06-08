#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "x86.h"


struct lock_t
{
  uint locked;
};

struct lock_t myLock;

void lock_init(struct lock_t *lk)
{
  lk->locked = 0;
}

void lock_acquire(struct lock_t *lk)
{
//  pushcli();

  while(xchg(&lk->locked, 1) != 0)
    ;

  __sync_synchronize();
}

void lock_release(struct lock_t *lk)
{
	__sync_synchronize();
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
//	popcli();
}

struct thread_arr {
	int index;
};

int frisbee_location = 0;
int pass = 0;
int max_passes;
int num_players


void*
frisbee(int thread_num)
{
  while( pass < max_passes )
  {
    lock_acquire(&myLock);
    if(frisbee_location == thread_num)
    {
      printf(1,"Pass numer no: %d, Thread %d is passing the token to thread %d\n", shared_counter, shared_counter+1%players);
      frisbee_location++;
      passes++;
      frisbee_location %= players;
    }
    lock_release(&myLock);
  }

  return (void*)0;

}

int
thread_create(void * (*start_routine)(void *), void * arg)
{
  // allocate one page for new thread's stack	
  void * stack = (void *)malloc(4096);
  
  // clone parent state
  int thread_id = clone(stack, 4096);
  
  // execute "start_routine()" on child with pointer to args
  if(thread_id == 0)
  {
    start_routine(arg);
    //free(stack);
  }
  return 0;
}

int
main(int argc, char **argv)
{
  players = atoi(argv[1]);
  passes = atoi(argv[2]);

  lock_init(&myLock);

  //struct thread_arr* args[threads];

  // create one thread for each player
  for(threads = 0; threads<players; threads++)
  {
    //int t thread_arr* arg = (struct thread_arr*)malloc(sizeof(struct thread_arr));
    //arg->index = threads;
    //args[threads] = arg;
    thread_create(frisbee, &threads);
  }

  for (threads = 0; threads < players; threads++)
  {
    ;
    //free(args[threads]);
  }
  printf(1, "Exit\n");
  exit();
} 
