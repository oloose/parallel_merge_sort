#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

////##############################
////UBUNG 4 - AUFGABE 7 ##########
////##############################
void MergeSort(int* mUnsortedList, int* mSortedList, int mStart, int mEnd);
void MergeSortParallel(int* mUnsortedList, int* mSortedList, int mStart, int mEnd);
void MergeParallel(int* mUnsortedList, int* mSortedList, int mStart, int mMid, int mEnd);

int count = 0;
const int iterations = 1;
int numberCount = 10;
int main(int argc, char *argv[]) {
	int procs = 4;
	if (argc > 1) {
		procs = atoi(argv[1]);
	}
	printf("Anzahl Prozessoren gesamt: %d\n", omp_get_num_procs());
	printf("Anzahl der thread festgelegt auf: %d\n", procs);
	omp_set_num_threads(procs);

	if (argc > 2)
		numberCount = atoi(argv[2]);

	// variables
	long i;
	double clock_start, clock_stop, wtime, avgTime;
	int* unsortedList = new int[numberCount];

	//##########################################
	// Sequentiell #############################
	//##########################################
	int* tmpList = new int[numberCount];
	//add random numbers to unsortedList
	for (i = 0; i < numberCount; i++) {
		unsortedList[i] = rand() % 1001;
	}

	for (int k = 0; k <= numberCount; k++) {
		printf("%d\n", unsortedList[k]);
	}
	printf("##################\n\n");

	for (i = 0; i < iterations; i++) {
		clock_start = omp_get_wtime(); // Start time

		MergeSort(unsortedList, tmpList, 0, numberCount - 1);

		clock_stop = omp_get_wtime(); //finish time
		wtime = clock_stop - clock_start;
		printf("Sequentiell Time %14f\n", wtime);
		avgTime += (clock_stop - clock_start);
	}
	//for (i = 0; i < numberCount; i++) {
	//	printf("%d\n", unsortedList[i]);
	//}

	printf("\nAvg Sequentiell Time %14f\n", avgTime / iterations);
	avgTime = 0;

	//##########################################
	// OpenMP ##################################
	//##########################################
	//int* sortList = new int[numberCount];
	////add random numbers to unsortedList
	//for (i = 0; i < numberCount; i++) {
	//	unsortedList[i] = rand() % 1001;
	//}


	//for (i = 0; i < iterations; i++) {
	//	clock_start = omp_get_wtime(); // Start time

	//	#pragma omp parallel
	//	{
	//		#pragma omp single
	//		MergeSortParallel(unsortedList, sortList, 0, numberCount - 1);
	//	}


	//	clock_stop = omp_get_wtime(); //finish time
	//	wtime = clock_stop - clock_start;
	//	printf("Parallel Time %14f\n", wtime);
	//	avgTime += (clock_stop - clock_start);
	//}
	////for (i = 0; i < numberCount; i++) {
	////	printf("%d\n", sortList[i]);
	////}
	//printf("\nAvg nParallel Time %14f\n", avgTime / iterations);
	//avgTime = 0;
	return 0;
}

//mUnsortedList = original unsorted List
//tmpList = temporary sorted list
//mStart = start index of a sublist respective to start index of this list in the unsorted list
//mEnd = end index of a sublist respective to end index of this list in the unsorted list
void MergeSortParallel(int* mUnsortedList, int* mSortList, int mStart, int mEnd) {
	if (mEnd <= mStart) {
		//empty or single element
		return;
	}
	int mid = (mStart + mEnd) / 2;

	//only create task if work amount is sufficient (array size is (1 << 14))
	//For 100mio -> optimal value: 1<<13||14||15 for best result
	if (mEnd - mStart >= (1 << 14)) {
		#pragma omp taskgroup //use taskgroup to wait for inner task completion
		{
			//left side
			//do not bind task to cpu core to allow different threads to resume at task scheduling points
			#pragma omp task untied //if(mEnd-mStart >= (1<<14))
			MergeSortParallel(mUnsortedList, mSortList, mStart, mid);

			//right side 
			//#pragma omp task untied //if(mEnd-mStart >= (1<<14)) //dont use additional task to prevent exponential task creation
			MergeSortParallel(mUnsortedList, mSortList, mid + 1, mEnd);
			#pragma omp taskyield //suspend thread to allow execution thread to perform other work 
		}
	}
	else {
		MergeSortParallel(mUnsortedList, mSortList, mStart, mid);
		MergeSortParallel(mUnsortedList, mSortList, mid + 1, mEnd);
	}

	//#pragma omp taskwait //alternative to omp taskgroup, but slower
	MergeParallel(mUnsortedList, mSortList, mStart, mid, mEnd);
}

void MergeParallel(int* mUnsortedList, int* mSortList, int mStart, int mMid, int mEnd) {
	int left = mStart; // begin of the left side
	int right = mMid + 1; // begin of the right sub-array

	// sort and save in tmpList
	int i;

	//sort list and use tmpList as temporary save
	for (i = mStart; i <= mEnd; i++) {
		if (left == mMid + 1) {
			// left pointer has reached the limit
			mSortList[i] = mUnsortedList[right];
			right++;
		} else if (right == mEnd + 1) {
			// right pointer has reached the limit
			mSortList[i] = mUnsortedList[left];
			left++;
		} else if (mUnsortedList[left] < mUnsortedList[right]) {
			// pointer left points to smaller element       
			mSortList[i] = mUnsortedList[left];
			left++;
		} else {
			// pointer right points to smaller element      
			mSortList[i] = mUnsortedList[right];
			right++;
		}
	}

	//copy sorted tmpList to unsortedList
	for (int j = mStart; j <= mEnd; j++) {
		mUnsortedList[j] = mSortList[j];
	}	
}

//mUnsortedList = original unsorted List
//tmpList = temporary sorted list
//mStart = start index of a sublist respective to start index of this list in the unsorted list
//mEnd = end index of a sublist respective to end index of this list in the unsorted list
void MergeSort(int* mUnsortedList, int* mSortList, int mStart, int mEnd) {
	if (mEnd <= mStart) {
		//empty or single element
		return;
	}
	int mid = (mStart + mEnd) / 2;

	//left side
	MergeSort(mUnsortedList, mSortList, mStart, mid);
	//right side
	MergeSort(mUnsortedList, mSortList, mid + 1, mEnd);

	int pointer_left = mStart; // begin of the left side
	int pointer_right = mid + 1; // begin of the right sub-array

	//### MERGE
	// sort and save in tmpList
	int i;
	for (i = mStart; i <= mEnd; i++) {
		if (pointer_left == mid + 1) {
			// left has reached the limit
			mSortList[i] = mUnsortedList[pointer_right];
			pointer_right++;
		}
		else if (pointer_right == mEnd + 1) {
			// right has reached the limit
			mSortList[i] = mUnsortedList[pointer_left];
			pointer_left++;
		}
		else if (mUnsortedList[pointer_left] < mUnsortedList[pointer_right]) {
			// left points to smaller element       
			mSortList[i] = mUnsortedList[pointer_left];
			pointer_left++;
		}
		else {
			// right points to smaller element      
			mSortList[i] = mUnsortedList[pointer_right];
			pointer_right++;
		}
	}

	// copy sorted tmpList to unsortedList
	for (i = mStart; i <= mEnd; i++) {
		mUnsortedList[i] = mSortList[i];
	}
}