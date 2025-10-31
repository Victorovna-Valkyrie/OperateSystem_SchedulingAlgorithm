/*
 * 进程调度算法模拟实验
 * 功能：实现FCFS、RR、SJF、SRTF、非抢占式优先级、抢占式优先级6种调度算法
 * 输出：调度过程日志、甘特图、周转时间/等待时间统计
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

 // 进程控制块（PCB）结构体定义
typedef struct PCB {
    int pid;             // 进程ID
    int arrival;         // 到达时间
    int burst;           // 总运行时间（CPU区间）
    int rest;            // 剩余运行时间
    int priority;        // 优先级（数字越小优先级越高）
    char state;          // 状态：'W'就绪, 'R'运行, 'T'终止
    struct PCB* next;    // 链表指针（用于队列）
} PCB;

// 全局变量（调度相关队列和状态）
PCB* job = NULL;       // 原始进程队列（未到达的进程）
PCB* ready = NULL;     // 就绪队列
PCB* run = NULL;       // 当前运行的进程
PCB* finish = NULL;    // 已完成进程队列
int timenow = 0;       // 当前时间
int gantt[100];        // 甘特图数组（记录每个时间片运行的进程PID）
const int SLICE = 2;   // RR算法的时间片大小（可修改）


PCB* createPCB(int pid, int arrival, int burst, int priority) {
    PCB* p = (PCB*)malloc(sizeof(PCB));
    p->pid = pid;
    p->arrival = arrival;
    p->burst = burst;
    p->rest = burst;    // 初始剩余时间=总运行时间
    p->priority = priority;
    p->state = 'W';     // 初始状态为就绪
    p->next = NULL;
    return p;
}

void InitialJob() {
    job = createPCB(0, -1, 0, 0);  // 哨兵节点（简化链表操作）
    PCB* tail = job;

    // 随机生成进程参数（到达时间0-3，运行时间1-4，优先级1-5）
    srand((unsigned int)time(NULL));
    for (int i = 1; i <= 5; i++) {
        int arrival = rand() % 4;    // 0-3
        int burst = rand() % 4 + 1;  // 1-4
        int priority = rand() % 5 + 1; // 1-5
        PCB* p = createPCB(i, arrival, burst, priority);
        tail->next = p;
        tail = p;
    }

    printf("===== 初始进程信息 =====\n");
    printf("PID | 到达时间 | 运行时间 | 优先级\n");
    printf("--------------------------\n");
    PCB* temp = job->next;
    while (temp != NULL) {
        printf("P%d  |    %d     |    %d     |    %d\n",
            temp->pid, temp->arrival, temp->burst, temp->priority);
        temp = temp->next;
    }
    printf("==========================\n\n");
}

// 辅助函数：显示队列中的进程信息（用于调试和输出）
void DisplayPCB(PCB* head) {
    if (head == NULL) {
        printf("(队列为空)\n");
        return;
    }
    printf("PID | 到达时间 | 剩余时间 | 优先级 | 状态\n");
    printf("----------------------------------------\n");
    PCB* p = head;
    while (p != NULL) {
        printf("P%d  |    %d     |    %d     |    %d    |   %c\n",
            p->pid, p->arrival, p->rest, p->priority, p->state);
        p = p->next;
    }
    printf("----------------------------------------\n");
}

// 核心函数：刷新就绪队列（根据算法类型添加新到达的进程，并排序）
// algorithm: "FCFS", "RR", "SJF", "SRTF", "NonPriority", "Priority"
void ReadyQueue(const char* algorithm, int time) {
    // 1. 将job队列中到达时间<=当前时间的进程移到就绪队列
    PCB* prev = job;
    PCB* curr = job->next;
    while (curr != NULL) {
        if (curr->arrival <= time) {
            // 从job队列移除
            prev->next = curr->next;
            curr->next = NULL;

            // 添加到就绪队列（根据算法排序）
            if (ready == NULL) {
                ready = curr;
            }
            else {
                // 按算法规则插入到就绪队列的对应位置
                PCB* p = ready;
                PCB* q = NULL;

                // FCFS/RR：按到达时间排序（到达时间相同按PID）
                if (strcmp(algorithm, "FCFS") == 0 || strcmp(algorithm, "RR") == 0) {
                    while (p != NULL && (p->arrival < curr->arrival ||
                        (p->arrival == curr->arrival && p->pid < curr->pid))) {
                        q = p;
                        p = p->next;
                    }
                }
                // SJF：按总运行时间（burst）排序（短作业优先）
                else if (strcmp(algorithm, "SJF") == 0) {
                    while (p != NULL && (p->burst < curr->burst ||
                        (p->burst == curr->burst && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }
                // SRTF：按剩余时间（rest）排序（最短剩余时间优先）
                else if (strcmp(algorithm, "SRTF") == 0) {
                    while (p != NULL && (p->rest < curr->rest ||
                        (p->rest == curr->rest && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }
                // 优先级调度（非抢占/抢占）：按优先级排序（数字小优先）
                else if (strcmp(algorithm, "NonPriority") == 0 || strcmp(algorithm, "Priority") == 0) {
                    while (p != NULL && (p->priority < curr->priority ||
                        (p->priority == curr->priority && p->arrival < curr->arrival))) {
                        q = p;
                        p = p->next;
                    }
                }

                // 插入节点
                if (q == NULL) { // 插入头部
                    curr->next = ready;
                    ready = curr;
                }
                else { // 插入中间或尾部
                    curr->next = p;
                    q->next = curr;
                }
            }
            curr->state = 'W'; // 标记为就绪状态
            curr = prev->next; // 继续处理下一个进程
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }
}

// 1. 先来先服务调度算法（FCFS）
void FCFS() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt)); // 初始化甘特图

    printf("\n===== 开始FCFS调度 =====\n");
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        // 刷新就绪队列（添加新到达的进程）
        ReadyQueue("FCFS", timenow);
        printf("就绪队列：\n");
        DisplayPCB(ready);

        // 退出条件：所有进程处理完毕
        if (job->next == NULL && ready == NULL && run == NULL) {
            break;
        }

        // 若CPU空闲，从就绪队列取第一个进程
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R'; // 标记为运行状态
            printf("调度：P%d开始运行（FCFS）\n", run->pid);
        }

        // 运行当前进程（若存在）
        if (run != NULL) {
            run->rest--; // 剩余时间减1
            gantt[timenow] = run->pid; // 记录甘特图
            printf("运行中：P%d（剩余时间：%d）\n", run->pid, run->rest);

            // 进程运行结束
            if (run->rest == 0) {
                run->state = 'T'; // 标记为终止状态
                printf("结束：P%d完成运行\n", run->pid);
                // 加入完成队列
                run->next = finish;
                finish = run;
                run = NULL; // CPU空闲
            }
        }
        else {
            gantt[timenow] = 0; // 记录CPU空闲
            printf("CPU空闲\n");
        }

        timenow++; // 时间推进
    }
    printf("\n===== FCFS调度结束 =====\n");
}

// 2. 轮转调度算法（RR）
void RR() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));
    int time_count = 0; // 记录当前进程已运行的时间片

    printf("\n===== 开始RR调度（时间片：%d） =====\n", SLICE);
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        ReadyQueue("RR", timenow); // 刷新就绪队列（按到达时间排序）
        printf("就绪队列：\n");
        DisplayPCB(ready);

        // 退出条件
        if (job->next == NULL && ready == NULL && run == NULL) {
            break;
        }

        // 若CPU空闲，从就绪队列取进程
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            time_count = 0; // 重置时间片计数
            printf("调度：P%d开始运行（RR）\n", run->pid);
        }

        // 运行当前进程
        if (run != NULL) {
            run->rest--;
            time_count++;
            gantt[timenow] = run->pid;
            printf("运行中：P%d（剩余时间：%d，已用时间片：%d）\n",
                run->pid, run->rest, time_count);

            // 进程结束
            if (run->rest == 0) {
                run->state = 'T';
                printf("结束：P%d完成运行\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
                time_count = 0;
            }
            // 时间片用完，放回就绪队列尾部
            else if (time_count >= SLICE) {
                printf("时间片用完：P%d放回就绪队列\n", run->pid);
                run->state = 'W';
                // 插入就绪队列尾部
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
            printf("CPU空闲\n");
        }

        timenow++;
    }
    printf("\n===== RR调度结束 =====\n");
}

// 3. 短作业优先（SJF，非抢占）
void SJF() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== 开始SJF调度 =====\n");
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        ReadyQueue("SJF", timenow); // 按运行时间排序就绪队列
        printf("就绪队列（按运行时间排序）：\n");
        DisplayPCB(ready);
        if (job->next == NULL && ready == NULL && run == NULL) break;
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("调度：P%d（最短作业，运行时间：%d）开始运行\n", run->pid, run->burst);
        }

        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("运行中：P%d（剩余时间：%d）\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("结束：P%d完成运行\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU空闲\n");
        }

        timenow++;
    }
    printf("\n===== SJF调度结束 =====\n");
}

// 4. 最短剩余时间优先（SRTF，抢占式）
void SRTF() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== 开始SRTF调度 =====\n");
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        ReadyQueue("SRTF", timenow); // 按剩余时间排序就绪队列
        printf("就绪队列（按剩余时间排序）：\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // 抢占判断：就绪队列头的剩余时间 < 当前运行进程的剩余时间
        if (run != NULL && ready != NULL && ready->rest < run->rest) {
            printf("抢占：P%d（剩余时间：%d）抢占P%d（剩余时间：%d）\n",
                ready->pid, ready->rest, run->pid, run->rest);
            // 当前进程放回就绪队列（会重新排序）
            run->state = 'W';
            run->next = ready;
            ready = run;
            run = NULL;
        }

        // CPU空闲时取最短剩余时间进程
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("调度：P%d（剩余时间：%d）开始运行\n", run->pid, run->rest);
        }

        // 运行当前进程
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("运行中：P%d（剩余时间：%d）\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("结束：P%d完成运行\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU空闲\n");
        }

        timenow++;
    }
    printf("\n===== SRTF调度结束 =====\n");
}

// 5. 非抢占式优先级调度
void NonPriority() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== 开始非抢占式优先级调度 =====\n");
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        ReadyQueue("NonPriority", timenow); // 按优先级排序就绪队列
        printf("就绪队列（按优先级排序，数字越小优先级越高）：\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // CPU空闲时取最高优先级进程
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("调度：P%d（优先级：%d）开始运行\n", run->pid, run->priority);
        }

        // 运行当前进程（一旦运行，直到结束）
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("运行中：P%d（剩余时间：%d）\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("结束：P%d完成运行\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU空闲\n");
        }

        timenow++;
    }
    printf("\n===== 非抢占式优先级调度结束 =====\n");
}

// 6. 抢占式优先级调度
void Priority() {
    timenow = 0;
    memset(gantt, 0, sizeof(gantt));

    printf("\n===== 开始抢占式优先级调度 =====\n");
    while (1) {
        printf("\n【时间片：%d】\n", timenow);
        ReadyQueue("Priority", timenow); // 按优先级排序就绪队列
        printf("就绪队列（按优先级排序，数字越小优先级越高）：\n");
        DisplayPCB(ready);

        if (job->next == NULL && ready == NULL && run == NULL) break;

        // 抢占判断：就绪队列头的优先级 > 当前进程（数字更小）
        if (run != NULL && ready != NULL && ready->priority < run->priority) {
            printf("抢占：P%d（优先级：%d）抢占P%d（优先级：%d）\n",
                ready->pid, ready->priority, run->pid, run->priority);
            // 当前进程放回就绪队列
            run->state = 'W';
            run->next = ready;
            ready = run;
            run = NULL;
        }

        // CPU空闲时取最高优先级进程
        if (run == NULL && ready != NULL) {
            run = ready;
            ready = ready->next;
            run->next = NULL;
            run->state = 'R';
            printf("调度：P%d（优先级：%d）开始运行\n", run->pid, run->priority);
        }

        // 运行当前进程
        if (run != NULL) {
            run->rest--;
            gantt[timenow] = run->pid;
            printf("运行中：P%d（剩余时间：%d）\n", run->pid, run->rest);

            if (run->rest == 0) {
                run->state = 'T';
                printf("结束：P%d完成运行\n", run->pid);
                run->next = finish;
                finish = run;
                run = NULL;
            }
        }
        else {
            gantt[timenow] = 0;
            printf("CPU空闲\n");
        }

        timenow++;
    }
    printf("\n===== 抢占式优先级调度结束 =====\n");
}

// 显示甘特图
void DisplayGantt() {
    printf("\n===== 甘特图 =====\n");
    printf("时间片：");
    for (int i = 0; i < timenow; i++) {
        printf("%d ", i);
    }
    printf("\n进程   ：");
    for (int i = 0; i < timenow; i++) {
        if (gantt[i] == 0) {
            printf("- "); // 表示空闲
        }
        else {
            printf("P%d ", gantt[i]);
        }
    }
    printf("\n==================\n");
}

// 计算并显示周转时间、等待时间
void DisplayTime() {
    printf("\n===== 调度性能统计 =====\n");
    printf("PID | 到达时间 | 运行时间 | 完成时间 | 周转时间 | 等待时间\n");
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
        int turnaround = finish_time - p->arrival + 1; // +1是因为时间片从0开始
        int wait = turnaround - p->burst;

        printf("P%d  |    %d     |    %d     |    %d     |    %d     |    %d\n",
            p->pid, p->arrival, p->burst, finish_time + 1, turnaround, wait);

        total_turnaround += turnaround;
        total_wait += wait;
        count++;
        p = p->next;
    }

    printf("--------------------------------------------------------\n");
    printf("平均周转时间：%.2f\n", (float)total_turnaround / count);
    printf("平均等待时间：%.2f\n", (float)total_wait / count);
    printf("========================================================\n");
}






int main() {
    InitialJob(); // 初始化进程

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