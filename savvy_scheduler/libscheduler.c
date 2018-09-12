/**
* Savvy_scheduler Lab
* CS 241 - Spring 2018
*/

#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _job_info {
    int id;
    int priority;
    bool start;
    double runningTime;
    double lastStartTime;
  	double arivalTime;
  	double beenRunTime;
    double remainingTime;
    double startTime;
    double endTime;
    // Used for rr
    int turn;
    /* Add whatever other bookkeeping you need into this struct. */
} job_info;

priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;

int count;
double turnaround;
double response;
double waiting;



void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    count = 0;
    turnaround = 0;
    response = 0;
    waiting = 0;
    // Put any set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
  job_info* a_job  = (job_info*)(((job*)a)->metadata);
  job_info* b_job  = (job_info*)(((job*)b)->metadata);
  double a_arTime = a_job->arivalTime;
  double b_arTime = b_job->arivalTime;
  if (a_arTime < b_arTime) {
    return -1;
  }
  else if (a_arTime < b_arTime) {
    return 1;
  }
  else {
    if (a_job->id < b_job->id) {
      return -1;
    }
    else {
      return 1;
    }
  }
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
  int a_job  = ((job_info*)(((job*)a)->metadata))->priority;
  int b_job  = ((job_info*)(((job*)b)->metadata))->priority;
  if (a_job < b_job) {
    return -1;
  }
  else if (a_job > b_job) {
    return 1;
  }
  else {
    return break_tie(a, b);
  }
}

int comparer_psrtf(const void *a, const void *b) {
  double a_job  = ((job_info*)(((job*)a)->metadata))->remainingTime;
  double b_job  = ((job_info*)(((job*)b)->metadata))->remainingTime;
  if (a_job < b_job) {
    return -1;
  }
  else if (a_job > b_job) {
    return 1;
  }
  else {
    return break_tie(a, b);
  }
}

int comparer_rr(const void *a, const void *b) {
  int a_job  = ((job_info*)(((job*)a)->metadata))->turn;
  int b_job  = ((job_info*)(((job*)b)->metadata))->turn;
  if (a_job < b_job) {
    return -1;
  }
  else if (a_job > b_job) {
    return 1;
  }
  else {
    return break_tie(a, b);
  }
}

int comparer_sjf(const void *a, const void *b) {
  double a_job  = ((job_info*)(((job*)a)->metadata))->runningTime;
  double b_job  = ((job_info*)(((job*)b)->metadata))->runningTime;
  if (a_job < b_job) {
    return -1;
  }
  else if (a_job > b_job) {
    return 1;
  }
  else {
    return break_tie(a, b);
  }
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_countber, double time,
                       scheduler_info *sched_data) {
    // TODO complete me!
    count ++;
    job_info *temp = (job_info*)malloc(sizeof(job_info));
    temp->id = job_countber;
  	temp->priority = sched_data->priority;
  	temp->runningTime = sched_data->running_time;
    temp->remainingTime = sched_data->running_time;
  	temp->arivalTime = time;
  	temp->start = 0;
  	temp->beenRunTime = 0;
  	temp->lastStartTime = 0;
    temp->startTime = 0;
  	temp->endTime = 0;
    temp->turn = sched_data->priority;
  	newjob->metadata = ((void*)temp);
  	priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO complete me!
    if (!job_evicted) {
      job* curr = (job*)priqueue_poll(&pqueue);
      if (!curr){
        // All done
        return NULL;
      }
      job_info* curr_info = (job_info*)(curr-> metadata);
      if (!curr_info->start) {
        // Have not started yet
        curr_info->start = 1;
        curr_info->startTime = time;
        curr_info->lastStartTime = time;
      }
      else {
        curr_info->remainingTime -= (time - curr_info->lastStartTime);
        curr_info->lastStartTime = time;
      }
      return curr;
    }

    job_info* curr = (job_info*)(job_evicted-> metadata);
    if (!curr->start) {
      curr->start = 1;
      curr->startTime = time;
      curr->lastStartTime = time;
    }
    else {
      curr->remainingTime -= (time - curr->lastStartTime);
      curr->lastStartTime = time;
    }

    if (pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == SJF){
  		return job_evicted;
  	}
    else {
      curr->turn += count;
  		priqueue_offer(&pqueue, job_evicted);
      job *curr_job = (job*)priqueue_poll(&pqueue);
  		curr = (job_info*)(curr_job->metadata);
  		if (!curr->start) {
        curr->start = 1;
  			curr->startTime = time;
  			curr->lastStartTime = time;
  		}
  		else {
  			curr->lastStartTime = time;
  		}
  		return curr_job;
    }
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO complete me!
    job_info* curr = (job_info*)(job_done->metadata);
    turnaround += time - curr->arivalTime;
    response += curr->startTime - curr->arivalTime;
    waiting += time - curr->arivalTime - curr->runningTime;
    free(job_done->metadata);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO complete me!
    return waiting / count;
}

double scheduler_average_turnaround_time() {
    // TODO complete me!
    return turnaround / count;
}

double scheduler_average_response_time() {
    // TODO complete me!
    return response / count;
}

void scheduler_show_queue() {
    // Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}