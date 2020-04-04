#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <queue>
using namespace std;
int sizeOfBuffer;
queue<int> buffer;
// semaphores
sem_t mutexCounter; // counter semaphore
sem_t bufferSize, noOfItems, bufferUsage; // producer consumer semaphores
int messageCount; // global shared resource that holds number of messages
// 3 functions for 3 thread types
void* producer(void*);
void* consumer(void*);
void* incrementCounter(void*);
int randomSleepSeconds(); // generates random number for sleep intervals
int main()
{
    int mCounter_count; // N of mCounter threads
    cout << "Enter mCounter count: ";
    cin >> mCounter_count;
    cout << "Enter size of buffer: "; // size of buffer
    cin >> sizeOfBuffer;
    messageCount = 0; // global shared resource 
    pthread_t mMonitor; // producer thread
    pthread_t mCollector; // consumer thread
    pthread_t mCounter[mCounter_count]; // N mCounter threads
    sem_init(&mutexCounter,0,1); // init counter usage semaphore to one for thread use
    sem_init(&bufferUsage,0,1); // init bufferUsage semaphore to one for thread use
    sem_init(&noOfItems,0,0); // initializing no of items semaphore to zero items
    sem_init(&bufferSize,0,sizeOfBuffer); // initializing buffersize semaphore to siz Of buffer input
    pthread_create(&mMonitor,NULL,producer,NULL); // creating Monitor thread
    // Creation of mCounter threads 
    for(int i = 0 ; i < mCounter_count ; ++i){
        int *arg = (int*)malloc(sizeof(*arg));
        *arg = i+1;
        pthread_create(&mCounter[i],NULL,incrementCounter,arg);
    }
    // Creating consumer thread
    pthread_create(&mCollector,NULL,consumer,NULL);
    // joining threads
    for(int i = 0 ; i < mCounter_count ; ++i)
        pthread_join(mCounter[i],NULL);
    pthread_join(mMonitor,NULL);
    pthread_join(mCollector,NULL);
}
// producer function
void* producer(void* arg)
{
    sleep(randomSleepSeconds()); // inteval of time t1
    int localCounter; /* local variable to hold global variable resource value*/
    while(true){   // produce
        int ret = sem_trywait(&mutexCounter);
        if(ret == -1) // if can't enter counter critical section
        {
            cout << "Monitor thread: waiting to read counter" << endl;
            sleep(1);
            sem_wait(&mutexCounter);
        }
        // Counter critical section
        // get counter global variable resource and put it in local variable
        // reset counter global variable resource value to 0
        localCounter = messageCount;
        messageCount = 0;
        cout << "Monitor thread: reading a count value of " << localCounter << endl;
        sem_post(&mutexCounter);
        int ret2 = sem_trywait(&bufferSize);
        if(ret2 == -1) // if buffer is full
        {
            cout << "Monitor thread: Buffer full!!" << endl;
            sleep(randomSleepSeconds());
            sem_wait(&bufferSize);
        }
        sem_wait(&bufferUsage);
        // buffer critical section
        cout << "Monitor thread: writing to buffer at position " << buffer.size() << " value: " << localCounter <<endl;
        buffer.push(localCounter); // add to buffer
        sleep(randomSleepSeconds());
        sem_post(&bufferUsage);
        sem_post(&noOfItems);
    }
    
}
// consumer function
void* consumer(void* arg)
{
    
    sleep(randomSleepSeconds()); // sleeping for interval of time
    int poppedElement; // local variable to hold popped item from buffer (FIFO)
    while(true){
        int ret = sem_trywait(&noOfItems);
        if(ret == -1) // if buffer is still empty or has no items in it
        {
            cout << "Collector thread: nothing is in the buffer!" << endl;
            sleep(randomSleepSeconds());
            sem_wait(&noOfItems);
        }
        sem_wait(&bufferUsage);
        // critical section
        poppedElement = buffer.front(); // FIFO
        cout << "Collector thread: reading from buffer value: " << poppedElement << endl;
        buffer.pop(); // removing front
        sleep(randomSleepSeconds()); // sleeping for interval
        sem_post(&bufferUsage);
        sem_post(&bufferSize);
    }
    
}
// Counter function
void* incrementCounter(void* arg)
{
        sleep(randomSleepSeconds());
        int threadNumber = *((int*)arg);
        free(arg);
        cout << endl <<"Counter thread " << threadNumber << ": received a message" << endl; 
        sleep(1);
        int ret = sem_trywait(&mutexCounter);
        if(ret == -1) // if can't enter critical section
        {
            cout << endl << "Counter thread " << threadNumber << ": waiting to write" << endl; 
            sleep(1);
            sem_wait(&mutexCounter);
        }
        // critical section
        // incrementing global variable counter resource
        ++messageCount;
        cout << "Counter thread "<< threadNumber <<": now adding to counter, counter value="<< messageCount << endl;
        sleep(randomSleepSeconds());
        sem_post(&mutexCounter);
}
// random number generator 
int randomSleepSeconds()
{
    return (rand() % 3) + 1;;
}

