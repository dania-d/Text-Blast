/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Name:Dania Dababo
* Student ID:922001571
* GitHub ID:dania-d
* Project: Assignment 4 – Word Blast
*
* File: Dababo_Dania_HW4_main.c
*
* Description: A C program that reads War and Peace and counts 
* and tallies each word that is 6 or more characters long.
*
**************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";
// pair represents a word and its associated frequency found in a text file
struct pair {
    char * word;
    int freq;
}; 
int textFile;
int wordCount = 0; // keep track of words[] index
int buffSize; // size of each threads buffer
struct pair words[200000];  // array to hold tokens (words)
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // lock declaration 

// every thread begins execution at this function
void *entryPoint ( void *ptr) {
    // printf("hello from entry point\n");
    // allocate buffer for a thread
    char *buffer = malloc(buffSize + 100);  // + 100 to account for any remainder
    if (buffer == NULL) {   // malloc failed
        printf("Malloc failed to allocate memory.");
        return (void*) -1; 
    }
    // read up to buffSize bytes from textFile into buffer
    read (textFile, buffer, buffSize);  
    char *saveptr;  // maintain context between successive calls parsing same string
    char *token;  
    int duplicate = 0;  // flag to keep track of word being a duplicate or not
   
    token = strtok_r(buffer, delim, &saveptr); // get first word in file 
    while (token != NULL) {  // loop through buffer
        if (strlen(token) > 5) {    // word is 6 or more characters
            for(int i = 0; i < wordCount; i++){ // iterate pair words[] array
                // strcasecmp returns 0 when both parameters are equal
                if(strcasecmp(words[i].word, token) == 0){ 
                    pthread_mutex_lock(&lock);  // critical section
                    words[i].freq++;    // increase frequecnt of the found duplicate
                    pthread_mutex_unlock(&lock);    // end critical section
                    duplicate = 1;  // set flag (word is a duplicate)
                    break;  // don't need to continue iterating 
                } 
            }
            if (duplicate != 1){    // new instance of a word
                pthread_mutex_lock(&lock);	// start critical section
                words[wordCount].word = token;  // add the word to a pair
                words[wordCount].freq = 1;      // set initial frequency 
                wordCount++;    // increment so next pair can be at next index       
                pthread_mutex_unlock(&lock);
            }
           duplicate = 0;   // reset flag
           
        }
        token = strtok_r(NULL, delim, &saveptr);    // get next token
    }
    // uninizialize mutex lock 
    int x = pthread_mutex_destroy(&lock);
    if (x != 0){
        printf("pthread_mutex_destroy() failed [%d] \n", x);
    }
   return 0;
}   // end entryPoint()

int main (int argc, char *argv[]) {
    char * fileName;    // name of text file
    int threadCount;    // number of threads to be used
    int fileSize;     // size of file in bytes
    
    // check that argc[2] is not null 
    if (argc > 2) threadCount = atoi (argv[2]); // get thread count
    // check that argv[1] is not null
    if (argc > 1) fileName = argv[1];   // get name of file
    // open file; returns file-descriptor
    textFile = open (fileName, O_RDONLY);   
    if (textFile < 0) { //  on error, open() returns negative integer
        printf ("error opening file.");
        exit (-1);  
    }
    
    fileSize = lseek (textFile, 0, SEEK_END);  // get size of file
    // printf("length of text file in bytes: %d\n", fileSize);
    lseek (textFile, 0, SEEK_SET);   // file offset reset to beginning of file
   
    buffSize = (fileSize + 1) /  threadCount;   // set the length for each threads buffer
                                                // +1 accounts for rounding up
    
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************

    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish
    pthread_t threads[threadCount];  // array of threads
    for(int i = 0; i < threadCount; i++) {    // loops for each thread
        // start new thread in calling process
        // start execution at entryPoint()
        int returnVal = pthread_create (&threads[i], NULL, entryPoint, NULL);
        if (returnVal != 0) {   
            printf("error creating thread");
        }
    }
    
    // wait until threads terminate before returning to main
    for (int i = 0; i < threadCount; i++) {
        int x = pthread_join (threads[i], NULL);
        if(x){
            // zombies may have been created...
            printf("Join unsuccessful");
        }
    }
    // close text file
    if(close(textFile) != 0) printf("File did not close successfully");
    
    // ***TO DO *** Process TOP 10 and display
    //**************************************************************
    // sort list by frequency in descending order
    // for each array index i, exchange with the pairs that are 
    //smaller at index 0 to i-1
    for(int i = 0; i < wordCount; i++){ 
         struct pair temp = words[i];   
         int j = i;
        while ( j > 0 && (words[j-1].freq < temp.freq)){
            words[j] = words[j-1];
            j--;
        }
        words[j] = temp; 
    }
    printf("\nWord Frequency Count on %s with %d Threads\n",
         fileName, threadCount);
    // print the 10 words with the highest tallies in descending order
    for (int i = 0; i < 10; i++) {
        printf("Number %d is %s with a count of %d\n", (i+1), 
            words[i].word, words[i].freq);
    }
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }
    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //free(buffer);
    return 0;
    }
