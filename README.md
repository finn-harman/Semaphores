# Semaphores

A typical producer-consumer scenario consists of a group of producers ‘producing’ jobs with a group of consumers ‘consuming’ these jobs. In order for the producers and
consumers to be able to communicate with each other, a shared data structure is traditionally used to store the jobs and with which the producers and consumers interact. Due to the shared nature of this data structure, it needs to be protected by synchronisation mechanisms to ensure consistent access to the data. In this project, I implement the producer-consumer scenario using POSIX threads, where each producer and consumer will be running in a different thread. The shared data structure used will be a
circular queue, which can be implemented as an array for this coursework. This array will contain details of the job, where a job has a job id (which is one plus the location that they occupy in the array ) job ids will start from 1) and a duration (which is the time taken to ‘process’ the job, processing a job will mean that the consumer thread will sleep for that duration).

### Example use

Example use:
```
$ git clone https://github.com/finn-harman/Semaphores.git
$ cd Semaphores
$ ./main
```

## Authors

* **Finn Harman** - [GitHub](https://github.com/finn-harman)
