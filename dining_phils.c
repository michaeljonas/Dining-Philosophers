
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// initialize the mutex, monitor, that controls access to 
// functions GetChopSticks and ReleaseChopSticks
pthread_mutex_t monitor = PTHREAD_MUTEX_INITIALIZER;
// varailable used to signal when the mutex is unlocked, one for each chop stick
pthread_cond_t resource_0_ready;
pthread_cond_t resource_1_ready;
pthread_cond_t resource_2_ready;
pthread_cond_t resource_3_ready;
pthread_cond_t resource_4_ready;

// arrary to track the usage of each chop stick, false if in use
bool chopStick[5] = {true,true,true,true,true};

// start up function for the threads
void *phil_thread(void *param);
// is passed the Phil_id, then picks up the correct chop sticks
void GetChopSticks(int id);
// is passed the Phil_id, then releases the chop sticks 
void ReleaseChopSticks(int id);
// returns a random number
int Gen_Random();
// checks if the mutex, monitor, is locked
// waits if it is, and proceeds if not
void Get_Monitor(int Phil_id);
// blocks or waiting the calling thread
// with condition variable associated with the chop stick
void BlockOn(int chop_stick_id);
// signal mutex is available for given chop sticks, right then left
void SignalNeighbor(int phil_id);
// signal mutex is available for chop sticks that are not to the right or left
void SignalNotNeighbor(int phil_id);

int main (int argc, const char * argv[]){
    
    // numbers for the phil's
    char *id1 = "0";
    char *id2 = "1";
    char *id3 = "2";
    char *id4 = "3";
    char *id5 = "4";
    
    //creating each phil with thier own id
    pthread_t tid1;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid1,&attr,phil_thread,id1);
    
    pthread_t tid2;
    pthread_create(&tid2,&attr,phil_thread,id2);
    
    pthread_t tid3;
    pthread_create(&tid3,&attr,phil_thread,id3);
    
    pthread_t tid4;
    pthread_create(&tid4,&attr,phil_thread,id4);
    
    pthread_t tid5;
    pthread_create(&tid5,&attr,phil_thread,id5);
    
    //joining the threads to the process
    pthread_join( tid1, NULL);
    pthread_join( tid2, NULL);
    pthread_join( tid3, NULL);
    pthread_join( tid4, NULL);
    pthread_join( tid5, NULL);

    printf("SUCCESSFUL EXIT!\n");
    return 0;
}

void *phil_thread(void *param)
{
    int Phil_id = atoi(param);
    int count = 0;
    
    while (count < 10) {

        //Thinking - sleeping for a random ammount of time
        printf("Phil %d is thinking.\n", Phil_id);
        usleep(Gen_Random()); // thinking
        
        // get mutex lock, monitor
        Get_Monitor(Phil_id);
        
        // pick up chop sticks
        GetChopSticks(Phil_id);
        
        // Eating - sleeping for a random ammount of time
        printf("Phil %d is eating.\n", Phil_id);
        usleep(Gen_Random()); // eating
        
        // get mutex lock, monitor
        Get_Monitor(Phil_id);
        
        // put down chop sticks
        ReleaseChopSticks(Phil_id);
        
        count++;
    }
}

void GetChopSticks(int id){
    
    int left = id;                  // left chop stick number
    int right = (1+id)%5;           // right chop stick number
    
    // check chop stick availability, wait if both are not available
	while(!chopStick[left] || !chopStick[right]){
		if(!chopStick[left])
            BlockOn(left);
        else
            BlockOn(right);
	}
    
    // pick up each chop stick
	chopStick[left] = false;
	chopStick[right] = false;
    
    printf("Phil %d picked up his chop sticks\n", id);
    
    // release mutex, monitor
	pthread_mutex_unlock(&monitor);
    
    // signal waiting threads that the mutex is free
    SignalNotNeighbor(id);
    

}

void ReleaseChopSticks(int id){
        
    int left = id;                  // left chop stick number
    int right = (1+id)%5;           // right chop stick number
    
    // putting down each chop stick
	chopStick[left] = true;
	chopStick[right] = true;
    
    printf("Phil %d released his chop sticks\n", id);
    
    // releasing mutex, monitor
	pthread_mutex_unlock(&monitor);
    
    // signaling waiting threads that the mutex is free
    SignalNeighbor(id);
    
}

int Gen_Random(){
    // returns a random number
    return arc4random()%1000000;
}

void Get_Monitor(int Phil_id){
    // checks if the mutex, monitor, is locked
    // if so the thread blocks waiting for a signal
    // if not it locks the mutex and proceeds
    if(pthread_mutex_trylock(&monitor)){
        BlockOn(Phil_id);
    }
}

// blocks the calling thread with the condition varailable
// identified by the argument
void BlockOn(int chop_stick_id){
    if(chop_stick_id == 0)
        pthread_cond_wait(&resource_0_ready, &monitor);
    else if(chop_stick_id == 1)
        pthread_cond_wait(&resource_1_ready, &monitor);
    else if(chop_stick_id == 2)
        pthread_cond_wait(&resource_2_ready, &monitor);
    else if(chop_stick_id == 3)
        pthread_cond_wait(&resource_3_ready, &monitor);
    else if(chop_stick_id == 4)
        pthread_cond_wait(&resource_4_ready, &monitor);
    else
        printf("ERROR IN BLOCKON - CHOP_STICK_ID = %d", chop_stick_id);
}

// signals threads waiting for the chop stick on the right or left
// of the argument, threads waiting for their right are signaled first
void SignalNeighbor(int phil_id){
    if(phil_id == 0){
        pthread_cond_signal(&resource_0_ready);
        pthread_cond_signal(&resource_1_ready);
    }
    else if(phil_id == 1){
        pthread_cond_signal(&resource_1_ready);
        pthread_cond_signal(&resource_2_ready);
    }
    else if(phil_id == 2){
        pthread_cond_signal(&resource_2_ready);
        pthread_cond_signal(&resource_3_ready);
    }
    else if(phil_id == 3){
        pthread_cond_signal(&resource_3_ready);
        pthread_cond_signal(&resource_4_ready);
    }
    else if(phil_id == 4){
        pthread_cond_signal(&resource_4_ready);
        pthread_cond_signal(&resource_0_ready);
    }
    else
        printf("ERROR IN SIGNALON - PHIL_ID = %d", phil_id);
}

// signals threads waiting for the chop sticks not on the right or left
// of the argument
void SignalNotNeighbor(int phil_id){
    if(phil_id == 0){
        pthread_cond_signal(&resource_2_ready);
        pthread_cond_signal(&resource_3_ready);
    }
    else if(phil_id == 1){
        pthread_cond_signal(&resource_3_ready);
        pthread_cond_signal(&resource_4_ready);
    }
    else if(phil_id == 2){
        pthread_cond_signal(&resource_4_ready);
        pthread_cond_signal(&resource_0_ready);
    }
    else if(phil_id == 3){
        pthread_cond_signal(&resource_0_ready);
        pthread_cond_signal(&resource_1_ready);
    }
    else if(phil_id == 4){
        pthread_cond_signal(&resource_1_ready);
        pthread_cond_signal(&resource_2_ready);
    }
    else
        printf("ERROR IN SIGNALOFF - PHIL_ID = %d", phil_id);
}



