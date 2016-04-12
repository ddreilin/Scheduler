/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _job_t
{
  //job characteristics
  int core, number, priority, arrival_time, running_time;
  int started;
  int remaining_time;
  int first_call;//I was overriding arrival_time, so this is the first time it was called
                 //arrival time represents the time it arrives at the core

  //time statistics
  int waiting_time, turnaround_time, response_time;

} job_t;

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _scheduler_t
{
  int cores;
  int* idle;
  scheme_t scheme;
  priqueue_t q;
  int jobs;

  //running totals of time
  int waiting, turnaround, response;

} scheduler_t;

//scheme compares

//compare for First Come First Serve changed from arrival time to first call
int fcfs_compare(const void * a, const void * b)
{
	return ( ((job_t*)a)->first_call - ((job_t*)b)->first_call );
}

//compare for Shortest Job First
int sjf_compare(const void * a, const void * b)
{
	return ( ((job_t*)a)->running_time - ((job_t*)b)->running_time );
}

//compare for Pre-emptive Shortest Job First
int psjf_compare(const void * a, const void * b)
{
	return ( ((job_t*)a)->remaining_time - ((job_t*)b)->remaining_time );
}

int pri_compare(const void * a, const void * b)
{
	return ( ((job_t*)a)->priority - ((job_t*)b)->priority );
}

int ppri_compare(const void * a, const void * b)
{
  if( ( ((job_t*)a)->priority - ((job_t*)b)->priority ) == 0){
	  return ( ((job_t*)a)->first_call - ((job_t*)b)->first_call );
  }
  else{
    return ( ((job_t*)a)->priority - ((job_t*)b)->priority );
  }
}


//global scheduler variable
scheduler_t *s;

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
    s = malloc(sizeof(scheduler_t));
    //initialize number of cores in scheduler
    s->cores = cores;
    //Tell the scheduler which shceme we are using
    s->scheme = scheme;

    //initialize jobs
    s->jobs = 0;

    //initize idle core array
    s->idle = malloc(cores * sizeof(int));
    for(int i = 0; i < cores; i++){
      s->idle[i] = 1;
    }

    //set times to 0
    s->waiting = 0;
    s->turnaround = 0;
    s->response = 0;

    //initialize the queue based on scheme
    if(s->scheme == FCFS){
      priqueue_init( &s->q, fcfs_compare);
    }
    else if(s->scheme == SJF){
      priqueue_init( &s->q, sjf_compare);
    }
    else if(s->scheme == PSJF){
      priqueue_init( &s->q, psjf_compare);
    }
    else if(s->scheme == PRI){
      priqueue_init( &s->q, pri_compare);
    }
    else if(s->scheme == PPRI){
      priqueue_init( &s->q, ppri_compare);
    }

}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  //make a new job node with time, running time, priority
  struct _job_t *new_job = malloc(sizeof(job_t));
  new_job->number = job_number;
  new_job->arrival_time = time;
  new_job->first_call = time;
  new_job->running_time = running_time;
  new_job->remaining_time = running_time;
  new_job->priority = priority;
  new_job->waiting_time = 0;
  new_job->turnaround_time = 0;
  new_job->response_time = 0;
  new_job->core = -1;
  new_job->started = 0;

  //sees if there is an idle core
  for(int i = 0; i < s->cores; i++){
    if(s->idle[i] == 1){
      new_job->core = i;
      s->idle[i] = 0;
      break;
    }
  }

  //starts running immediately if idling core
  if(new_job->core != -1){
    new_job->started = 1;
  }

  //else if PSJF
  else if(s->scheme == PSJF){
    int index;
    int remaining = 0;

    //find the longest currently running remaining time
    for(int i = 0; i < priqueue_size(&s->q); i++){
      //if currently running
      if(((job_t *)priqueue_at(&s->q, i))->core != -1){
        //if the current time remaning is newest remaining
        if( (((job_t *)priqueue_at(&s->q, i))->running_time -  (time - ((job_t *)priqueue_at(&s->q, i))->started ) )> remaining){
          remaining = ((job_t *)priqueue_at(&s->q, i))->running_time -  (time - ((job_t *)priqueue_at(&s->q, i))->started);
          index = i;
        }
      }
    }

    //if new job will finish sooner
    if( new_job->remaining_time < remaining ){
        struct _job_t *prev_job = (job_t *)priqueue_remove_at(&s->q, index);
        new_job->core = prev_job->core;
        new_job->started = 1;

        prev_job->remaining_time = remaining;
        prev_job->core = -1;

        //put prev_job back in queue
        priqueue_offer(&s->q, prev_job);

    }

  }

  else if(s->scheme == PPRI){
    int index;
    int lowest_priority = -1;
    for(int i = 0; i < priqueue_size(&s->q); i++){
      //if currently running
      if(((job_t *)priqueue_at(&s->q, i))->core != -1){
        //if the current time remaning is newest remaining
        if( ((job_t *)priqueue_at(&s->q, i))->priority > lowest_priority){
          lowest_priority = ((job_t *)priqueue_at(&s->q, i))->priority;
          index = i;
        }
      }
    }

    if( new_job->priority < lowest_priority){
      new_job->core = ((job_t *)priqueue_at(&s->q, index))->core;
      new_job->started = 1;

      ((job_t *)priqueue_at(&s->q, index))->core = -1;
      ((job_t *)priqueue_at(&s->q, index))->arrival_time = time;
    }
  }

  //put in queue
  priqueue_offer(&s->q, new_job);

  //increment number of jobs
  s->jobs++;

  return new_job->core;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
  int index = -1;
  job_t* old_job;

  //find the job in the queue and remove it
  for(int i = 0; i < priqueue_size(&s->q); i++){
    if( ((job_t *)priqueue_at(&s->q, i))->core == core_id){
      old_job = (job_t *)priqueue_remove_at(&s->q, i);
      break;
    }
  }

  //get the time statistics and delete old_job
  old_job->turnaround_time = time - old_job->arrival_time;
  s->waiting += old_job->waiting_time;
  s->turnaround += old_job->turnaround_time;
  s->response += old_job->response_time;
  free(old_job);

  //sanity
  scheduler_show_queue();

  //if there is a job waiting, start it
  for(int i = 0; i < priqueue_size(&s->q); i++){
    if(((job_t *)priqueue_at(&s->q, i))->core == -1){
      old_job = (job_t *)priqueue_remove_at(&s->q, i);
      old_job->core = core_id;
      old_job->waiting_time += (time - old_job->arrival_time);
      if(old_job->started == 0){
        old_job->response_time = (time - old_job->arrival_time );
        old_job->started == 1;
      }
      index = old_job->number;
      priqueue_offer(&s->q, old_job);
      return index;
    }
  }

  //otherwise set core to idle
  s->idle[core_id] = 1;
  return index;

}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{

  //divide by number of jobs

	return (float)s->waiting/s->jobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{

  //divide running total by jobs

	return (float)s->turnaround/s->jobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
  //divide running total by jobs

	return (float)s->response/s->jobs;
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  for(int i = 0; i < priqueue_size(&s->q); i++ ){
    printf("%d(%d) ", ((job_t *)priqueue_at(&s->q, i))->number, ((job_t *)priqueue_at(&s->q, i))->core);
  }
  printf("\n");
}
