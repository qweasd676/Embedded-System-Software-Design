#include "thread.h"

void
Thread::setThreadID(int ID)
{
	_ID = ID;
}


void
Thread::setThreadCore(int core)
{ 
	setCore = core;
}


void
Thread::setThreadMatrixSize(int matrix_size)
{
	_matrixSize = matrix_size;
}	


/*
 * Set up the affinity mask for the thread.
 *
 * @ Part1 Recommendation:
 * Implement the function to pinned current thread
 * to core with index CPU_NUM.
 *
 */
void
Thread::setUpCPUAffinityMask(int cpu_num)
{
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Pined the thread to core.
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu_num,&set);
	sched_setaffinity(0,sizeof(set),&set);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
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


void
Thread::setSchedulingPolicy(int policy)
{
    _schedulingPolicy = policy;
}


void 
Thread::setCheck (Check* tmp_check)
{
    check = tmp_check;
}


/*
 * Set up the _utilization of the thread depend on the size of matrix.
 * Let matrix pointer (singleResult, multiResult and Matrix) point 
 * to the matrix whcih create by System.
 *
 */
void
Thread::initialThread(float** single_result, float** multi_result, float** input_matrix)
{
	_utilization = float(_matrixSize / float(UTILIZATION_DIVIDER));	

    startCalculatePoint = 0;
    endCalculatePoint = _matrixSize;

    singleResult = single_result;
    multiResult = multi_result;
    matrix = input_matrix;
}


/*
 * Matrix multiplication operation and store the result into
 * singleResult which used to check the corennectness of multiResult 
 * by compare singleResult with multiResult.
 *
 */
void
Thread::singleMatrixMultiplication()
{	
	/* Print Thread information */
	core = sched_getcpu();
	PID = syscall(SYS_gettid);
	printInformation();

	/* Multiplication */
	for (int i = 0 ; i < _matrixSize; i++) {
		for (int j = 0 ; j < _matrixSize; j++) {
			singleResult[i][j] = 0;
			for (int k = 0 ; k < _matrixSize; k++) {
				singleResult[i][j] += matrix[i][k]*matrix[k][j];
			}	
		}
	}
}


/*
 * Matrix multiplication operation and store the result into
 * multieResult. 
 *
 * 1. For all part, pinned the thread to coresspond core dependent
 *    on variable setCore. 
 *
 * 2. For all part, using the system call the check is thread migrat
 *    to others core or not.
 *
 * 3. For Part3, call the member function setUpScheduler to set the 
 *    scheduler for current thread, and print out the Core0 current
 *    executed thread id to observie the different between FIFO and RR.
 *
 * @ Part1 Recommendation:
 * Implementation of pinned the current thread onto specify core.
 * Could be accomplish by call function SetUpCPUAffinityMask or other methods.
 * Implementation of detected the thread is migrate or not. 
 *
 * @ Part3 Recommendation:
 * Implementation of detected the Core0 is encounter the context
 * switch or not. If there is context swich then print out it was
 * switch from thread # to thread #.
 *
 */
void*
Thread::matrixMultiplication(void* args)
{
    Thread *obj = (Thread*)args;
#if (PART == 3)
    obj->setUpScheduler();
#endif
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Set up the affinity mask
	// pthread_mutex_lock(&count_Mutex);
	if(obj->setCore == -1 ){
		obj->core = sched_getcpu();
		obj->PID = syscall(SYS_gettid);
		if(PART != 3){
			obj->printInformation();
		}
		
	}
	else{
		obj->setUpCPUAffinityMask(obj->setCore);
		obj->core = sched_getcpu();
		obj->PID = syscall(SYS_gettid);
		if(PART != 3){
			obj->printInformation();
		}
			
	}
	// pthread_mutex_unlock(&count_Mutex);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    /* matrix multiplication */
	for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++) {
		for (int j = 0 ; j < obj->_matrixSize; j++) {
			obj->multiResult[i][j] = 0;
			for (int k = 0 ; k < obj->_matrixSize; k++) {
				obj->multiResult[i][j] += obj->matrix[i][k] * obj->matrix[k][j];
            }	

	        /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
            // Observe the thread migration
			if(PART !=3){
				// pthread_mutex_lock(&count_Mutex);
				if(obj->core != sched_getcpu() & PART ==1){
					printf("The thread %d PID %d is moved from CPU %d to %d\n",obj->_ID,obj->PID,obj->core,sched_getcpu());
					obj->core =sched_getcpu();
				}
				// pthread_mutex_unlock(&count_Mutex);
			}


	        /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

		}
#if (PART == 3)
	    /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
        // Obaserve the execute thread on core-0
		// pthread_mutex_lock(&count_Mutex);
		
		if(obj->core ==0 && current_PID == -1){
			printf("Core0 start PID-%d\n",obj->PID);
			current_PID =syscall(SYS_gettid);
		}
		// else if (current_PID != obj->PID && obj->core ==0 ){
		else if (current_PID != syscall(SYS_gettid) && obj->core ==0 ){
			printf("Core:%d context switch from PID-%d to PID-%d\n",obj->core,current_PID,obj->PID);
			current_PID = syscall(SYS_gettid);
		}
		// pthread_mutex_unlock(&count_Mutex);
	    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif
	}

	pthread_mutex_lock( &count_Mutex );
    obj->check->checkCorrectness();
	pthread_mutex_unlock( &count_Mutex );
    return 0;
}


/*
 * Print out the tread information.
 *
 */
void
Thread::printInformation()
{
    std::cout << "Thread ID : " << _ID ;
    std::cout << "\tPID : " << PID;
    std::cout << "\tCore : " << core;
#if (PART != 1)
    std::cout << "\tUtilization : " << _utilization;
    std::cout << "\tMatrixSize : " << _matrixSize;	
#endif
    std::cout << std::endl;
}


/* 
 * Set up the scheduler for current thread.
 *
 * @ Part3 Recommendation:
 * Using function sched_setscheduler to set up the scheduling
 * policy, or other method.
 *
 */
void
Thread::setUpScheduler()
{
	/*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
    // Set up the scheduler for current thread
	struct sched_param sp;
	// printf("ID:%d,_schedulingPolicy:%d\n",PID,_schedulingPolicy);
	sp.sched_priority = sched_get_priority_max(_schedulingPolicy);
	sched_setscheduler(0,_schedulingPolicy,&sp);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
}
