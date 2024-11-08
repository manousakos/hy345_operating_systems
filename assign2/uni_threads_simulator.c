#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t physicsSem, csdSem, chemSem, mathSem;
pthread_mutex_t mutex;
int *busCurrentStop; // 0 : is on the move, -1 is A, 1 is B

enum Department{
  PHYSICS,
  CSD,
  CHEMISTRY,
  MATHS
};

enum StudentState {
  STOP,
  BUS,
  UNI
};
enum BusState{
  WAIT,
  GO
};

struct Student {
  int AM;
  char dept[15];
};
/* each dept has a distinct 1st char that will help us with the selection
of the semaphores:

CSD: c
physics: p
maths: m
chem: h
*/

struct Student **State_Array;

/*This function checks if the bus has reached stop : stop  
busCurrentStop has only 3 values:
-1 : stop_A
0 : is on the move
1 : stop_B*/
bool busArrived( int stop ){
  if ( *busCurrentStop == stop ) return true;
  else return false;
}

// TODO : implement functionality of every student thread to change the DB 
void *student( void * AM ){
  

  int destinationStop=0;
  enum StudentState studState;
  bool currentStop = false; /* false == stopA , true == stopB */
  char amStud[15];
  int busDropCounter = 0; /* if counter== 2 , we back in A, thread finish*/
  int studyingTime = 5 + ( rand()%10 );

  
  strcpy(amStud, (char*)AM); // copy the string from null pointer AM to amStud

  studState = STOP; 
  printf("The student with AM : %s is waiting on the bus stop.\n",amStud);
  while(1){

//  TODO :  fix sem_wait, possibility of threads using wait eve if bus is not on stop.
    if(studState ==  STOP ){
      
    
      if ( busDropCounter >=2 )break; // we are back on stop_A, we finished


      /*if the queue allows it, the Students
      can call the wait function ( board the bus)*/
      if( amStud[0] == 'm' ){
        sem_wait( &mathSem );
      }
      else if( amStud[0] == 'c' ){
        sem_wait( &csdSem);
      }  
      else if( amStud[0] == 'p' ){
        sem_wait( &physicsSem);
      } 
      else if( amStud[0] == 'h' ){
        sem_wait( &chemSem);
      }

// setting destination stop before going to Bus state
// reminder: stopA = -1 , stopB is 1
      if( busDropCounter == 0){
        destinationStop = 1; // if currentStop = stopA ( busDropCounter == 0), dest = 1 ( stopB )
      }
      else{
        destinationStop = -1;
      }


      studState = BUS ;     // STATE --> BUS
    }
    else if( studState == BUS){
      




      sleep(1); /* check every 1 sec if we reached destination stop*/


      if( busArrived( destinationStop ) ) {
        studState = UNI;
        printf("Student %s got off the bus and entered the University\n", amStud) ;
        /* now that the student load of the bus
        they free the semaphores
        */
        if( amStud[0] == 'm' ){
          sem_post(&mathSem );
        }
        else if( amStud[0] == 'c' ){
          sem_post(&csdSem);
        }  
        else if( amStud[0] == 'p' ){
          sem_post( &physicsSem);
        } 
        else if( amStud[0] == 'h' ){
          sem_post( &chemSem);
        }
       studState = UNI;      // STATE --> UNI
        busDropCounter++;
      }


    }
    else if ( studState == UNI ){
      
      /* the student reads*/
      sleep(studyingTime);
      studState = STOP; //   STATE ---> Stop
    }
  }

  printf("Student %s went Home.\n",amStud);
}

bool changeBusStop(bool busStop ){
  return !busStop;
}


/* Helper function that returns a bool whether bus is full*/
bool seatsFull( void){

  bool isFull = false; 
  int chem_Sum, ph_Sum, csd_Sum, mth_Sum;

  sem_getvalue( &csdSem , &csd_Sum);

  sem_getvalue( &physicsSem , &ph_Sum);
  sem_getvalue( &mathSem , &mth_Sum);
  sem_getvalue( &chemSem , &chem_Sum);

  if( (mth_Sum + ph_Sum + csd_Sum + chem_Sum) == 0 ) {
    isFull = true ;
    printf("The bus is full, time to leave!\n");
  }
  else{
    printf("Bus is still not full...");
  }
  
  return isFull;
}




// TODO IMPLEMENT THE DB 
void *bus(void){
  int prevStop;   // init at stopA
  int waitCounter =0; // counter that counts to 3 secs, then if even if !busFull() / !stopEmpty(), leave
  int seatsN = 4;   // TODO , fix hardcoded stuff
  int busStop =0;   
  enum BusState busState;
  sem_init(&physicsSem, 0, seatsN);
  sem_init(&csdSem, 0, seatsN);
  sem_init(&mathSem, 0, seatsN);
  sem_init(&chemSem, 0, seatsN);
// TODO , implement exit method if all students go home / DB empty.

// bus init
  busState = WAIT;
  *busCurrentStop = -1;        // 1: stopA , 0: on the move , -1 stopB
  prevStop = -1;


/* This while loop operates the Bus State FSM */
  while(1){
    if( busState == WAIT ){
      
      // TODO Implement isEmpty()

      if( seatsFull() ){
        printf("Time to GO !\n");
        busState = GO;
      }
      else if(waitCounter == 2 ){
        waitCounter = 0 ; // reset counter
        busState = GO ; // leave regardless , timeout
      }
      else{
        sleep(1);
        printf("Waiting... Time to check if empty or full ! \n");
        waitCounter++;
      }
      
      //if(busStop >=2 )  break;
    }
    else if( busState == GO ){
      prevStop = *busCurrentStop;  // renew prevStop

      printf("We left the bus stop\n");
      *busCurrentStop = 0;      // on the move 
      sleep(10);
      
      // stop --> NewStop
      busStop ++;
      if( prevStop = -1) *busCurrentStop = 1;
      else  *busCurrentStop = -1;


      printf("We arrived at the bus stop \n");
      busState = WAIT;
    }
  }

  printf("All students have arrived at their homes.\n");
}


void printDS( struct Student **states, int numbStudents, int N ){

  int i,j;
  printf("Hello!");
  for(i = 0; i< 4 ; i++ ){
    switch (i)
    {
    case 0:
      printf("BUS : ");
      break;
    case 1:
      printf("STOP A: ");
      break;
    case 2:
      printf("UNIVERSITY");
      break;
    case 4:
      printf("STOP B");
      break;
      
    default:
      break;
    }
    if( i == 0 ){
      for( j = 0 ; j< N ; i++){

        if( states[i][j].AM != 0 ) {
          printf(" [ %d , %s ] ", states[i][j].AM ,states[i][j].dept );
        }
      }
    }
    else{
      for( j = 0 ; j< numbStudents ; j++){

        if( states[i][j].AM != 0 ) {
          printf(" [ %d , %s ] ", states[i][j].AM ,states[i][j].dept );
        }
      }
    }
    printf('\n');
  
  }
}

// initialise the Data Struct we have;
struct Student **initDS(struct Student **states , int N , int numbStudents){
  int i;
  int j;
  char testStr[7];  

  // init all to nothing
  for(  i =0 ; i< 4 ; i++ ){
    if( i == 0 ){
      for(j = 0; j<N ; j++){
        states[i][j].AM = 0; 
        
        strcpy( states[i][j].dept, "" );
      } 
      printf("We init'd the 1st row.\n");
    }
    else{
      for(j = 0; j< numbStudents ; j++){
        states[i][j].AM = 0; 
        strcpy( states[i][j].dept, "" );
      } 
    }
  }

  /* all students start from stop A that is the 2nd element of the State_Array*/
  for( i = 0; i < numbStudents; i++){
    j = rand()%4 ; 
    switch (j)
    {
    case 0:
      states[1][i].AM = i+1; 
      strcpy( states[1][i].dept,"physics" );
      break;

    case 1:
      states[1][i].AM = i+1; 
      strcpy( states[1][i].dept,"maths" );
      break;

    case 2:
      states[1][i].AM = i+1; 
      strcpy( states[1][i].dept,"csd" );
      break;

    case 3:
      states[1][i].AM = i+1; 
      strcpy( states[1][i].dept,"chemistry" );
      break;

    default:
      states[1][i].AM = i+1; 
      strcpy( states[1][i].dept,"chemistry" );     
      break;
    }
  }   

  printDS(states, numbStudents, N);
  return states;
}



int main(void){

  int N = 2;
  pthread_t buss;
  pthread_t stud;
  int validd;
  int i,j;
  int numbStudents = 1;

// TODO create an INIT funct
  
  
  srand(time(NULL)); 
  //pthread_mutex_init(&mutex, NULL);
  
/* the struct Bus_State has at a total N seats,
 * and the rest of the structs of the States have no real limits,
 * so we allocate space of size struct Student * NumberOfStudents 
 * */ 

/* the states are 4 so we only need to allocate space for 4 struct Student ** */
  State_Array = malloc(4*sizeof( struct Student * ));
    
  for(i = 0; i<4 ; i++){
    //if( i == 0 ) State_Array[i] = malloc( N * sizeof(struct Student)); // bus only has N seats
    //else 
      State_Array[i]=malloc( numbStudents * sizeof(struct Student));        // TODO fix this warning
  }
  
  busCurrentStop = malloc(sizeof(int) );
  
  if(busCurrentStop == NULL){
    printf("Error while trying to allocate space for the pointer busCurrentStop.\n");
    return -1;
  }
  
  *busCurrentStop = -1; // we start at stop_A

  printf("Please Enter the Number of students: ");
  scanf("%d",&numbStudents);

  if( numbStudents <= 0 || numbStudents > 200 ){
    printf("Error, input must be between 1 - 200.\n");

    return -1;
  }
  
  /*validd = pthread_create(&stud, NULL, student(5050), NULL); */

  printf("Lets Initialise our Students.\n");
  State_Array = initDS(State_Array, N, numbStudents);
  
  printf("We succesfully Initialised our students.\n");
  for(i =0; i<4 ; i++) printf("Here %d : %p ", i,State_Array);
  printDS(State_Array, numbStudents,N);
  
  validd = pthread_create(&buss, NULL, bus, NULL );

 for(i = 0; i<4 ; i++){
    if( i == 0 ) free( State_Array[i] );
    else free( State_Array[i] );
  }
  
  free(State_Array);



  return 0; 




}
