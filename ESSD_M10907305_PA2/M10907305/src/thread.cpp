#include "thread.h"

int sem1_key = 0;
int sem2_key = 0;
/**
 * Set up the id, matrixSize, and all matrix.
 *
 */
void
Thread::initialThread (int program_id, int id, int matrix_size, int** single_result, 
                       int** multi_result, int** mul_matrix, int** input_matrix, int* shared_sum)
{
    programID = program_id;
    ID = id;
    matrixSize = matrix_size;

    singleResult = single_result;
    multiResult = multi_result;
    matrix = mul_matrix;
    inputMatrix = input_matrix;

    sharedSum = shared_sum;
}


void
Thread::setThreadCore(int core)
{ 
	setCore = core;
}


void
Thread::setStartCalculatePoint(int start_calculate_point)
{
    startCalculatePoint = start_calculate_point;
}


void
Thread::setEndCalculatePoint(int end_calculate_point)
{
    endCalculatePoint = end_calculate_point;
}


/**
 * Set up the affinity mask for the thread.
 *
 */
void
Thread::setUpCPUAffinityMask ()
{
	if (setCore == -1) {
        return;
    } else {
		cpu_set_t set;
		CPU_ZERO (&set);
		CPU_SET (setCore, &set);
		sched_setaffinity (0, sizeof (set), &set);
		setCore = -1;
	}
}


/**
 * Reset the matrix store value by copy the inputMatrix.
 *
 */
void
Thread::resetMatrix()
{
    for (int i = 0; i < matrixSize; i++)
        memcpy (matrix [i], inputMatrix [i], matrixSize * sizeof (int));
}


/**
 * Print out the tread information inclue ID, PID,
 * and execute core.
 *
 */
void
Thread::printInformation()
{
	core = sched_getcpu ();
	PID = syscall (SYS_gettid);
	pthread_mutex_lock (ioMutex);
    std::cout << "Program ID : " << programID;
    std::cout << "\tThread ID : " << ID;
    std::cout << "\tPID : " << PID;
    std::cout << "\tCore : " << core;
    std::cout << std::endl;
	pthread_mutex_unlock (ioMutex);
}


void
Thread::synchronize ()
{
#if SYNCHRONIZE == BARRIER
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
	// std::cout << "1" << std::endl;
	pthread_barrier_wait(barr);
	// std::cout << "2" << std::endl;
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#elif SYNCHRONIZE == SEMAPHORE

	sem_wait(sem);
	pthread_mutex_lock (ioMutex);
	++sem1_key ;
	pthread_mutex_unlock (ioMutex);
	if( sem1_key == THREAD_NUM*PROGRAM_NUM){
		pthread_mutex_lock (ioMutex);
		for(int i = 0 ; i<THREAD_NUM*PROGRAM_NUM ; ++i){
			sem_post(sem1);
		}
		pthread_mutex_unlock (ioMutex);
	}
	sem_post(sem);
	sem_wait(sem1);
	sem2_key = 0;
	-- sem1_key;
	while(1){
		if(sem1_key==0 or sem2_key ==1){
			break;
		}
	}
	sem2_key = 1 ;


#else

	pthread_mutex_lock (ioMutex);
    std::cout << "Synchronize method not supported." << std::endl;
	pthread_mutex_unlock (ioMutex);
#endif
}


void 
Thread::enterCriticalSection ()
{
#if PROTECT_SHARED_RESOURCE == MUTEX
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
	pthread_mutex_lock(ioMutex);


	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
	pthread_mutex_lock (ioMutex);
    std::cout << "Synchronize method not supported." << std::endl;
	pthread_mutex_unlock (ioMutex);
#endif
}


/**
 * Multi the array matrix to matrix for MULTI_TIME times.
 * After finishing multiplication, copy the result back to
 * matrix and do the multiplication agian.
 *
 */
void
Thread::singleMatrixMultiplication()
{	
	printInformation();

	// Multiplication for MULTI_TIME times
    for (int num_multi = 0; num_multi < MULTI_TIME; num_multi++) {

	    for (int i = 0 ; i < matrixSize; i++) {

	    	for (int j = 0 ; j < matrixSize; j++) {

                singleResult [i][j] = 0;
	    		for (int k = 0 ; k < matrixSize; k++)
	    			singleResult [i][j] += matrix [i][k] * matrix [k][j];

	    	} // for (int j...

	    } // for (int i...

        // Copy the singleResult back to matrix
        for (int i = 0; i < matrixSize; i++)
            memcpy (matrix [i], singleResult [i], matrixSize * sizeof (int));

    } // for (int num_multi...

}


/*
 * Matrix multiplication operation and store the result into
 * multieResult. 
 *
 */
void*
Thread::matrixMultiplication(void* args)
{
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/

    Thread *obj = (Thread*)args;
	
	obj->setUpCPUAffinityMask ();
	obj->printInformation ();

	// obj->enterCriticalSection();

	// Multiplication for MULTI_TIME times
	// obj->synchronize();
    for (int num_multi = 0; num_multi < MULTI_TIME; num_multi++) {
		
		
	    for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++) {

	    	for (int j = 0 ; j < obj->matrixSize; j++) {

#if (PART != 2)
				if(PART == 1){
					pthread_mutex_lock(&count_mutex);
					*obj->sharedSum = 0;
					for (int k = 0 ; k < obj->matrixSize; k++)
						*obj->sharedSum += obj->matrix [i][k] * obj->matrix [k][j];

					obj->multiResult [i][j] = *obj->sharedSum;
					pthread_mutex_unlock(&count_mutex);
				}
				else{
					pthread_spin_lock(&lock);
					*obj->sharedSum = 0;
					for (int k = 0 ; k < obj->matrixSize; k++)
						*obj->sharedSum += obj->matrix [i][k] * obj->matrix [k][j];

					obj->multiResult [i][j] = *obj->sharedSum;
					pthread_spin_unlock(&lock);
				}

#else

	        /*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
				int local_parameter = 0;
	    		for (int k = 0 ; k < obj->matrixSize; k++)
	    			// *obj->sharedSum += obj->matrix [i][k] * obj->matrix [k][j];
					local_parameter += obj->matrix [i][k] * obj->matrix [k][j];
                obj->multiResult [i][j] = local_parameter;
	        /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

#endif


	    	} // for (int j...

	    } // for (int i...
        // Copy the multiResult back to matrix
		obj->sem_para = 0;
		obj->synchronize();
        for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++){
            memcpy (obj->matrix [i], obj->multiResult [i], obj->matrixSize * sizeof (int));
		}
		obj->sem_para =1;
		obj->synchronize();
    } // for (int num_multi...

	

    return 0;
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
}
