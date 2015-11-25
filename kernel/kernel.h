/**
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 *
 * Copyright (C) 2013 Hong MingJian<hongmingjian@gmail.com>
 * All rights reserved.
 *
 * This file is part of the EPOS.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 *
 */
#ifndef _KERNEL_H
#define _KERNEL_H

#include <sys/types.h>
#include <inttypes.h>
#include "machdep.h"

/*中断向量表*/
extern void (*g_intr_vector[])(uint32_t irq, struct context *ctx);

/*让中断控制器打开某个中断*/
void disable_irq(uint32_t irq);

/*让中断控制器关闭某个中断*/
void enable_irq(uint32_t irq);

/*定时器以HZ的频率中断CPU*/
#define HZ   100

/*记录系统启动以来，定时器中断的次数*/
extern unsigned volatile g_timer_ticks;



struct tm {
    int tm_sec;         /* seconds */
    int tm_min;         /* minutes */
    int tm_hour;        /* hours */
    int tm_mday;        /* day of the month */
    int tm_mon;         /* month */
    int tm_year;        /* year */
    int tm_wday;        /* day of the week */
    int tm_yday;        /* day in the year */
    int tm_isdst;       /* daylight saving time */
};
time_t mktime(struct tm * tm);

/*计算机启动时，自1970-01-01 00:00:00 +0000 (UTC)以来的秒数*/
extern time_t g_startup_time;

/**
 * 线程控制块
 */
struct tcb {
    /*hardcoded*/
    uint32_t        kstack;/*saved top of the kernel stack for this task*/

    int         tid;/* task id */
    int         state;/* -1:waiting, 0:running, 1:ready, 2:zombie */
#define TASK_STATE_WAITING  -1
#define TASK_STATE_READY     1
#define TASK_STATE_ZOMBIE    2

    int         timeslice;      //时间片
#define DEFAULT_TIMESLICE 4

    int          code_exit;     //保存该线程的退出代码
    struct wait_queue *wq_exit; //等待该线程退出的队列

    struct tcb     *next;
    struct x87      fpu;        //数学协处理器的寄存器
};

extern struct tcb *g_task_running;
extern struct tcb *g_task_head;
extern struct tcb *task0;
extern struct tcb *g_task_own_fpu;

extern int g_resched;
void schedule();
void switch_to(struct tcb *new);

struct wait_queue {
    struct tcb *tsk;
    struct wait_queue *next;
};
void sleep_on(struct wait_queue **head);
void wake_up(struct wait_queue **head, int n);

void init_task(void);
void syscall(struct context *ctx);
extern void *ret_from_syscall;

/**
 * `VADDR' comes from FreeBSD
 */
#define VADDR(pdi, pti) ((uint32_t)(((pdi)<<PGDR_SHIFT)|((pti)<<PAGE_SHIFT)))

#define KERN_MAX_ADDR VADDR(1023, 1023)
#define KERN_MIN_ADDR VADDR(767, 767)
#define USER_MAX_ADDR VADDR(767, 0)
#define USER_MIN_ADDR VADDR(1, 0)

#define LOAD_ADDR 0x100000
#define KERNBASE  VADDR(768, 0)
#define R(x) ((x)-KERNBASE+LOAD_ADDR)

#define NR_KERN_PAGETABLE 20

/**
 * `PT', `PTD' and `vtopte' come from FreeBSD
 */
extern uint8_t   end;
extern uint32_t *PT;
extern uint32_t *PTD;
#define vtopte(va) (PT+((va)>>PAGE_SHIFT))
#define vtop(va) ((*vtopte(va))&(~PAGE_MASK)|((va)&PAGE_MASK))

void  init_kmalloc(void *mem, size_t bytes);
void *kmalloc(size_t bytes);
void *krealloc(void *oldptr, size_t size);
void  kfree(void *ptr);
void *aligned_kmalloc(size_t bytes, size_t align);
void  aligned_kfree(void *ptr);

#define RAM_ZONE_LEN (2 * 8)
extern uint32_t g_ram_zone[RAM_ZONE_LEN];

int do_page_fault(struct context *ctx, uint32_t vaddr, uint32_t code);

#include "dosfs.h"
uint32_t load_pe(VOLINFO *pvi, char *filename, uint32_t *end);

int     sys_putchar(int c);
int     sys_getchar();

struct tcb *sys_task_create(void *tos, void (*func)(void *pv), void *pv);
void        sys_task_exit(int code_exit);
int         sys_task_wait(int tid, int *pcode_exit);
int         sys_task_getid();
void        sys_task_yield();

void     init_vmspace(uint32_t brk);
uint32_t page_alloc(int npages, uint32_t user);
uint32_t page_alloc_in_addr(uint32_t va, int npages);
int      page_free(uint32_t va, int npages);
int      page_check(uint32_t va);
void     page_map(uint32_t vaddr, uint32_t paddr, uint32_t npages, uint32_t flags);
void     page_unmap(uint32_t vaddr, uint32_t npages);

uint32_t init_frame(uint32_t brk);
uint32_t frame_alloc(uint32_t npages);
uint32_t frame_alloc_in_addr(uint32_t pa, uint32_t npages);
void     frame_free(uint32_t paddr, uint32_t npages);

#endif /*_KERNEL_H*/