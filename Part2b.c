#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#define EXAM_QUESTIONS 5
#define RUBRIC_LINE 5

struct SharedMemory {
    char rubric[RUBRIC_LINE];   
    int currentExam;            
    int totalExams;             
    int completed;              
};

int sem_wait_op(int semid) {
    struct sembuf op = {0, -1, 0};
    return semop(semid, &op, 1);
}

int sem_post_op(int semid) {
    struct sembuf op = {0, 1, 0};
    return semop(semid, &op, 1);
}

void markingProcess(int id, struct SharedMemory *data, int semid_rubric, int semid_exam) {
    srand(time(NULL) + id); 

    while (1) {
        sem_wait_op(semid_exam);
        if (data->currentExam >= data->totalExams || data->completed) {
            sem_post_op(semid_exam);
            break;
        }
        int examId = data->currentExam;
        data->currentExam++;
        sem_post_op(semid_exam);

        printf("TA %d: Loading exam %d\n", id, examId + 1);

        //Changing/not changing rubric
        for (int j = 0; j < RUBRIC_LINE; j++) {
            usleep(500000 + rand() % 500000); //changing decision takes 0.5-1.0 seconds

            int yesChange = rand() % 2; //there is a 50% chance TA will change rubric
            if (yesChange) {   //making a change/correction 
                sem_wait_op(semid_rubric);

                char postChange = data->rubric[j];  //go to line to be changed
                data->rubric[j] = postChange + 1;  //replace with next ASCII code

                printf("TA %d: Corrected rubric line %d: %c -> %c\n", id, j + 1, postChange, data->rubric[j]);
                sem_post_op(semid_rubric);

            } else { //not making a change/correction
                printf("TA %d: Checked rubric line %d: %c\n", id, j + 1, data->rubric[j]);
            }
        }

        // mark questions
        for (int j = 0; j < EXAM_QUESTIONS; j++) {
            usleep(1000000 + rand() % 1000000); //takes 1-2 seconds for TA to mark
            printf("TA %d: Marked exam %d question %d\n", id, examId + 1, j + 1);
        }

        if (examId == data->totalExams - 1) {
            data->completed = 1;
        }
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {  //must have n >=2 processes running concurrently 
        printf("Not enough TAs n >=2 processe required");
        return 1;
    }

    int numTAs = atoi(argv[1]);
    int totalExams = atoi(argv[2]);
    if (numTAs < 2) numTAs = 2;

    // create shared memory
    int shmid = shmget(IPC_PRIVATE, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) { 
        perror("shmget"); 
        exit(1); 
    }
    struct SharedMemory *data = shmat(shmid, NULL, 0);

    data->currentExam = 0;
    data->completed = 0;
    data->totalExams = totalExams;
    for (int i = 0; i < RUBRIC_LINE; i++) {
        data->rubric[i] = 'A' + i;
    }

    // create semaphores
    int semid_rubric = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    int semid_exam = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun { 
        int val; 
    } arg;
    arg.val = 1;
    semctl(semid_rubric, 0, SETVAL, arg);
    semctl(semid_exam, 0, SETVAL, arg);

    // fork TA processes
    for (int i = 1; i <= numTAs; i++) {
        if (fork() == 0) {
            markingProcess(i, data, semid_rubric, semid_exam);
        }
    }

    // wait for all TAs
    for (int i = 0; i < numTAs; i++) {
        wait(NULL);
    }

    // cleanup
    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid_rubric, 0, IPC_RMID);
    semctl(semid_exam, 0, IPC_RMID);

    printf("All TAs have finished marking.\n");
    return 0;
}
