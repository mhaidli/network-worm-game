#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"
#include "util.h"

/// The type of a task function
typedef void (*job_t)();

//Variable to see if program should run
int stopped = 1;

typedef struct node
{
  job_t job;
  size_t interval;
  struct node * next;
  size_t lastruntime;
} JobListNode; /* note semi-colon here */

typedef struct {
  JobListNode * head;
  JobListNode * tail;
  JobListNode * currentJob;
  JobListNode * prevNode;
  int count;
} JobList;

//Initialize Queue
JobList myList = {NULL, NULL, NULL, NULL, 0};

// Add a periodic job to the job list. The function `job_fn` should run every `interval` ms


int myEmpty(){
  if(myList.head == NULL)
    return 1;
  else
    return 0;
}

void add_job(job_t task_fn, size_t interval){
  //Create node
  myList.count++;
  JobListNode * newNode = (JobListNode*) malloc(sizeof(JobListNode));
  newNode->job = task_fn;
  newNode->interval = interval;
  newNode->next = NULL;
  newNode->lastruntime = -1;
  if(myEmpty() == 1) {
    
    myList.head = newNode;
    myList.tail = newNode;

  }
  else {
    //Place node in list
    myList.tail->next = newNode;
    myList.tail = newNode;
  }
}

//Inspired by Julia's and Khoa's implementation of swap
void swap(JobListNode* a, JobListNode* b) {
  size_t temp = a->interval;
  job_t temp2 = a->job;

  a->interval = b->interval;
  a->job = b->job;
  b->interval = temp;
  b->job = temp2;
}

void bubblesort() {
  if (myList.count == 1)
    ;
  else{
    //myList.prevNode = myList.head;
    //myList.currentJob = myList.prevNode->next;
    int has_swapped = 1;
    JobListNode * prev = (JobListNode*) malloc(sizeof(JobListNode));
    JobListNode * current = (JobListNode*) malloc(sizeof(JobListNode));
      
    while(has_swapped){

      has_swapped = 0;
      prev = myList.head;
      current = prev->next;

      while(current != NULL){
        if (prev->interval > current->interval){
          swap(prev, current);
          has_swapped = 1;
        }
        prev = current;
        current = current->next;
      }
    }
  }
}

// Remove the current job from the job list
void remove_job(){
  if(myList.prevNode == NULL) {
    myList.head = myList.currentJob->next;
    free(myList.currentJob);
    myList.currentJob = myList.head;
  }
  else{
    myList.prevNode->next = myList.currentJob->next;
    free(myList.currentJob);
    myList.currentJob = myList.prevNode;
    //If element removed is the last element in the list, have tail change accordingly
    if (myList.tail == NULL)
      myList.tail = myList.prevNode;
  }
   
}

// Change the interval of the current job
void update_job_interval(size_t interval){
  myList.currentJob->interval = interval;

  bubblesort();
}

// Stop running the scheduler after the current job finishes
void stop_scheduler(){
  stopped = 0;
}

// Run the game scheduler until there are no jobs left to run or the scheduler is stopped
void run_scheduler(){

  bubblesort();
  myList.currentJob = myList.head;
  myList.prevNode = NULL;
  
  while(stopped) {

    if (myList.currentJob->lastruntime == -1)
      myList.currentJob->lastruntime = time_ms();

    
    if((time_ms() - myList.currentJob->lastruntime) >= myList.currentJob->interval) {
      myList.currentJob->job();
      myList.currentJob->lastruntime = time_ms();
    }

    myList.prevNode = myList.currentJob;
    myList.currentJob = myList.currentJob->next;
    if (myList.currentJob == NULL){
      myList.currentJob = myList.head;
      myList.prevNode = NULL;
    }
  }
}
