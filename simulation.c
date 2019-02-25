#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
typedef int bool;
enum {
	false, true
};

int nmax = -1;
int tmax = -1;
float pdealloc = -1;
float palloc = -1;
int jmax = -1;

int **storage;

int timeAmount = 0;
int currentId = 1;
int listSizeCount = 0;
float failures = 0;
float requests = 0;

typedef struct JobList JOB;

struct JobList {
	int id;
	int blockStart;
	int blockEnd;
	JOB *next;
};

/**
 * Creates a job for use within a JobList. A job represents a program in memory.
 */
JOB *create_JobList(int id, int blockStart, int blockEnd) {
	JOB *j = (JOB *) malloc(sizeof(JOB));
	j->id = id;
	j->blockStart = blockStart;
	j->blockEnd = blockEnd;
	j->next = NULL;
	return j;
}

/**
 * Inserts a job within the linked list. The linked list is sorted via the Job ID field, with the smallest ID coming first within the linked list.
 * *list - JobList to insert Job into.
 * *job - Job to be inserted into JobList.
 */
JOB *insertSorted(JOB *list, JOB *job) {
	if (list == NULL)
		return job;
	if (job->id < list->id) {
		job->next = job;
		listSizeCount++;
		return job;
	}
	list->next = insertSorted(list->next, job);
	return list;
}

/**
 * Prints information of all jobs held within the linked list.
 */
void traverse(JOB *list) {
	if (list == NULL)
		return;
	printf("ID: %d\tBlockStart: %d\tBlockEnd: %d\n", list->id, list->blockStart,
			list->blockEnd);
	traverse(list->next);
}

/**
 * Returns a job at specified index within the linked list.
 */
JOB * searchIndex(JOB *list, int index) {
	JOB *tmp = list;
	int i;
	for (i = 0; i <= index; i++) {
		if (i == index)
			return tmp;
		else
			tmp = tmp->next;
	}
	return NULL;
}

/**
 * Returns a job with specified id from within the linked list.
 */
JOB * search(JOB *list, int id) {
	if (list == NULL)
		return NULL;
	if (list->id == id)
		return list;
	return search(list->next, id);
}

/**
 * Deletes a job at specified index within the linked list, returns a copy of the deleted list.
 */
JOB * deleteIndex(JOB *list, int index) {
	JOB *tmp = list;
	if (list == NULL)
		return NULL;
	if (index == 0) {
		listSizeCount--;
		free(list);
		list = tmp->next;
		return list;
	}
	int i;
	for (i = 1; i <= index; i++) {
		if (i == index) {
			JOB *remove = tmp->next;
			listSizeCount--;
			//free(list->id);
			free(remove);
			tmp->next = remove->next;
			return remove;
		} else {
			tmp = list->next;
		}
	}
	return NULL;
}

/**
 * Deletes a job with specified id from within the linked list, returns a copy of the deleted list.
 */
JOB * delete(JOB *list, int id) {
	if (list == NULL)
		return NULL;
	if (list->id > id)
		return NULL;
	if (list->id == id) {
		JOB *tmp = list->next;
		free(list);
		return tmp;
	}
	list->next = delete(list->next, id);
	return list;
}

/**
 * Uses the firstfit algorithm to allocate memory to a program with specified blocksize.
 */
int firstFit(int *blocksize) {
	int i;
	bool memoryFound = false;
	int currentTest = -1;
	for (i = 0; i < nmax; i++) {
		if (storage[i] == NULL && currentTest == -1) {
			currentTest = i;
		} else if (storage[i] == NULL) {
			int test;
			test = i - currentTest;
			if (test >= *blocksize) {
				memoryFound = true;
				break;
			}
		}
		if (storage[i] != NULL) {
			currentTest = -1;
		}
	}
	if (memoryFound == true)
		return currentTest;
	else
		return -1;
}

/**
 * Uses the worstfit algorithm to allocate memory to a program with specified blocksize.
 */
int worstFit(int *blocksize) {
	int i;
	int memorySize = 0;
	int bestMemorySize = 0;
	int bestFound = -1;
	int currentTest = -1;
	for (i = 0; i < nmax; i++) {
		if (storage[i] == NULL && currentTest == -1) {
			if (bestFound == -1) {
				bestMemorySize++;
				bestFound = i;
			}
			currentTest = i;
			memorySize++;
		} else if (storage[i] == NULL) {
			memorySize++;
			if (memorySize > bestMemorySize) {
				bestFound = currentTest;
				bestMemorySize = memorySize;
			}
		}
		if (storage[i] != NULL && currentTest != -1) {
			currentTest = -1;
			memorySize = 0;
		}
	}
	if (bestMemorySize >= *blocksize)
		return bestFound;
	else
		return -1;
}

/**
 * Uses the bestfit algorithm to allocate memory to a program with specified blocksize.
 */
int bestFit(int *blocksize) {
	int i;
	int memorySize = 0;
	int bestMemorySize = 0;
	int bestFound = -1;
	int currentTest = -1;
	for (i = 0; i < nmax; i++) {
		if (storage[i] == NULL && currentTest == -1) {
			currentTest = i;
			memorySize++;
		} else if (storage[i] == NULL) {
			memorySize++;
		}
		if (storage[i] != NULL && currentTest != -1) {
			if (bestMemorySize == 0 && memorySize >= *blocksize) {
				bestFound = currentTest;
				bestMemorySize = memorySize;
			} else {
				int bestDiff = abs(*blocksize - bestMemorySize);
				int currentDiff = abs(*blocksize - memorySize);
				if (currentDiff < bestDiff && currentDiff >= *blocksize) {
					bestFound = currentTest;
					bestMemorySize = memorySize;
				}
			}
			currentTest = -1;
			memorySize = 0;
		}
	}
	//Nothing in memory yet.
	if (bestMemorySize == 0) {
		bestMemorySize = memorySize;
		bestFound = currentTest;
	}
	if (bestMemorySize >= *blocksize)
		return bestFound;
	else
		return -1;
}

/**
 * Prints the program ID held within each unit of memory storage.
 */
void printStorage() {
	int j;
	for (j = 0; j < nmax; j++) {
		if (storage[j] != NULL)
			printf("%d, ", *storage[j]);
		else
			printf("/0, ");

	}
	printf("\n");
}

/**
 * Deallocates any program ID associated in memory from start to end.
 */
void dealloc(int start, int end) {
	for (; start <= end; start++) {
		storage[start] = NULL;
	}
}

/**
 * Returns a float representing the percentage of memory empty.
 */
float memoryLeft() {
	int i;
	float filled = 0;
	float empty = 0;
	for (i = 0; i < nmax; i++) {
		if (storage[i] == NULL)
			empty++;
		else
			filled++;
	}
	return (filled / nmax);
}

int main(int argc, char **argv) {
	JOB *list = NULL;
	int opt;
	bool first = false;
	bool worst = false;
	bool best = false;

	////////////////////////////////////////////////////////////////////////
	//COMMAND LINE ARGUMENT PARSING BEGINS
	while ((opt = getopt(argc, argv, "fwbn:t:j:a:d:")) != -1) {
		switch (opt) {
		case 'f':
			if (best == true || worst == true) {
				fprintf(stderr, "Can only specify one algorithm to run");
				exit(EXIT_FAILURE);
			}
			first = true;
			break;
		case 'w':
			if (first == true || best == true) {
				fprintf(stderr, "Can only specify one algorithm to run");
				exit(EXIT_FAILURE);
			}
			worst = true;
			break;
		case 'b':
			if (first == true || worst == true) {
				fprintf(stderr, "Can only specify one algorithm to run");
				exit(EXIT_FAILURE);
			}
			best = true;
			break;
		case 'n':
			nmax = atoi(optarg);
			storage = malloc(sizeof(int *) * nmax);
			break;
		case 't':
			tmax = atoi(optarg);
			break;
		case 'j':
			jmax = atoi(optarg);
			break;
		case 'a':
			palloc = atof(optarg);
			break;
		case 'd':
			pdealloc = atof(optarg);
			break;
		default:
			fprintf(stderr, "Usage: %s [-ilw] [file...]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (nmax == -1 || tmax == -1 || pdealloc == -1 || palloc == -1
			|| jmax == -1) {
		fprintf(stderr,
				"Usage: simulate -<f|w|b>\n\t\t-n <number>\t-t <number>\t-j <number>\n\t\t-a <number>\t-d <number>\n\n");
		fprintf(stderr, "Where each letter represents the following:\n");
		fprintf(stderr,
				"\t\t-f = FirstFit\t-w = WorstFit\t-b = BestFit\n\t\t-n = NMAX\t-t = TMAX\t-j = JMAX\n\t\t-a = PALLOC\t-d = PDEALLOC\n");
		exit(EXIT_FAILURE);
	}
	//COMMAND LINE ARGUMENT PARSING ENDS
	////////////////////////////////////////////////////////////////////////
	srand ( time(NULL) );
	while (timeAmount < tmax) {
		//Deallocate occurs first to ensure that jobs added in the same time-step are not removed
		int r = rand() % 101;
		float f = r;
		float f2 = 100 * pdealloc;
		//Deallocate job if random value exceeds PDEALLOC and time has exceeded threshold.
		bool jobAdded = false;
		bool jobRemoved = false;
		JOB *removedJob;
		JOB *newJob;
		if (timeAmount > 100 && f < (f2) && listSizeCount > 0) {
			//Update boolean to show that a job was removed this time-step
			jobRemoved = true;
			//Chose random index of current jobs
			int jobRemoveTest = rand() % listSizeCount;
			//Find job associated with index
			removedJob = searchIndex(list, jobRemoveTest);
			//Deallocate memory associated with job
			dealloc(removedJob->blockStart, removedJob->blockEnd);
			//Delete the job from current jobs list and remove from memory
			list = delete(list, removedJob->id);
			//Decrease current job count
			listSizeCount--;
		}
		r = rand() % 101;
		float testFloat = r;
		float testFloat2 = 100 * palloc;
		//Create new job if random value exceeds PALLOC
		if (testFloat < testFloat2) {
			//Update boolean to show that a job was added this time-step
			jobAdded = true;
			//Non-uniform distribution
			//Calculate next random block size
			int blockSize = rand() % (jmax - 1 + 1) + 1;
			int startBlock;
			//Use first fit algorithm to find continuous data space
			if (first == true)
				startBlock = firstFit(&blockSize);
			//Use worst fit algorithm to find continuous data space
			if (worst == true)
				startBlock = worstFit(&blockSize);
			//Use best fit algorithm to find continuous data space
			if (best == true)
				startBlock = bestFit(&blockSize);
			//Increase request amount
			requests++;
			//Indicates not enough contiguous memory available
			if (startBlock == -1) {
				failures++;
			} else {
				int endBlock = startBlock + blockSize;
				int i;
				//Create new job
				newJob = create_JobList(currentId, startBlock, endBlock);
				//Assign memory to job
				for (i = startBlock; i <= endBlock; i++) {
					storage[i] = &newJob->id;
				}
				//Increase current job count
				listSizeCount++;
				//Add job to list of current jobs
				list = insertSorted(list, newJob);
				//Increase counter for program id
				currentId++;
			}
		}
		if (jobAdded == true && jobRemoved == true)
			printf(
					"TimeStep: %d\tEvent: Job allocated AND deallocated\tUnused memory: %.2f%\n",
					timeAmount, (1 - memoryLeft()) * 100);
		else if (jobAdded == true)
			printf("TimeStep: %d\tEvent: Job allocated\tUnused memory: %.2f%\n",
					timeAmount, (1 - memoryLeft()) * 100);
		else if (jobRemoved == true)
			printf(
					"TimeStep: %d\tEvent: Job deallocated\tUnused memory: %.2f%\n",
					timeAmount, (1 - memoryLeft()) * 100);
		else
			printf(
					"TimeStep: %d\tEvent: Neither job allocated or deallocated\tUnused memory: %.2f%\n",
					timeAmount, (1 - memoryLeft()) * 100);
		//Show memory storage for small values of NMAX
		if (nmax <= 16)
			printStorage();
		//Increase time unit measurement
		//printStorage();
		timeAmount++;
	}
	//Print failure rate
	float percent = (1 - memoryLeft()) * 100;
	printf("Requests: %f\n", requests);
	printf("Failures: %f\n", failures);
	if (failures > 0)
		printf("Failure rate: %.2f%\n", (failures/requests) * 100);
	else
		printf("Failure rate: 0%%");
}
