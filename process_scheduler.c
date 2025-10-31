/*
 * ���̵����㷨ģ��ʵ��
 * ���ܣ�ʵ��FCFS��RR��SJF��SRTF������ռʽ���ȼ�����ռʽ���ȼ�6�ֵ����㷨
 * ��������ȹ�����־������ͼ����תʱ��/�ȴ�ʱ��ͳ��
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

 // ���̿��ƿ飨PCB���ṹ�嶨��
typedef struct PCB {
    int pid;             // ����ID
    int arrival;         // ����ʱ��
    int burst;           // ������ʱ�䣨CPU���䣩
    int rest;            // ʣ������ʱ��
    int priority;        // ���ȼ�������ԽС���ȼ�Խ�ߣ�
    char state;          // ״̬��'W'����, 'R'����, 'T'��ֹ
    struct PCB* next;    // ����ָ�루���ڶ��У�
} PCB;

// ȫ�ֱ�����������ض��к�״̬��
PCB* job = NULL;       // ԭʼ���̶��У�δ����Ľ��̣�
PCB* ready = NULL;     // ��������
PCB* run = NULL;       // ��ǰ���еĽ���
PCB* finish = NULL;    // ����ɽ��̶���
int timenow = 0;       // ��ǰʱ��
int gantt[100];        // ����ͼ���飨��¼ÿ��ʱ��Ƭ���еĽ���PID��
const int SLICE = 2;   // RR�㷨��ʱ��Ƭ��С�����޸ģ�


PCB* createPCB(int pid, int arrival, int burst, int priority) {
    PCB* p = (PCB*)malloc(sizeof(PCB));
    p->pid = pid;
    p->arrival = arrival;
    p->burst = burst;
    p->rest = burst;    // ��ʼʣ��ʱ��=������ʱ��
    p->priority = priority;
    p->state = 'W';     // ��ʼ״̬Ϊ����
    p->next = NULL;
    return p;
}

void InitialJob() {
    job = createPCB(0, -1, 0, 0);  // �ڱ��ڵ㣨�����������
    PCB* tail = job;

    // ������ɽ��̲���������ʱ��0-3������ʱ��1-4�����ȼ�1-5��
    srand((unsigned int)time(NULL));
    for (int i = 1; i <= 5; i++) {
        int arrival = rand() % 4;    // 0-3
        int burst = rand() % 4 + 1;  // 1-4
        int priority = rand() % 5 + 1; // 1-5
        PCB* p = createPCB(i, arrival, burst, priority);
        tail->next = p;
        tail = p;
    }

    printf("===== ��ʼ������Ϣ =====\n");
    printf("PID | ����ʱ�� | ����ʱ�� | ���ȼ�\n");
    printf("--------------------------\n");
    PCB* temp = job->next;
    while (temp != NULL) {
        printf("P%d  |    %d     |    %d     |    %d\n",
            temp->pid, temp->arrival, temp->burst, temp->priority);
        temp = temp->next;
    }
    printf("==========================\n\n");
}

// ������������ʾ�����еĽ�����Ϣ�����ڵ��Ժ������
void DisplayPCB(PCB* head) {
    if (head == NULL) {
        printf("(����Ϊ��)\n");
        return;
    }
    printf("PID | ����ʱ�� | ʣ��ʱ�� | ���ȼ� | ״̬\n");
    printf("----------------------------------------\n");
    PCB* p = head;
    while (p != NULL) {
        printf("P%d  |    %d     |    %d     |    %d    |   %c\n",
            p->pid, p->arrival, p->rest, p->priority, p->state);
        p = p->next;
    }
    printf("----------------------------------------\n");
}

// ���ĺ�����ˢ�¾������У������㷨��������µ���Ľ��̣�������
// algorithm: "FCFS", "RR", "SJF", "SRTF", "NonPriority", "Priority"
void ReadyQueue(const char* algorithm, int time) {
    // 1. ��job�����е���ʱ��<=��ǰʱ��Ľ����Ƶ���������
    PCB* prev = job;
    PCB* curr = job->next;
    while (curr != NULL) {
        if (curr->arrival <= time) {
            // ��job�����Ƴ�
            prev->next = curr->next;
            curr->next = NULL;

            // ��ӵ��������У������㷨����
            if (ready == NULL) {
                ready = curr;
            }
            else {
                // ���㷨������뵽�������еĶ�Ӧλ��
                PCB* p = ready;
                PCB* q = NULL;

                // FCFS/RR��������ʱ�����򣨵���ʱ����ͬ��PID��
                if (strcmp(algorithm, "FCFS") == 0 || strcmp(algorithm, "RR") == 0) {
                    while (p != NULL && (p->arrival < curr->arrival ||
                        (p->arrival == curr->arrival && p->pid < curr->pid))) {
                        q = p;
                        p = p->next;
                    }
                }
                // SJF����������ʱ�䣨burst�����򣨶���ҵ���ȣ�
                else if (strcmp(algorithm, "SJF") == 0) {
                    while (p != NULL && (p->burst < curr->burst ||
                        (p->burst == curr->burst && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }
                // SRTF����ʣ��ʱ�䣨rest���������ʣ��ʱ�����ȣ�
                else if (strcmp(algorithm, "SRTF") == 0) {
                    while (p != NULL && (p->rest < curr->rest ||
                        (p->rest == curr->rest && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }
                // ���ȼ����ȣ�����ռ/��ռ���������ȼ���������С���ȣ�
                else if (strcmp(algorithm, "NonPriority") == 0 || strcmp(algorithm, "Priority") == 0) {
                    while (p != NULL && (p->priority < curr->priority ||
                        (p->priority == curr->priority && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }

                // ����ڵ�
                if (q == NULL) { // ����ͷ��
                    curr->next = ready;
                    ready = curr;
                }
                else { // �����м��β��
                    curr->next = p;
                    q->next = curr;
                }
            }
            curr->state = 'W'; // ���Ϊ����״̬
            curr = prev->next; // ����������һ������
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }
}

// 1. �����ȷ�������㷨��FCFS��
void FCFS() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt)); // ��ʼ������ͼ

    printf("\n===== ��ʼFCFS���� =====\n");
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        // ˢ�¾������У�����µ���Ľ��̣�
        ReadyQueue("FCFS", timenow);
        printf("�������У�\n");
        DisplayPCB(ready);

        // �˳����������н��̴������
        if (job->next == NULL && ready == NULL && run == NULL) {
            break;
        }

        // ��CPU���У��Ӿ�������ȡ��һ������
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R'; // ���Ϊ����״̬
            printf("���ȣ�P%d��ʼ���У�FCFS��\n", run->pid);
        }

        // ���е�ǰ���̣������ڣ�
        if (run != NULL) {
            run->rest--; // ʣ��ʱ���1
            gantt[timenow] = run->pid; // ��¼����ͼ
            printf("�����У�P%d��ʣ��ʱ�䣺%d��\n", run->pid, run->rest);

            // �������н���
            if (run->rest == 0) {
                run->state = 'T'; // ���Ϊ��ֹ״̬
                printf("������P%d�������\n", run->pid);
                // ������ɶ���
                run->next = finish;
                finish = run;
                run = NULL; // CPU����
            }
        }
        else {
            gantt[timenow] = 0; // ��¼CPU����
            printf("CPU����\n");
        }

        timenow++; // ʱ���ƽ�
    }
    printf("\n===== FCFS���Ƚ��� =====\n");
}

// 2. ��ת�����㷨��RR��
void RR() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));
    int time_count = 0; // ��¼��ǰ���������е�ʱ��Ƭ

    printf("\n===== ��ʼRR���ȣ�ʱ��Ƭ��%d�� =====\n", SLICE);
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        ReadyQueue("RR", timenow); // ˢ�¾������У�������ʱ������
        printf("�������У�\n");
        DisplayPCB(ready);

        // �˳�����
        if (job->next == NULL && ready == NULL && run == NULL) {
            break;
        }

        // ��CPU���У��Ӿ�������ȡ����
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            time_count = 0; // ����ʱ��Ƭ����
            printf("���ȣ�P%d��ʼ���У�RR��\n", run->pid);
        }

        // ���е�ǰ����
        if (run != NULL) {
            run->rest--;
            time_count++;
            gantt[timenow] = run->pid;
            printf("�����У�P%d��ʣ��ʱ�䣺%d������ʱ��Ƭ��%d��\n",
                run->pid, run->rest, time_count);

            // ���̽���
            if (run->rest == 0) {
                run->state = 'T';
                printf("������P%d�������\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
                time_count = 0;
            }
            // ʱ��Ƭ���꣬�Żؾ�������β��
            else if (time_count >= SLICE) {
                printf("ʱ��Ƭ���꣺P%d�Żؾ�������\n", run->pid);
                run->state = 'W';
                // �����������β��
                if (ready == NULL) {
                    ready = run;
                }
                else {
                    PCB* p = ready;
                    while (p->next != NULL) p = p->next;
                    p->next = run;
                }
                run->next = NULL;
                run = NULL;
                time_count = 0;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU����\n");
        }

        timenow++;
    }
    printf("\n===== RR���Ƚ��� =====\n");
}

// 3. ����ҵ���ȣ�SJF������ռ��
void SJF() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== ��ʼSJF���� =====\n");
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        ReadyQueue("SJF", timenow); // ������ʱ�������������
        printf("�������У�������ʱ�����򣩣�\n");
        DisplayPCB(ready);
        if (job->next == NULL && ready == NULL && run == NULL) break;
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("���ȣ�P%d�������ҵ������ʱ�䣺%d����ʼ����\n", run->pid, run->burst);
        }

        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("�����У�P%d��ʣ��ʱ�䣺%d��\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("������P%d�������\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU����\n");
        }

        timenow++;
    }
    printf("\n===== SJF���Ƚ��� =====\n");
}

// 4. ���ʣ��ʱ�����ȣ�SRTF����ռʽ��
void SRTF() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== ��ʼSRTF���� =====\n");
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        ReadyQueue("SRTF", timenow); // ��ʣ��ʱ�������������
        printf("�������У���ʣ��ʱ�����򣩣�\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // ��ռ�жϣ���������ͷ��ʣ��ʱ�� < ��ǰ���н��̵�ʣ��ʱ��
        if (run != NULL && ready != NULL && ready->rest < run->rest) {
            printf("��ռ��P%d��ʣ��ʱ�䣺%d����ռP%d��ʣ��ʱ�䣺%d��\n",
                ready->pid, ready->rest, run->pid, run->rest);
            // ��ǰ���̷Żؾ������У�����������
            run->state = 'W';
            run->next = ready;
            ready = run;
            run = NULL;
        }

        // CPU����ʱȡ���ʣ��ʱ�����
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("���ȣ�P%d��ʣ��ʱ�䣺%d����ʼ����\n", run->pid, run->rest);
        }

        // ���е�ǰ����
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("�����У�P%d��ʣ��ʱ�䣺%d��\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("������P%d�������\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU����\n");
        }

        timenow++;
    }
    printf("\n===== SRTF���Ƚ��� =====\n");
}

// 5. ����ռʽ���ȼ�����
void NonPriority() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== ��ʼ����ռʽ���ȼ����� =====\n");
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        ReadyQueue("NonPriority", timenow); // �����ȼ������������
        printf("�������У������ȼ���������ԽС���ȼ�Խ�ߣ���\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // CPU����ʱȡ������ȼ�����
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("���ȣ�P%d�����ȼ���%d����ʼ����\n", run->pid, run->priority);
        }

        // ���е�ǰ���̣�һ�����У�ֱ��������
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("�����У�P%d��ʣ��ʱ�䣺%d��\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("������P%d�������\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU����\n");
        }

        timenow++;
    }
    printf("\n===== ����ռʽ���ȼ����Ƚ��� =====\n");
}

// 6. ��ռʽ���ȼ�����
void Priority() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== ��ʼ��ռʽ���ȼ����� =====\n");
    while (1) {
        printf("\n��ʱ��Ƭ��%d��\n", timenow);
        ReadyQueue("Priority", timenow); // �����ȼ������������
        printf("�������У������ȼ���������ԽС���ȼ�Խ�ߣ���\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // ��ռ�жϣ���������ͷ�����ȼ� > ��ǰ���̣����ָ�С��
        if (run != NULL && ready != NULL && ready->priority < run->priority) {
            printf("��ռ��P%d�����ȼ���%d����ռP%d�����ȼ���%d��\n",
                ready->pid, ready->priority, run->pid, run->priority);
            // ��ǰ���̷Żؾ�������
            run->state = 'W';
            run->next = ready;
            ready = run;
            run = NULL;
        }

        // CPU����ʱȡ������ȼ�����
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("���ȣ�P%d�����ȼ���%d����ʼ����\n", run->pid, run->priority);
        }

        // ���е�ǰ����
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("�����У�P%d��ʣ��ʱ�䣺%d��\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("������P%d�������\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU����\n");
        }

        timenow++;
    }
    printf("\n===== ��ռʽ���ȼ����Ƚ��� =====\n");
}

// ��ʾ����ͼ
void DisplayGantt() {
    printf("\n===== ����ͼ =====\n");
    printf("ʱ��Ƭ��");
    for (int i = 0; i < timenow; i++) {
        printf("%d ", i);
    }
    printf("\n����   ��");
    for (int i = 0; i < timenow; i++) {
        if (gantt[i] == 0) {
            printf("- "); // ��ʾ����
        }
        else {
            printf("P%d ", gantt[i]);
        }
    }
    printf("\n==================\n");
}

// ���㲢��ʾ��תʱ�䡢�ȴ�ʱ��
void DisplayTime() {
    printf("\n===== ��������ͳ�� =====\n");
    printf("PID | ����ʱ�� | ����ʱ�� | ���ʱ�� | ��תʱ�� | �ȴ�ʱ��\n");
    printf("--------------------------------------------------------\n");

    int total_turnaround = 0;
    int total_wait = 0;
    int count = 0;

    PCB* p = finish;
    while (p != NULL) {
        int finish_time = 0;
        for (int i = 0; i < timenow; i++) {
            if (gantt[i] == p->pid) {
                finish_time = i;
            }
        }
        int turnaround = finish_time - p->arrival + 1; // +1����Ϊʱ��Ƭ��0��ʼ
        int wait = turnaround - p->burst;

        printf("P%d  |    %d     |    %d     |    %d     |    %d     |    %d\n",
            p->pid, p->arrival, p->burst, finish_time + 1, turnaround, wait);

        total_turnaround += turnaround;
        total_wait += wait;
        count++;
        p = p->next;
    }

    printf("--------------------------------------------------------\n");
    printf("ƽ����תʱ�䣺%.2f\n", (float)total_turnaround / count);
    printf("ƽ���ȴ�ʱ�䣺%.2f\n", (float)total_wait / count);
    printf("========================================================\n");
}






int main() {
    InitialJob(); // ��ʼ������

    //FCFS();
    //RR();
     //SJF();
    //SRTF();
    //NonPriority();
     Priority();

    DisplayGantt();
    DisplayTime();

    return 0;
}