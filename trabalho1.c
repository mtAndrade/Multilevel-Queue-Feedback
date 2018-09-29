#include<stdio.h> 
#include<stdlib.h>
#include<string.h>

#define INITIALIZER 50000

int nProcess = INITIALIZER;

typedef struct 
{ 
      int pid, burst_time, priority;
      int total_time, arrival_time, ready, io_time;      
      int type;
      int time_until_next_io_request;
}process_structure; 

process_structure high_queue[INITIALIZER]; 
process_structure low_queue[INITIALIZER]; 
process_structure io_queue[INITIALIZER]; 
process_structure ended_queue[INITIALIZER]; 

process_structure register_queue[INITIALIZER]; 

int last_pid = 0;
int quantum = 0; 
int swap_time = 1;
int total_system_time = 0;
int last_arrival = 0;
int ended_process = 0;
int swaps = 0;

//Time that the IO takes to resolve - We are considering only one IO at a time
int disk_time = 5;
int print_time = 1;
int mag_time = 10;

// "Pointers" to avoid dealing with pointers
int low_first = 0;
int low_last = 0;
int low_np = 0;

int high_first = 0;
int high_last = 0;
int high_np = 0;
int search_ready = 0;

int io_first = 0;
int io_last = 0;
int io_np =0;

int firstIo = 1;
int firstLow = 1;

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

int GetIoOperationTime(int type){
	if (type == 1){
		return disk_time;
	}
	else if(type == 2){
		return mag_time;
	}
	else if(type == 3){
		return print_time;
	}
}

void ShowIoOperation(int type, int pid){
	if (type == 1)
	{
		printf("Process IO - DISCO: %d\n", pid);		
	}
	else if(type == 2){
		printf("Process IO - MAG: %d\n", pid);
	}
	else if(type == 3){
		printf("Process IO - PRINT: %d\n", pid);
	}
	printf("Running IO\n");
}

void RegisterType(int count){
	char typedMessage[INITIALIZER];
	
	printf("\n--New Process--\n");	
    printf("Process type (cpu - disk - mag - printer): \t");
    scanf("%s", typedMessage);

    register_queue[count].time_until_next_io_request = 0;
    register_queue[count].total_time = 0;
    register_queue[count].io_time = 0;
	register_queue[count].ready =  0;

    last_arrival += rand_interval(1,20);

    if(strcmp(typedMessage, "cpu") == 0){    	
    	printf("Cpu selected\n");
       	register_queue[count].type = 0;
		register_queue[count].burst_time = rand_interval(1,50);
		register_queue[count].priority = 0;
		register_queue[count].arrival_time = last_arrival;
    }
    else if(strcmp(typedMessage, "disk") == 0){
    	printf("DISK selected\n");
        register_queue[count].type = 1;
        register_queue[count].burst_time = rand_interval(1,50);
        register_queue[count].priority = 0;
        register_queue[count].time_until_next_io_request = rand_interval(1, register_queue[count].burst_time -1);
        register_queue[count].arrival_time = last_arrival;
    }
    else if(strcmp(typedMessage, "mag") == 0){
    	printf("MAG selected\n");
        register_queue[count].type = 2;
        register_queue[count].burst_time = rand_interval(1,50);
        register_queue[count].priority = 1;
        register_queue[count].time_until_next_io_request = rand_interval(1, register_queue[count].burst_time -1);
        register_queue[count].arrival_time = last_arrival;
    }
    else if(strcmp(typedMessage, "printer") == 0){
    	printf("PRINT selected\n");
        register_queue[count].type = 3;
        register_queue[count].burst_time = rand_interval(1,50);
        register_queue[count].priority = 1;
        register_queue[count].time_until_next_io_request = rand_interval(1, register_queue[count].burst_time -1);
        register_queue[count].arrival_time = last_arrival;
    }
}

void ReceiveList(){
	printf("\nRegister the number of process (<100) (pids will be automatically assigned):\t"); 
  	scanf("%d", &nProcess);  

  	printf("Register quantum:\t"); 
  	scanf("%d", &quantum);

  	printf("Register the swap duration:\t"); 
  	scanf("%d", &swap_time);

	for(int count = 0; count < nProcess; count++) 
	{ 
		last_pid += 1;
		RegisterType(count);			
		register_queue[count].pid = last_pid;
	}
	high_np = nProcess;
}

int ProcessHighQueue(){
	//This means nobody to process
	if(high_first > high_last) return -1;

	process_structure p = high_queue[high_first];

	//This will check if the next process in queue has arrived
	if(p.arrival_time <= total_system_time || p.ready){
		p.ready = 1;
		search_ready = 0;
	}else{
		printf("Looking for high ready process\n"); 
		high_last +=1;
		high_queue[high_last] = p;
		high_first +=1;

		search_ready += 1;
		return search_ready;		
	}

	high_np -=1;
	printf("Loading high priority process: %d\n", p.pid);
		
	if (p.type == 0)
	{		
		printf("Running CPU process: %d\n", p.pid);
		p.burst_time = p.burst_time - quantum;
		p.total_time = p.total_time + quantum;
		total_system_time += quantum;

		//This means the process is over
		if (p.burst_time <= 0)
		{
			printf("Process: %d -- Ended\n", p.pid);
			high_first += 1;

			ended_queue[ended_process] = p;

			ended_process +=1;
		}
		//Process not over, so place this process in the end of low queue (Preempção)
		else{
			printf("Process: %d -- Moved to LowQueue end\n", p.pid);
			printf("Swap time\n\n");
			swaps +=1;
			total_system_time += swap_time;

			high_first += 1;

			if (firstLow)
			{
				firstLow = 0;
				low_queue[low_first] = p;
			}else{
				low_last +=1;
				low_queue[low_last] = p;	
			}
		}		
	}else{
		printf("Running IO process: %d\n", p.pid);

		//Check if IO operation is needed before end of quantum
		if (p.time_until_next_io_request <= quantum)
		{
			printf("IO operation requested -- Changing Context\n");		
			if (firstIo)
			{
				total_system_time += p.time_until_next_io_request;
				p.total_time += p.time_until_next_io_request;
				firstIo = 0;
				io_queue[io_first] = p;				

			}else{
				io_last +=1;
				io_queue[io_last] = p;				
			}	
			high_first +=1;
		}else{
			p.time_until_next_io_request -= quantum;

			p.burst_time = p.burst_time - quantum;
			p.total_time = p.total_time + quantum;
			total_system_time += quantum;

			//This means the process is over
			if (p.burst_time <= 0)
			{
				printf("Process: %d -- Ended\n", p.pid);
				high_first += 1;
				ended_queue[ended_process] = p;
				ended_process +=1;
			}
			//Process not over, so place this process in the end of low queue (Preempção)
			else{
				printf("Process: %d -- Moved to LowQueue end\n", p.pid);
				printf("Swap time\n\n");
				swaps +=1;
				total_system_time += swap_time;

				high_first += 1;

				if (firstLow)
				{
					firstLow = 0;
					low_queue[low_first] = p;
				}else{
					low_last +=1;
					low_queue[low_last] = p;	
				}
			}	
		}			
	}
}

int ProcessLowQueue(){
	//This means nobody to process
	if(low_first > low_last) return -1;

	if (firstLow) return 0;

	total_system_time += swap_time;

	process_structure p = low_queue[low_first];
	printf("Loading low priority process: %d\n", p.pid);
		
	if (p.type == 0)
	{		
		printf("Running CPU process: %d\n", p.pid);
		p.burst_time = p.burst_time - quantum;
		p.total_time = p.total_time + quantum;
		total_system_time += quantum;

		//This means the process is over
		if (p.burst_time <= 0)
		{
			printf("Process: %d -- Ended\n", p.pid);
			low_first += 1;
			ended_queue[ended_process] = p;
			ended_process +=1;
		}
		//Process not over, so place this process in the end
		else{
			printf("Process: %d -- Moved to End\n", p.pid);
			low_first += 1;
			low_last +=1;
			low_queue[low_last] = p;
		}
	}else{
		printf("Running IO process: %d\n", p.pid);

		//Check if IO operation is needed before end of quantum
		if (p.time_until_next_io_request <= quantum)
		{
			printf("IO operation requested -- Changing Context\n");		
			if (firstIo)
			{
				total_system_time += p.time_until_next_io_request;
				p.total_time += p.time_until_next_io_request;
				firstIo = 0;
				io_queue[io_first] = p;				

			}else{
				io_last +=1;
				io_queue[io_last] = p;				
			}	
			low_first +=1;
		}else{
			p.time_until_next_io_request -= quantum;

			p.burst_time = p.burst_time - quantum;
			p.total_time = p.total_time + quantum;
			total_system_time += quantum;

			//This means the process is over
			if (p.burst_time <= 0)
			{
				printf("Process: %d -- Ended\n", p.pid);
				low_first += 1;
				ended_queue[ended_process] = p;
				ended_process +=1;
			}
			//Process not over, so place this process in the end
			else{
				printf("Process: %d -- Moved to LowQueue end\n", p.pid);
				low_first += 1;
				low_last +=1;
				low_queue[low_last] = p;
			}
		}			
	}	
}

int ProcessIoQueue(){
	//This means nobody to process
	if(io_first > io_last) return -1;

	if (firstIo) return 0;

	process_structure p = io_queue[io_first];
	
	ShowIoOperation(p.type, p.pid);
	p.io_time += GetIoOperationTime(p.type);
	p.time_until_next_io_request += rand_interval(1,10);
		
	if (p.type == 1)
	{		
		printf("Process: %d -- Moved to LowQueue end\n", p.pid);
		printf("Swap time\n\n");
		swaps +=1;
		total_system_time += swap_time;

		io_first += 1;

		if (firstLow)
		{
			firstLow = 0;
			low_queue[low_first] = p;
		}else{
			low_last +=1;
			low_queue[low_last] = p;	
		}


	}else{
		printf("Process: %d -- Moved to HighQueue end\n", p.pid);
		printf("Swap time\n\n");
		swaps +=1;
		total_system_time += swap_time;

		io_first += 1;

		high_last +=1;
		high_queue[high_last] = p;					
	}	
}

void InitializeQueues(){
	int high_index = 0;

	for(int count = 0; count < nProcess; count++){
		high_queue[high_index] = register_queue[count];
		high_index += 1;
	}

	high_last = high_index-1;
}

void PrintResults(){	
	printf("\n\n--Results--\n\n");
	printf("Total total_system_time: %d", total_system_time);
	printf("Number of Swaps:%d\n", swaps * swap_time );
	
	for(int i = 0 ; i< nProcess; i++){
		process_structure p = ended_queue[i];

		printf("PID:\t%d\t| ", p.pid );
		printf("Type:\t%d\t| ", p.type );
		printf("Arrival:\t%d\t| ", p.arrival_time );
		printf("IO Time:\t%d\t| ", p.io_time );
		printf("Total Time:\t%d\t| ", p.total_time );
		printf("\n\n");
	}
}

int main() 
{ 
	ReceiveList();
	InitializeQueues();
	
	while(1){
		int res = ProcessHighQueue();
		if(res >= high_np  && ended_process < nProcess){
			//This means that the current project did not arrive		
			printf("No High priority process is ready. Check for low priority\n");
			int res_low = ProcessLowQueue();
			if (res_low == 0){
				printf("No process is ready. Waiting for process\n");
				total_system_time += 5;
			} 	
		}
		else if (res == -1)
		{
			int res_low = ProcessLowQueue();
			if (res_low == -1) break;
		}
		ProcessIoQueue();
		
		int holt;
		//scanf("%d", &holt); 
	}

	PrintResults();

	return 0;
}