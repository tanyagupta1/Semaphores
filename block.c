#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
/*non blocking semaphore structure */
typedef struct
{
    int value;
    pthread_mutex_t mutex;
	pthread_cond_t cond;

} blocking_semaphore;
typedef struct
{
   int val;
   int n;
} my_arg;
void initialise_sem(blocking_semaphore *S, int a)
{
        pthread_mutex_lock(&(S->mutex));
        S->value=a;
        pthread_mutex_unlock(&(S->mutex));

}
/*wait() implementation */
void blocking_wait(blocking_semaphore *S)
{
    
    pthread_mutex_lock(&(S->mutex));
    while(S->value<=0)
    {
        pthread_cond_wait(&(S->cond), &(S->mutex));
        //temporarily release the mutex lock and make the thread sleep until signalled
    }
    S->value--;
    pthread_mutex_unlock(&(S->mutex));
    //mutex lock while decremenenting the value of the semaphore to prevent race conditions

}
void blocking_signal(blocking_semaphore *S)
{
    
    pthread_mutex_lock(&(S->mutex));
    S->value++;
    pthread_cond_broadcast(&(S->cond));
    //waking up all the threads that have been waiting on this semaphore's comditional variable   
    pthread_mutex_unlock(&(S->mutex));
    //mutex lock while incremenenting the value of the semaphore to prevent race conditions

}
blocking_semaphore dining_room;
blocking_semaphore forks[1000];
blocking_semaphore sauce_bowls[2];//all the required semaphores
int diningroom=0; int forkTaken[1000]; int sauceBowlTaken[2]; //all shared variables
void * philosopher(void * num) //this is the function allotted to each philosopher thread
{
	int phil=((my_arg *)num)->val;;
    int n=((my_arg *)num)->n;

	blocking_wait(&dining_room); //try to enter dining room
    diningroom++;
	printf("\nP%d has entered room",phil);
	blocking_wait(&forks[phil]);   //try to take i-th fork
    forkTaken[phil]++;
    printf("\nP%d has received F%d",phil,phil);
	blocking_wait(&forks[(phil+1)%n]); //try to take i+1 th fork
    forkTaken[(phil+1)%n]++;
    printf("\nP%d has received F%d",phil,(phil+1)%n);
    blocking_wait(&sauce_bowls[0]); //try to take first bowl
    sauceBowlTaken[0]++;
    printf("\nP%d has received B%d",phil,1);
    sauceBowlTaken[1]++;
    blocking_wait(&sauce_bowls[1]); //try to take second bowl
    printf("\nP%d has received B%d",phil,2);
    sleep(1);                        //eating
    printf("\nP%d has finished eating",phil);
    sauceBowlTaken[1]--;
    blocking_signal(&sauce_bowls[1]);  //release 2nd bowl
    printf("\nP%d has released B%d",phil,2);
    sauceBowlTaken[0]--;
    blocking_signal(&sauce_bowls[0]); //release 1st bowl
    printf("\nP%d has released B%d",phil,1 );
    forkTaken[(phil+1)%n]--;
    blocking_signal(&forks[(phil+1)%n]); //release i+1th fork
    printf("\nP%d has released F%d",phil,(phil+1)%n);
    forkTaken[phil]--;
	blocking_signal(&forks[phil]);  //release ith fork
    printf("\nP%d has released F%d",phil,phil);
    diningroom--;
    blocking_signal(&dining_room);  //leave dining room
    printf("\nP%d has left room",phil);
}
//implementation signal_printVal for debugging
void signal_printVal(blocking_semaphore *S)
{
    printf("%d\n",S->value);
}
int main()
{
    int n;
    printf("Enter number of philosophers(>=2):");
    scanf("%d",&n);
    //initialse shared variables
    diningroom=0; //0 people in dining room
    for(int i=0;i<n;i++)
    {
        forkTaken[i]=0; //fork not taken yet
    }
    sauceBowlTaken[0]=0; sauceBowlTaken[1]=0; //sauce bowls not taken yet;
    //array of philosopher threads, array of my_args to pass n and i

    pthread_t philosophers[n]; my_arg a[n];
    //initialising all the semaphores
    initialise_sem(&dining_room,n-1); 
    initialise_sem(&sauce_bowls[0],1);
    initialise_sem(&sauce_bowls[1],1);
    for(int i=0;i<n;i++) {initialise_sem(&forks[i],1); a[i].val=i; a[i].n=n;}

	for(int i=0;i<n;i++)pthread_create(&philosophers[i],NULL,philosopher,(void *)&a[i]);
	/*int i=0;
    while(1)
    {
        
        pthread_create(&philosophers[i%n],NULL,philosopher,(void *)&a[i%n]);
        i++;
    } */
	for(int i=0;i<n;i++)pthread_join(philosophers[i],NULL);
    return 0;

}