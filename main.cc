/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

// Create class for jobs 
class Job {
public:
  int job_id;
  int duration; 
};

// Create class for buffer
class Buffer {
public:
  int producer_loc;
  int consumer_loc;
  int array_len;
  Job* data;
};

// General declarations 
void initialise_variables(char** argv);
int initialise_semaphores();
void initialise_buffer();
int random_int(int min, int max);

// Producer declarations
void* producer (void* id);
void deposit_item(Job job);

// Consumer declarations
void* consumer (void* id);
Job fetch_item();

// Global variables - Semaphores
int item = 0, space = 1, mutex = 2, sem_id;

// Global variables - Producers & Consumers
int buffer_len, jobs_per_producer, producer_no, consumer_no;
Buffer buffer;

int main (int argc, char** argv) {

  //Use computer's internal clock to control seed for 'random' number generation
  srand(time(NULL));

  //Check argument number
  if(argc != 5) {
    cerr << "Incorrect number of parameters" << endl;
    cerr << "Usage: ./main <queue size> <jobs per producer> <consumer number> <consumer number>" << endl;
    return INCORRECT_NUMBER_OF_PARAMETERS;
  }
  
  //Check argument values
  for(int i = 1; i < argc; i++) 
    if(check_arg(argv[i]) == -1) {
      cerr << "Parameters must be positive integers" << endl;
      return NON_POSITIVE_INTEGER;
    }
  
  //Create a semaphore set and check for errors
  sem_id = sem_create(SEM_KEY, 3);
  if (sem_id == -1) {
    cerr << "Error creating semaphore set. Error: " << endl;
    semget_err(errno);
    return errno;
  }

  initialise_variables(argv);

  //Declare array of POSIX threads for producers and consumers
  //pthread_t identifies a thread
  pthread_t producer_arr[producer_no];
  pthread_t consumer_arr[consumer_no];

  //Initialise semaphores and check for errors
  if (initialise_semaphores() != NO_ERROR) {
    cerr << "Error initialising semaphores. Error: " << endl;
    semctl_err(errno);
    sem_close(sem_id);
    return errno;	
  }
    
  initialise_buffer();

  //Create producer POSIX threads and check for errors
  int* producer_thread_err = new int [producer_no];
  for(int i = 0; i < producer_no; i++) {
    if (pthread_create(&producer_arr[i], NULL, producer, (void *) (intptr_t) (i + 1)) != NO_ERROR) {
      producer_thread_err[i] = 0;
      cerr << "Error creating producer POSIX thread " << i << ". Error message: ";
      pthread_create_err(errno);
    }
    else {
      producer_thread_err[i] = 1;
    }
  }
  
  //Create consumer POSIX threads and check for errors
  int* consumer_thread_err = new int [consumer_no];
  for(int i = 0 ; i < consumer_no; i++) {
    if (pthread_create(&consumer_arr[i], NULL, consumer, (void *) (intptr_t) (i + 1)) != NO_ERROR) {
      consumer_thread_err[i] = 0;
      cerr << "Error creating consumer POSIX thread " << i << ". Error message: ";
      pthread_create_err(errno);
    }
    else {
      consumer_thread_err[i] = 1;
    }
  }
  
  //Wait for producer threads to finish and check for errors
  for(int i = 0; i < producer_no; i++)
    if (producer_thread_err[i] == 1) 
      if (pthread_join(producer_arr[i], NULL) != NO_ERROR) {
	cerr << "Error joining producer POSIX thread " << i << ". Error message: ";
	pthread_join_err(errno);
      }
  

  //Wait for consumer threads to finish and check for errors
  for(int i = 0; i < consumer_no; i++)
    if (consumer_thread_err[i] == 1) 
      if (pthread_join(consumer_arr[i], NULL) != NO_ERROR) {
	cerr << "Error joining consumer POSIX thread " << i << ". Error message: ";
	pthread_join_err(errno);
      }
	       
  
  //Close semaphore set and check for errors
  if (sem_close(sem_id) != NO_ERROR) {
    cerr << "Error closing semaphore set. Error message: " << endl;
    semctl_err(errno);
    delete [] producer_thread_err;
    delete [] consumer_thread_err;
    delete [] buffer.data;
    return errno;
  }

  //Delete dynamic memory
  delete [] producer_thread_err;
  delete [] consumer_thread_err;
  delete [] buffer.data;
  
  return NO_ERROR;
}

void initialise_variables(char **argv) {
  buffer_len = check_arg(argv[1]);
  jobs_per_producer = check_arg(argv[2]);
  producer_no = check_arg(argv[3]);
  consumer_no = check_arg(argv[4]);
}

int initialise_semaphores() {
  if (sem_init(sem_id, item, 0) != NO_ERROR) {
    cerr << "Error initialising 'item' semaphore. Error message: " << endl;
    sem_init_err(errno);
    return errno;
  }
  
  if (sem_init(sem_id, space, buffer_len) != NO_ERROR) {
    cerr << "Error initialising 'space' semaphore. Error message: " << endl;
    sem_init_err(errno);
    return errno;
  }
  
  if (sem_init(sem_id, mutex, 1) != NO_ERROR) {
    cerr << "Error initialising 'mutex' semaphore. Error message: " << endl;
    sem_init_err(errno);
    return errno;
  }
  
  return NO_ERROR;
}

void initialise_buffer() {
  buffer.producer_loc = 0;
  buffer.consumer_loc = 0;
  buffer.array_len = buffer_len;
  buffer.data = new Job[buffer_len];
}

int random_int(int lower, int upper) {
  int random = lower + (rand() % upper);
  return random;
}

void* producer(void* id) {
  int producer_number = (intptr_t) id;
  Job job;
  bool timeout = false;
  
  for(int i = 0; i < jobs_per_producer; i++) {

    //produce item - takes 1 to 10 seconds
    int duration = random_int(1, 10);
    
    //job added - every 1 to 5 seconds
    sleep(random_int(1, 5));		

    //down (space) - attempt for 20 seconds, else timeout
    if (sem_timed_wait(sem_id, space, 20) != NO_ERROR) {
      printf("Producer(%d): terminated due to a timeout\n", producer_number);
      timeout = true;
      break;
    }

    //down (mutex)
    sem_wait(sem_id, mutex);

    //create new job with id based on location job will occupy in buffer, and duration of job
    job.job_id = (buffer.producer_loc + 1);
    job.duration = duration;

    //deposit job in buffer 
    deposit_item(job);

    //up (mutex)
    sem_signal(sem_id, mutex);

    //up (item)
    sem_signal(sem_id, item);
      
    //output details of operation
    printf("Producer(%d): Job ID %d duration %d\n", producer_number, job.job_id, job.duration);
  }
  
  //output no more jobs if no timeout
  if(timeout == false)
    printf("Producer(%d): No more jobs to generate\n", producer_number);
  
  //close pthread
  pthread_exit(0);
}

void deposit_item(Job job) {
  buffer.data[buffer.producer_loc] = job;
  buffer.producer_loc = ((buffer.producer_loc + 1) % buffer_len);	
}

void* consumer (void *id) {
  int consumer_number = (intptr_t) id;
  Job job;

  //down(item) until buffer empty for 20 seconds
  while(sem_timed_wait(sem_id, item, 20) == 0) {
    
      //down(mutex)
      sem_wait (sem_id, mutex);
      
      //fetch job from buffer
      job = fetch_item();
      
      //up(mutex)
      sem_signal (sem_id, mutex);
      
      //up(space)
      sem_signal (sem_id, space);
      
      //output details of operation
      printf("Consumer(%d): Job ID %d executing sleep duration %d\n", consumer_number, job.job_id, job.duration);
      
      //consume job - takes same time as took to produce job
      sleep(job.duration);
      
      //output detials of consumption
      printf("Consumer(%d): Job ID %d completed\n", consumer_number, job.job_id);
    }
  
  //output when timeout  
  printf("Consumer(%d): No more jobs left\n", consumer_number);
  
  //close thread
  pthread_exit(0);
}

Job fetch_item() {
  Job _job = buffer.data[buffer.consumer_loc];
  buffer.consumer_loc = ((buffer.consumer_loc + 1) % buffer_len);
  
  return _job;
}

