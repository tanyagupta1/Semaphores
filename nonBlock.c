#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
/*non blocking semaphore structure */
typedef struct
{
   int value;
   pthread_mutex_t sem_mutex;
} non_block_semaphore;
typedef struct
{
   int val;
   int n;
} my_arg;
/*wait() implementation */
void semaphore_wait(non_block_semaphore *S)
{
    
    while(S->value<=0);
    pthread_mutex_lock(&(S->sem_mutex));
    S->value--;
    pthread_mutex_unlock(&(S->sem_mutex));
    //mutex lock while decremenenting the value of the semaphore to prevent race conditions

}
void semaphore_signal(non_block_semaphore *S)
{
    pthread_mutex_lock(&(S->sem_mutex));
    S->value++;
    pthread_mutex_unlock(&(S->sem_mutex));
    //mutex lock while incremenenting the value of the semaphore to prevent race conditions
}
void semaphore_initialize(non_block_semaphore *S,int val)
{
    pthread_mutex_lock(&(S->sem_mutex));

    S->value=val;
    pthread_mutex_unlock(&(S->sem_mutex));
    //mutex lock while initialising the value of the semaphore to prevent race conditions
}
//implementation signal_printVal for debugging
void signal_printVal(non_block_semaphore *S)
{
    printf("%d\n",S->value);
}

non_block_semaphore dining_room;
non_block_semaphore forks[1000];
non_block_semaphore sauce_bowls[2]; //all the required semaphores
int diningroom=0; int forkTaken[1000]; int sauceBowlTaken[2]; //shared variables
void * philosopher(void * num)        //this is the function allotted to each philosopher thread
{
	int phil=((my_arg *)num)->val;;
    int n=((my_arg *)num)->n;

	semaphore_wait(&dining_room);         //try to enter dining room
    diningroom++;
	printf("\nP%d has entered room",phil);
	semaphore_wait(&forks[phil]);        //try to take i-th fork
    forkTaken[phil]++;
    printf("\nP%d has received F%d",phil,phil);
	semaphore_wait(&forks[(phil+1)%n]);  //try to take i+1 th fork
    forkTaken[(phil+1)%n]++;
    printf("\nP%d has received F%d",phil,(phil+1)%n);
    semaphore_wait(&sauce_bowls[0]);    //try to take first bowl
    sauceBowlTaken[0]++;
    printf("\nP%d has received B%d",phil,1);
    semaphore_wait(&sauce_bowls[1]);   //try to take second bowl
    sauceBowlTaken[1]++;
    printf("\nP%d has received B%d",phil,2);
    sleep(1);                             //eating
    printf("\nP%d has finished eating",phil);
    sauceBowlTaken[1]--;
    semaphore_signal(&sauce_bowls[1]);    //release 2nd bowl
    printf("\nP%d has released B%d",phil,2);
    sauceBowlTaken[0]--;
    semaphore_signal(&sauce_bowls[0]);    //release 1st bowl
    printf("\nP%d has released B%d",phil,1 );
    forkTaken[(phil+1)%n]--;
    semaphore_signal(&forks[(phil+1)%n]);  //release i+1th fork
    printf("\nP%d has released F%d",phil,(phil+1)%n);
    forkTaken[phil]--;
	semaphore_signal(&forks[phil]);      //release ith fork
    printf("\nP%d has released F%d",phil,phil);
    diningroom--;
	semaphore_signal(&dining_room);      //leave dining room
    printf("\nP%d has left room",phil);
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
        forkTaken[i]=0;    //fork not taken yet
    }
    sauceBowlTaken[0]=0; sauceBowlTaken[1]=0; //sauce bowls not taken yet;
    

    //array of philosopher threads, array of my_args to pass n and i
    pthread_t philosophers[n]; my_arg a[n];
    //initialising all the semaphores
    semaphore_initialize(&dining_room,n-1);
    for(int i=0;i<n;i++) {semaphore_initialize(&forks[i],1); a[i].val=i; a[i].n=n;}
    semaphore_initialize(&sauce_bowls[0],1);
    semaphore_initialize(&sauce_bowls[1],1);

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