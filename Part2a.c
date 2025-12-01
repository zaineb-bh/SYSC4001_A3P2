#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#define EXAM_QUESTIONS 5
#define TOTAL_EXAMS 20
#define RUBRIC_LINE 5
#define TA_NAME_LENGTH 32

typedef struct {
    int studentNumber;                
    char rubric[RUBRIC_LINE][4];     
    char examFile[32];                
} SharedMemory;

// Non-hardcoded delay for marking
void timeDelay(double minTime, double maxTime) {
    double timeRange = maxTime - minTime;
    double delay = minTime + ((double)rand() / RAND_MAX) * timeRange;
    usleep((int)(delay * 1000000));   
}

void markingProcess(int taID, SharedMemory *shared, int totalExams) {
    
    srand(time(NULL) + getpid()); //so each TA has unique marking behaviour every time

    for (int i = 0; i < totalExams; i++) {
        int student = shared->studentNumber;

        if (student == 9999){
            break; //If student 9999 finish the processes, exam20.txt

        } 

        //Print marking status
        printf("TA %d: Loading %s for student %d\n", taID, shared->examFile, student);
        fflush(stdout); 

        //Changing/not changing rubric
        for (int j = 0; j < RUBRIC_LINE; j++) {
            timeDelay(0.5, 1.0); //changing decision takes 0.5-1.0 seconds

            int yesChange = rand() % 2; //there is a 50% chance TA will change rubric
            
            if (yesChange) {  //making a change/correction 
                char postChange = shared->rubric[j][2]; //go to line of rubric and letter
                shared->rubric[j][2] = postChange + 1; //replace with next ASCII code

                printf("TA %d: Correction made to rubric line %d: %c -> %c\n", taID, j + 1, postChange, shared->rubric[j][2]);

                //Save change to rubric file
                FILE *rubricFile = fopen("rubric.txt", "w");     //open file in write mode
                for (int line = 0; line < RUBRIC_LINE; line++)
                    fprintf(rubricFile, "%s\n", shared->rubric[line]);
                fclose(rubricFile);

            } else {   //not making a change/correction
                printf("TA %d: No correction needed on rubric line %d: %c\n", taID, j + 1, shared->rubric[j][2]);
            }
        }

        //for the Marking
        for (int j = 0; j < EXAM_QUESTIONS; j++) {
            timeDelay(1.0, 2.0);  //takes 1-2 seconds for TA to mark 
            printf("TA %d: Marked student %d question %d\n", taID, student, j + 1);
        }

        //Next student's exam
        shared->studentNumber++;
        sprintf(shared->examFile, "exam%d.txt", shared->studentNumber);

        //Stop when no more students left
        if (shared->studentNumber > totalExams || shared->studentNumber == 9999) {
            shared->studentNumber = 9999;
            break;

        }
    }

    printf("TA %d: All exams have been marked\n", taID);
    fflush(stdout);
    exit(0);
}

int main(int argc, char *argv[]) {

    if (argc < 3) {  //must have n >=2 processes running concurrently 
        printf("Not enough TAs n >=2 processe required");
        return 1;
    }

    //converts ASCII to integr
    int ta_count = atoi(argv[1]);     //convert number of TAs to int
    int allExams = atoi(argv[2]); //convert number of exams to int
    if (ta_count < 2) {
        ta_count = 2;   //if less than 2 TAs for to be 2
    }

    setbuf(stdout, NULL);

    // Allocate shared memory
    SharedMemory *shared = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    //set up rubric
    for (int i = 0; i < RUBRIC_LINE; i++)
        sprintf(shared->rubric[i], "%d,%c", i + 1, 'A' + i);

    //set up the first exam, exam1.txt
    shared->studentNumber = 1;
    sprintf(shared->examFile, "exam1.txt");

    //set TAs
    for (int i = 1; i <= ta_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            markingProcess(i, shared, allExams);
        }
    }

    //wait for TAs
    for (int i = 0; i < ta_count; i++) {
        wait(NULL);
    }

    printf("All TAs finished marking.\n");
    return 0;
}
