#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
// Single Producer - Multiple Consumer queue
template <typename T>
class PCQueue
{

public:

    PCQueue(){
        mutex= PTHREAD_MUTEX_INITIALIZER;
        signal= PTHREAD_COND_INITIALIZER;
    }
    

    //template <typename T>
    T pop() {
        pthread_mutex_lock(&mutex);
        while(PCQ.empty()){
            pthread_cond_wait(&signal,&mutex);
        }
        auto item= PCQ.front();
        PCQ.pop();
        pthread_mutex_unlock(&mutex);
        return item;
    }

  //  template <typename T>
    void push(const T& item) {
        pthread_mutex_lock(&mutex);
        PCQ.push(item);
        pthread_cond_signal(&signal);
        pthread_mutex_unlock(&mutex);
    }

    bool empty(){
        return PCQ.empty();
    }


private:
	std:: queue<T> PCQ;
	pthread_mutex_t mutex;
	pthread_cond_t signal;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif