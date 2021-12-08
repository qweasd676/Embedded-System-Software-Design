#include "system.h"
int current_PID;
/**
 * Set up the Set and threadSet dependent on the inputfile.
 * 
 * @ Part1 Recommendation:
 * Determin the which portion of matrix calculate by which thread using the
 * Start_Calculate_Point and End_Calculate_Point function in Thread.
 *
 * @ Part3 Recommendation:
 * Determine the scheduling policy for each thread by using function 
 * Thread::setSchedulingPolicy.
 * 
 */
System::System(char* input_file)
{
    loadInput(input_file); // Set up threadSet, singleResult, multiResult, and matrix

    for (int i = 0; i<numThread; i++) {
#if (PART == 1)
        // Set the singleResult, multResult, and matrix to thread.
		threadSet[i].initialThread(singleResult[0], multiResult[0], matrix[0]);
        /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
        // Set up the calculate range of matrix.
        threadSet[i].setStartCalculatePoint(i*threadSet[i].matrixSize()/numThread);
        threadSet[i].setEndCalculatePoint((i+1)*threadSet[i].matrixSize()/numThread);
	    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
        // Set the singleResult, multResult, and matrix to thread.
		threadSet[i].initialThread(singleResult[i], multiResult[i], matrix[i]);
#endif

#if (PART == 3)
	    /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
        // Set the scheduling policy for thread.
        threadSet[i].setSchedulingPolicy(SCHEDULING);
        // threadSet[i].setUpScheduler();
	    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif

    }

    /* CPU Initialize */
    cpuSet = new CPU[ CORE_NUM ];
    for (int i = 0; i < CORE_NUM; i++)
		cpuSet[i].createCPU( numThread, i );

    /* Set up data for checking correctness */
    check = new Check;
    check->initialCheck(singleResult, multiResult, numThread, CORE_NUM); 
    for (int i = 0; i < numThread; i++) {
        check->setThreadWithIndex (i, &threadSet[i].pthreadThread);
        check->setMatrixSizeWithIndex (i, threadSet[i].matrixSize());
        threadSet[i].setCheck(check);
    }

#if (PART == 3)
    if (SCHEDULING == SCHED_FIFO) {
        check->setCheckState(PARTITION_FIFO);
    } else if (SCHEDULING == SCHED_RR) {
        check->setCheckState(PARTITION_RR);
    } else {
        std::cout << "!! Not supported scheduler !!" << std::endl;
        assert(false);
    }
#endif
}


/*
 * Load the input file from command line argument.
 * Fetch out number of thread and widht/hight of matrix. Create the
 * matrix, singleResult and multiResult depend on the matrix size
 * declare by input file.
 *
 * @ input_file is the .txt in ./input
 *
 */
void
System::loadInput(char* input_file)
{
    std::ifstream myfile(input_file);
    std::string line;
	int read_matrix_size = 0;

	if (myfile.is_open()) {

		getline (myfile,line); // Get number of thread
		numThread = atoi(line.c_str());		

        std::cout << "Input File Name : " << input_file << std::endl;
        std::cout << "numThread : " << numThread << std::endl;

		threadSet = new Thread[numThread];
        singleResult = new float**[numThread];
        multiResult = new float**[numThread];
        matrix = new float**[numThread];

		for (int i = 0; i<numThread; i++) {
			getline(myfile,line); // Get teh width and height of matrix
			read_matrix_size = atoi(line.c_str());
			threadSet[i].setThreadID(i);
			threadSet[i].setThreadMatrixSize(read_matrix_size);			

            setUpMatrix(i, read_matrix_size); // Inital singleResult, multiResult, and matrix
		}

		myfile.close();

	} else {	

        std::cout << "Input file not found" << std::endl;
        assert(false);

	}
}


/*
 * Set up 2-D matrix for singleResult, multiResult
 * and matrix as the size of matrix_size * matrix_size.
 * Initial the value in martix
 *
 * @ thread_id is a index of initial matrix
 * @ matrix_size is a width and height of matrix
 *
 */
void
System::setUpMatrix(int thread_id, int matrix_size)
{
	singleResult[thread_id] = new float*[matrix_size];
	for (int i = 0; i < matrix_size; i++) {
		singleResult[thread_id][i] = new float[matrix_size];
	}

	multiResult[thread_id] = new float*[matrix_size];
	for (int i = 0; i < matrix_size; i++) {
		multiResult[thread_id][i] = new float[matrix_size];
	}

	//Initial Matrix Value
	matrix[thread_id] = new float*[matrix_size];
	for (int i = 0; i < matrix_size; i++) {
		matrix[thread_id][i] = new float[matrix_size];
		for (int j = 0; j < matrix_size; j++) {
			matrix[thread_id][i][j] = ((float) rand()/ (RAND_MAX));
		}
	}

}


/*
 * Execute the single thread matrix multiplication and store the result
 * at singleResult. For Part1, there is only one matrix. For Part2 and
 * Part3 each thread have their own matrix.
 *
 */
void
System::singleCoreMatrixMulti()
{
    std::cout << "\n===========Start Single Thread Matrix Multiplication===========" << std::endl; 
    setStartTime();

#if (PART == 1)
	threadSet[0].singleMatrixMultiplication();
#else
	for (int i = 0; i<numThread; i++)
		threadSet[i].singleMatrixMultiplication();
#endif

    setEndTime();
    std::cout << "Single Thread Spend time : " << _timeUse << std::endl;
}


/*
 * Clean up the multil result matrix.
 *
 */
void
System::cleanMultiResult()
{
    int matrix_size = 0;
    for (int thread_index = 0; thread_index < numThread; thread_index++ ) {
        matrix_size = threadSet[thread_index].matrixSize();

        for (int row_index = 0; row_index < matrix_size; row_index++) {

            for (int col_index = 0; col_index < matrix_size; col_index++) {
			    multiResult[thread_index][row_index][col_index] = 0;				
            }
        }
    }
}


/*
 * Creating the thread for matrix multiplication, each thread did not pinned
 * on the specifc core (e.g. Global scheduling).
 *
 * @ Part1 Recommendation:
 * Using the pthread_create and pthread_join in the empty block to create
 * Global multi-thread matrix multiplication.
 *
 */
void
System::globalMultiCoreMatrixMulti()
{
    std::cout << "\n===========Start Global Multi-Thread Matrix Multiplication===========" << std::endl; 
    check->setCheckState(GLOBAL);
    setStartTime();

	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Create thread and join
    pthread_create(&threadSet[0].pthreadThread,NULL, threadSet[0].matrixMultiplication,&threadSet[0]);
    pthread_create(&threadSet[1].pthreadThread,NULL, threadSet[1].matrixMultiplication,&threadSet[1]);
    pthread_create(&threadSet[2].pthreadThread,NULL, threadSet[2].matrixMultiplication,&threadSet[2]);
    pthread_create(&threadSet[3].pthreadThread,NULL, threadSet[3].matrixMultiplication,&threadSet[3]);
    pthread_join(threadSet[0].pthreadThread,NULL);
    pthread_join(threadSet[1].pthreadThread,NULL);
    pthread_join(threadSet[2].pthreadThread,NULL);
    pthread_join(threadSet[3].pthreadThread,NULL);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    setEndTime();
    std::cout << "Global Multi Thread Spend time : " << _timeUse << std::endl;
    cleanMultiResult();
}


/*
 * Creating the thread for matrix multiplication, each thread has pinned
 * on the specifc core.
 *
 * @ Part1 Recommendation:
 * Implement the pthread_create and pthread_join in the empty block. And
 * pinned four thread to four different core.
 *
 */

void
System::partitionMultiCoreMatrixMulti()
{
    current_PID = -1;
#if (PART == 1)
    std::cout << "\n===========Start Partition Multi-Thread Matrix Multiplication===========" << std::endl; 
    check->setCheckState(PARTITION);
#endif
    setStartTime();

	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Set thread execute core.
    // Create thread and join.
    if(PART == 1){
        threadSet[0].setThreadCore(0);
        threadSet[1].setThreadCore(1);
        threadSet[2].setThreadCore(2);
        threadSet[3].setThreadCore(3);
        pthread_create(&threadSet[0].pthreadThread,NULL, threadSet[0].matrixMultiplication,&threadSet[0]);
        pthread_create(&threadSet[1].pthreadThread,NULL, threadSet[1].matrixMultiplication,&threadSet[1]);
        pthread_create(&threadSet[2].pthreadThread,NULL, threadSet[2].matrixMultiplication,&threadSet[2]);
        pthread_create(&threadSet[3].pthreadThread,NULL, threadSet[3].matrixMultiplication,&threadSet[3]);
        pthread_join(threadSet[0].pthreadThread,NULL);
        pthread_join(threadSet[1].pthreadThread,NULL);
        pthread_join(threadSet[2].pthreadThread,NULL);
        pthread_join(threadSet[3].pthreadThread,NULL);
    }
    else{
        for(int i = 0;i<numThread;++i){
            pthread_create(&threadSet[i].pthreadThread,NULL,threadSet[i].matrixMultiplication,&threadSet[i]);

        }
        for(int j = 0;j<numThread;++j){

            pthread_join(threadSet[j].pthreadThread,NULL);
        }
    }  

	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    setEndTime();
    std::cout << "Partition Multi Thread Spend time : " << _timeUse << std::endl;
    cleanMultiResult();
}


/*
 * Using the First-Fit method to determine which core should be pinned on for
 * each thread. After finishing partition then call partitionMultiCoreMatrixMulti
 * which implement in Part1 to start the matrix multiplication.
 *
 * @ Part2 Recommendation:
 * Implement the partition First-Fit in the empty block below. Recommend using
 * pre-define class CPU which already support the print funciton to print out
 * the partition result.
 *
 */
void
System::partitionFirstFit()
{
    std::cout << "\n===========Partition First-Fit Multi Thread Matrix Multiplication===========" << std::endl;
#if (PART == 2)
    check->setCheckState(PARTITION_FF);
#endif

	for (int i = 0; i < CORE_NUM; i++)
		cpuSet[i].emptyCPU(); // Reset the CPU set

    
	/*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement parititon first-fit and print result.
    
    for(int cpu_number = 0; cpu_number<CORE_NUM;++cpu_number){
        for(int k = 0; k<numThread;++k){
            // printf("--%f--\n",cpuSet[cpu_number].utilization());
            if(cpuSet[cpu_number].utilization()+threadSet[k].utilization() <=1 & threadSet[k].setCore_()==-1){
                cpuSet[cpu_number].pushThreadToCPU(&threadSet[k]);
                threadSet[k].setThreadCore(cpu_number);
                // printf("utilization:%f,thread:ID:%d,cpuID:%d\n",cpuSet[cpu_number].utilization(),k,cpu_number);
            }      
        }  
    }
    for(int x = 0; x<numThread;++x){
        if(threadSet[x].setCore_()==-1){
            printf("Thread-%d not schedulable.\n",x);
        }
    }
    pthread_mutex_lock(&count_Mutex);
    for(int y = 0;y<CORE_NUM;++y){

        cpuSet[y].printCPUInformation();
    }
    pthread_mutex_unlock(&count_Mutex);
 
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    partitionMultiCoreMatrixMulti(); // Create the multi-thread matrix multiplication
}


/*
 * Using the Best-Fit method to determine which core should be pinned on for
 * each thread. After finishing partition then call partitionMultiCoreMatrixMulti
 * which implement in Part1 to start the matrix multiplication.
 *
 * @ Part2 Recommendation:
 * Implement the partition Best-Fit in the empty block below. Recommend using
 * pre-define class CPU which already support the print funciton to print out
 * the partition result.
 *
 */
void
System::partitionBestFit()
{
    std::cout << "\n===========Partition Best-Fit Multi Thread Matrix Multiplication===========" << std::endl;
    float totoal;
    float bestfit_valuse = 0;
    int thread_ID,cpu_ID;
#if (PART == 2)
    check->setCheckState(PARTITION_BF);
#endif

	for (int i = 0; i < CORE_NUM; i++){
		cpuSet[i].emptyCPU(); // Reset the CPU set
    }
	/*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement partition best-fit and print result.
    for(int j = 0 ; j<numThread;++j){
        for(int k = 0 ; k<CORE_NUM;++k){
            totoal = cpuSet[k].utilization() + threadSet[j].utilization();
            if(totoal <= 1 &  totoal> bestfit_valuse){
                bestfit_valuse = totoal;
                thread_ID = j;
                cpu_ID = k;
            }
        }
        if(thread_ID !=-1 & cpu_ID !=-1){
            cpuSet[cpu_ID].pushThreadToCPU(&threadSet[thread_ID]);
            threadSet[thread_ID].setThreadCore(cpu_ID);
            thread_ID =-1;
            cpu_ID = -1;
            bestfit_valuse = 0;
        }

    }
    for(int x = 0; x<numThread;++x){
        if(threadSet[x].setCore_()==-1){
            printf("Thread-%d not schedulable.\n",x);
        }
    }
    pthread_mutex_lock(&count_Mutex);
    for(int y = 0;y<CORE_NUM;++y){

        cpuSet[y].printCPUInformation();
    }
    pthread_mutex_unlock(&count_Mutex);
 
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    partitionMultiCoreMatrixMulti(); // Create the multi-thread matrix multiplication
}


/*
 * Using the Worst-Fit method to determine which core should be pinned on for
 * each thread. After finishing partition then call partitionMultiCoreMatrixMulti
 * which implement in Part1 to start the matrix multiplication.
 *
 * @ Part2 Recommendation:
 * Implement the partition Worst-Fit in the empty block below. Recommend using
 * pre-define class CPU which already support the print funciton to print out
 * the partition result.
 *
 */
void
System::partitionWorstFit()
{
    
    std::cout << "\n===========Partition Worst-Fit Multi Thread Matrix Multiplication===========" << std::endl;
    float totoal;
    float bestfit_valuse = 1;
    int thread_ID,cpu_ID;
#if (PART == 2)
    check->setCheckState(PARTITION_WF);
#endif

	for (int i = 0; i < CORE_NUM; i++)
		cpuSet[i].emptyCPU();
	
	/*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement partition worst-fit and print result.
    for(int j = 0 ; j<numThread;++j){
        for(int k = 0 ; k<CORE_NUM;++k){
            totoal = cpuSet[k].utilization() + threadSet[j].utilization();
            if(totoal <= 1 &  totoal< bestfit_valuse){
                bestfit_valuse = totoal;
                thread_ID = j;
                cpu_ID = k;
            }
        }
        if(thread_ID !=-1 & cpu_ID !=-1){
            cpuSet[cpu_ID].pushThreadToCPU(&threadSet[thread_ID]);
            threadSet[thread_ID].setThreadCore(cpu_ID);
            thread_ID =-1;
            cpu_ID = -1;
            bestfit_valuse = 1;
        }

    }
    for(int x = 0; x<numThread;++x){
        if(threadSet[x].setCore_()==-1){
            printf("Thread-%d not schedulable.\n",x);
        }
    }
    pthread_mutex_lock(&count_Mutex);
    for(int y = 0;y<CORE_NUM;++y){

        cpuSet[y].printCPUInformation();
    }
    pthread_mutex_unlock(&count_Mutex);
 
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    partitionMultiCoreMatrixMulti(); // Create the multi-thread matrix multiplication
}


/*
 * Set the current time as start time.
 *
 */
void
System::setStartTime()
{
	gettimeofday(&start, NULL);
}


/*
 * Set the current time as end time. Calculate the time interval between start 
 * time and end time.
 *
 */
void
System::setEndTime()
{
	gettimeofday(&end, NULL);
	_timeUse = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
}
