#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_STUDENTS 10
#define NUM_CHAIRS 3

sem_t student_sem;
sem_t tutor_sem;
pthread_mutex_t chair_mutex;
int waiting_students = 0;
int served_students = 0;
int chair_count = NUM_CHAIRS;

void *student(void *arg)
{
    int id = *(int *)arg;
    usleep(rand() % 1000000);
    pthread_mutex_lock(&chair_mutex);
    if (chair_count > 0)
    {
        chair_count--;
        waiting_students++;
        printf("Student %d started waiting for consultation\n", id);
        printf("Number of students now waiting: %d\n", waiting_students);
        pthread_mutex_unlock(&chair_mutex);
        sem_post(&student_sem);
        sem_wait(&tutor_sem);
        printf("Student %d is getting consultation\n", id);
        usleep(rand() % 500000);
        printf("Student %d finished getting consultation and left\n", id);
        pthread_mutex_lock(&chair_mutex);
        served_students++;
        printf("Number of served students: %d\n", served_students);
        pthread_mutex_unlock(&chair_mutex);
    }
    else
    {
        printf("No chairs remaining in lobby. Student %d Leaving......\n", id);
        printf("Student %d finished getting consultation and left\n", id);
        pthread_mutex_lock(&chair_mutex);
        served_students++;
        printf("Number of served students: %d\n", served_students);
        pthread_mutex_unlock(&chair_mutex);
    }
    return NULL;
}

void *tutor(void *arg)
{
    while (served_students < NUM_STUDENTS)
    {
        sem_wait(&student_sem);
        pthread_mutex_lock(&chair_mutex);
        waiting_students--;
        chair_count++;
        printf("A waiting student started getting consultation\n");
        printf("Number of students now waiting: %d\n", waiting_students);
        printf("ST giving consultation\n");
        pthread_mutex_unlock(&chair_mutex);
        sem_post(&tutor_sem);
        usleep(rand() % 500000);
    }
    return NULL;
}

int main()
{
    pthread_t tutor_thread;
    pthread_t student_threads[NUM_STUDENTS];
    int student_ids[NUM_STUDENTS];
    srand(time(NULL));
    sem_init(&student_sem, 0, 0);
    sem_init(&tutor_sem, 0, 0);
    pthread_mutex_init(&chair_mutex, NULL);
    if (pthread_create(&tutor_thread, NULL, tutor, NULL))
    {
        printf("Failed to create tutor thread\n");
        return 1;
    }
    for (int i = 0; i < NUM_STUDENTS; i++)
    {
        student_ids[i] = i;
        if (pthread_create(&student_threads[i], NULL, student, &student_ids[i]))
        {
            printf("Failed to create student thread %d\n", i);
            return 1;
        }
    }
    for (int i = 0; i < NUM_STUDENTS; i++)
    {
        pthread_join(student_threads[i], NULL);
    }
    pthread_join(tutor_thread, NULL);
    sem_destroy(&student_sem);
    sem_destroy(&tutor_sem);
    pthread_mutex_destroy(&chair_mutex);
    return 0;
}
