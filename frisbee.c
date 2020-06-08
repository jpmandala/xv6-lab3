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


void lock_init(struct lock_t *lk)
{
  lk->locked = 0;
}

void lock_acquire(struct lock_t *lk)
{
//  pushcli();
  // atomically sets and checks if lock == 1
  // if it was 0, xchg will return 0 and simultaenously set to 1
  while(xchg(&lk->locked, 1) != 0)
    ;
  
  // memory fence to prevent reordering
  __sync_synchronize();
}

void lock_release(struct lock_t *lk)
{
  // memory fence
  __sync_synchronize();

  // clear the lock using asm, movl will update all 4 bytes at once
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );
//	popcli();
}


// globals
struct lock_t myLock;
int frisbee_location = 0;
int pass = 0;
int max_passes;
int num_players;


void*
frisbee(void *thread_idx)
{
  // loop thread until all passes complete
  while( pass < max_passes )
  { 
    lock_acquire(&myLock);

    //printf(1,"pass: %d\n", pass);
    
    // check if thread's turn to pass token 
    if(frisbee_location == *((int*)thread_idx) )
    {
      printf(1,"Thread index: %d\n", *(int*)thread_idx);	    
      printf(1,"Pass numer no: %d, Thread %d is passing the token to thread %d\n", pass+1, 
		      frisbee_location, (frisbee_location+1) % num_players);
      frisbee_location = (frisbee_location + 1) % num_players;
      pass++;
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
  
  // execute "start_routine()" only on child threads
  if(thread_id == 0)
  {
    start_routine(arg);
  }
  return 0;
}

int
main(int argc, char **argv)
{
  int i;  
  num_players = atoi(argv[1]);
  max_passes = atoi(argv[2]);

  // create an array to store thread numbers
  // will be used for parameters of frisbee
  int* thread_nums = (int*)malloc(num_players*sizeof(int));
  
  lock_init(&myLock);

  // create one thread for each player
  for(i = 0; i<num_players; i++)
  {
    thread_nums[i] = i;  // assign thread idx
    // create thread to run frisbee function with thread idx as param
    thread_create(frisbee, &thread_nums[i]); 

  }
  exit();
} 
