#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "Headers.hpp"

// Synchronization Warm up 
class Semaphore {
public:
	Semaphore(){ // Constructs a new semaphore with a counter of 0			
		counter= 0;
		mutex= PTHREAD_MUTEX_INITIALIZER;
		signal= PTHREAD_COND_INITIALIZER;
	}
	
	Semaphore(unsigned val){
		// Constructs a new semaphore with a counter of val
		
		counter= val;
		mutex= PTHREAD_MUTEX_INITIALIZER;
		signal= PTHREAD_COND_INITIALIZER;
	}


	void up(){
		// Mark: 1 Thread has left the critical section
		pthread_mutex_lock(&mutex);
		counter++;
		pthread_mutex_unlock(&mutex);
		if (counter > 0){
			pthread_cond_signal(&signal);
		}
	}

	void down(){ // Block untill counter >0, and mark - One thread has entered the critical section.
		pthread_mutex_lock(&mutex);
		while (counter <= 0) {
			pthread_cond_wait(&signal,&mutex);
		}
		counter--;
    pthread_mutex_unlock(&mutex);
	}

private:
	int counter;
	pthread_mutex_t mutex;
	pthread_cond_t signal;
};

#endif
