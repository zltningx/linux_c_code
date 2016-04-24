/*************************************************************************
	> File Name: device.c
	> Author: 郑凌弢 LIT0
	> Mail: zltningx@outlook.com
	> Created Time: 2016年04月11日 星期一 20时17分05秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<curses.h>
#include<string.h>

#define ERROR_LINE 50

struct PCB {
    char p_name[10];
    struct PCB *next;
};

struct Node {
    char name[10];
    struct PCB *process;
    struct PCB *waitinglist;
    struct Node *next;
    struct Node *prior;
    struct Node *parent;
};

struct SDT {
    char name[10];
    struct Node *DCT;
    struct SDT *next;
    struct SDT *same;
};

//声明
struct Node * create_node();
struct SDT * create_sdt_head();
struct Node * append(struct Node *node, char my_name[]);
void add_dev(struct SDT *head, char type[], struct Node *d);
void append_process(struct Node *node, struct PCB * pcb);
void init_first(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs);
void draw_forever(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs);
void draw_input(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs);
void draw_useage();

int 
main(void)
{
    struct Node *CHCTs;  //通道队列头
    struct Node *COCTs; //控制器队列头
    struct Node *DCTs; //设备队列头
    struct SDT *SDTs; //系统设备表头

    //init
    CHCTs = create_node();
    COCTs = create_node();
    DCTs = create_node();
    SDTs = create_sdt_head();

    init_first(CHCTs, COCTs, DCTs, SDTs);
    //scr init
    initscr();


    while(1) {
        draw_forever(CHCTs, COCTs, DCTs, SDTs);
        draw_input(CHCTs,COCTs,DCTs,SDTs);
    }

    endwin();
    exit(0);
}

/* Init */
void 
init_first(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs)
{
    //初始化 keyboard,mouse,print,screen;
    //3个控制器
    //2个不可变通道！ Let's Begin 

    struct Node *d1, *d2, *d3, *d4, *co1, *co2, *co3, *ch1, *ch2;
    struct SDT *s = SDTs;
    d1 = append(DCTs,"keyboard_1");
    d2 = append(DCTs,"mouse_1");
    d3 = append(DCTs,"print_1");
    d4 = append(DCTs,"screen_1");
    
    co1 = append(COCTs,"co1");
    co2 = append(COCTs,"co2");
    co3 = append(COCTs,"co3");

    ch1 = append(CHCTs,"ch1");
    ch2 = append(CHCTs,"ch2");

    add_dev(SDTs, "k", d1); 
    add_dev(SDTs, "m", d2); 
    add_dev(SDTs, "p", d3); 
    add_dev(SDTs, "s", d4); 
    
    d1->parent = co1;
    d2->parent = co1;
    d3->parent = co2;
    d4->parent = co3;

    co1->parent = ch1;
    co2->parent = ch2;
    co3->parent = ch2;
}

/* End Init */

/* 主要功能实现 */
void
apply(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs, char *proc, char *dev)
{
    int flag = 0,t = 0;
    struct SDT *s = SDTs;
    struct PCB *new;
    struct Node *d,*co,*ch;
    while (s->next != NULL) {
        s = s->next;
        if (strncmp(s->name, dev, sizeof(s->name)) == 0) {
            while (s->same != NULL) {
                s  = s->same;
                d = s->DCT;
                if (s->DCT->process->next == NULL) {
                    new = (struct PCB *)malloc(sizeof(struct PCB));
                    strcpy(new->p_name, proc);
                    d->process->next = new;
                    new->next = NULL;

                    flag = 1;
                    break;
                } 
            }
            t = 1;
        }
        if (flag) {
            break;
        }
    }
    if (!t) {
        mvprintw(6,0,"No Found Dev");
        refresh();
        sleep(1);
        return ;
    }

    if (!flag) {
        new = (struct PCB *)malloc(sizeof(struct PCB));
        strcpy(new->p_name, proc);
        append_process(d, new);
        mvprintw(6,0,"Apply Failure");
        refresh();
        sleep(1);
        return;
    }
    co = d->parent;
    if (co->process->next == NULL) {
        new = (struct PCB *)malloc(sizeof(struct PCB));
        strcpy(new->p_name, proc);
        co->process->next = new;
        new->next = NULL;
    } else {
        new = (struct PCB *)malloc(sizeof(struct PCB));
        strcpy(new->p_name, proc);
        append_process(co, new);
        mvprintw(6,0,"Apply Failure");
        refresh();
        sleep(1);
        return;
    }
    ch = co->parent;
    if (ch->process->next == NULL) {
        new = (struct PCB *)malloc(sizeof(struct PCB));
        strcpy(new->p_name, proc);
        ch->process->next = new;
        new->next = NULL;
        mvprintw(6,0,"Apply Success");
        refresh();
        sleep(1);
        return;
    } else {
        new = (struct PCB *)malloc(sizeof(struct PCB));
        strcpy(new->p_name, proc);
        append_process(ch, new);
        mvprintw(6,0,"Apply Failure");
        refresh();
        sleep(1);
        return;
    }
}

char *
ret(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs, char *dev, char *dev_name)
{
    int flag = 0;
    struct SDT *s = SDTs;
    struct PCB *new,*p,*waittemp;
    struct Node *d,*co,*ch;
    while (s->next != NULL) {
        s = s->next;
        if (strncmp(s->name, dev, sizeof(s->name)) == 0) {
            while (s->same != NULL) {
                s  = s->same;
                d  = s->DCT;
                if (strncmp(d->name, dev_name, sizeof(d->name)) == 0) {
                    if (d->process->next != NULL) {
                        p = d->process->next;
                        d->process->next = NULL;
                        if (d->waitinglist->next != NULL) {
                            waittemp = d->waitinglist->next;
                            d->waitinglist->next = waittemp->next;
                            d->process->next = waittemp;
                            waittemp->next = NULL;
                        }
                        flag = 1;
                        break;
                    } else return "No Process Running"; 
                }
            }
            d = s->DCT;
            break;
        }
        if (flag) {
            break;
        }
    }
    if ((s->next == NULL || s->same == NULL) && !flag) {
        return "No found Dev";
    }
    co = d->parent;
    if (strncmp(co->process->next->p_name,p->p_name, sizeof(p->p_name)) != 0) {
        waittemp= co->waitinglist->next;
        co->waitinglist->next = waittemp->next;
        free(waittemp);
        free(p);
        return " ";
    }
    p = co->process->next;
    co->process->next = NULL;
    if (co->waitinglist->next != NULL) {
        waittemp = co->waitinglist->next;
        co->waitinglist->next = waittemp->next;
        co->process->next = waittemp;
        waittemp->next = NULL;
    }
    ch = co->parent;
    if (strncmp(ch->process->next->p_name,p->p_name, sizeof(p->p_name)) != 0) {
        waittemp= ch->waitinglist->next;
        ch->waitinglist->next = waittemp->next;
        free(waittemp);
        free(p);
        return " ";
    }
    p = ch->process->next;
    ch->process->next = NULL;
    if (ch->waitinglist->next != NULL) {
        waittemp = ch->waitinglist->next;
        ch->waitinglist->next = waittemp->next;
        ch->process->next = waittemp;
        waittemp->next = NULL;
    }
    free(p);
}

void 
add(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs, char *chnum, char *conum, char *dev, char *dev_name)
{   
    int flag = 0;
    struct SDT *s = SDTs, *sp;
    struct Node *dct = DCTs, *dr;
    struct PCB *new,*p,*waittemp;
    struct Node *d,*co,*ch;
    d = append(DCTs, dev_name);
    add_dev(SDTs, dev, d);

    co = COCTs;
    while (co->next != NULL) {
        co = co->next;
        if (strncmp(co->name, conum, sizeof(co->name)) == 0) {
            d->parent = co;
            flag = 1;
            break;
        }
    }
    if (!flag) {
        co = append(COCTs, conum);
        d->parent = co;
    }
    ch = CHCTs->next;
    if (strncmp(chnum, ch->name, sizeof(chnum)) == 0) {
        co->parent = ch;
    } else {
        ch = ch->next;
        co->parent = ch;
    }
}

void
rm(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs, char *dev, char *dev_name)
{
    int flag = 0;
    struct SDT *s = SDTs, *sp;
    struct Node *dct = DCTs, *dr;
    struct PCB *new,*p,*waittemp;
    struct Node *d,*co,*ch;
    while (s->next != NULL) {
        s = s->next;
        if (strncmp(s->name, dev, sizeof(s->name)) == 0) {
            while (s->same != NULL) {
                sp = s;
                s  = s->same;
                d  = s->DCT;
                if (strncmp(d->name, dev_name, sizeof(d->name)) == 0) {
                    sp->same = s->same;
                    free(s);
                    flag = 1;
                    break;
                }
            }
            break;
        }
        d  = s->DCT;
        if (flag) {
            break;
        }
    }
    if ((s->next == NULL || s->same == NULL) && !flag) {
        mvprintw(6,0,"No found Dev");
        refresh();
        sleep(1);
        return;
    }
    if (d->process->next != NULL) {
        mvprintw(6,0,"This Dev have some process,please clean process before rm");
        refresh();
        sleep(1);
        return;
    }
    co = d->parent;
    d->parent = NULL;
    d->prior->next = d->next;
    if (d->next != NULL)
        d->next->prior = d->prior;
    free(d);

    while (dct->next != NULL) {
        dct = dct->next;
        if (strncmp(dct->parent->name, co->name, sizeof(co->name)) == 0) {
            return;
        }
    }
    co->prior->next = co->next;
    if (co->next != NULL)
        co->next->prior = co->prior;
    ch = co->parent;
    if (co->process != NULL && strncmp(co->process->p_name, ch->process->p_name, sizeof(ch->process->p_name)) == 0) {
        p = ch->process->next; 
        ch->process->next = NULL;
        free(p);
    }
    free(co);
}

/* 结束 主要功能实现 */

/* 链表轮子！！！ */
struct Node *
create_node()
{
    //Node 类型通用轮子喵
    struct Node *my_node;
    struct PCB *h_process,*h_waitinglist;
    h_process = (struct PCB *)malloc(sizeof(struct PCB));
    h_process->next = NULL;
    h_waitinglist = (struct PCB *)malloc(sizeof(struct PCB));
    h_waitinglist->next = NULL;
    my_node = (struct Node *)malloc(sizeof(struct Node));
    my_node->process = h_process;
    my_node->waitinglist = h_waitinglist;
    my_node->next = NULL;
    my_node->parent= NULL;
    my_node->prior = NULL;
    return my_node;
}

struct Node *
append(struct Node *node, char my_name[])
{
    //Node Append
    struct Node *p = node, *new;
    while (p->next != NULL)
        p = p->next;
    new = create_node();
    p->next = new;
    new->next = NULL;
    new->prior = p;
    strcpy(new->name,my_name);
    return new;
}

void
append_process(struct Node *node, struct PCB * pcb)
{
    //Node Process Append
    struct PCB *p;
    if (node->process->next == NULL) {
        node->process->next = pcb;
        pcb->next = NULL;
    } else {
        p = node->waitinglist;
        while (p->next != NULL)
            p = p->next;
        p->next = pcb;
        pcb->next = NULL;
    }
}

struct SDT *
create_sdt_head()
{
    struct SDT *head;
    head = (struct SDT *)malloc(sizeof(struct SDT));
    head->next = NULL;
    head->same = NULL;
    strcpy(head->name,"sys_table");
    return head;
}

void 
add_dev(struct SDT *head, char type[], struct Node *d)
{
    struct SDT *p = head,*new;

    while (p->next != NULL) {
        p = p->next;
        if (strncmp(p->name,type,sizeof(p->name)) == 0) {
            while (p->same != NULL) {
                p = p->same;
            }
            new = (struct SDT *)malloc(sizeof(struct SDT)); 
            p->same = new;
            new->DCT = d;

            return;
        }
    }
    new = (struct SDT *)malloc(sizeof(struct SDT)); 
    strcpy(new->name, type);
    p->next = new;
    new->next = NULL;

    p = (struct SDT *)malloc(sizeof(struct SDT)); 
    new->same = p;
    p->next = NULL;
    p->DCT = d;
}

/* 链表轮子结束！！！ */

/* 命令行分割解析 */
void
getstring(char *string)
{
    int len;
    wgetnstr(stdscr,string,30);
    len = strlen(string);
    if (len > 0 && string[len - 1] == '\n')
        string[len - 1] = '\0';
}

void
deal_string(char *string, struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs)
{
    int i = 0, j;
    char *temp = string;
    char command[5][20] = {};
    char buf[80];
    if (*string == '\0') {
        mvprintw(ERROR_LINE,0,"Error: empty command, check it!");
        refresh();
        sleep(2);
        return;
    }

    while (*(temp) != '\0' && *(temp) != ' ') {
        command[i][j] = *(temp++);
        j++;
        if (*(temp) == ' ') {
            command[i][j] = '\0';
            i++;
            j = 0;
            temp++;
        }
        if (i >= 5 && *(temp) != '\0') {
            mvprintw(ERROR_LINE,0,"Error: too many options!");
            refresh();
            sleep(2);
            return;
        }
    }
    if (strncmp(command[0], "apply",sizeof("apply")) == 0) {
        //process, dev
        apply(CHCTs, COCTs, DCTs, SDTs, command[1],command[2]);
        return;
    } else if (strncmp(command[0], "ret", sizeof("ret")) == 0) {
        ret(CHCTs, COCTs, DCTs, SDTs, command[1],command[2]);
        return;
    } else if (strncmp(command[0], "add", sizeof("add")) == 0) {
        add(CHCTs, COCTs, DCTs, SDTs, command[1],command[2],command[3],command[4]);
        return;
    } else if (strncmp(command[0], "rm", sizeof("rm")) == 0) {
        rm(CHCTs, COCTs, DCTs, SDTs, command[1],command[2]);
        return;
    } else if (strncmp(command[0], "help", sizeof("help")) == 0) {
        draw_useage();
        getstring(buf);
    } else {
        mvprintw(ERROR_LINE,0, "Error: unkown command");
        refresh();
        sleep(2);
        return;
    }
}

/* 结束 命令行分割解析 */

/* scr drawer */
void 
draw_node(struct Node *node, int who) 
{
    int i = 10;
    struct Node *temp,*p = node;
    struct PCB *pr;
    if (who == 1) {
        mvprintw(9, who, "DCTs's Table");
    }
    if (who == 51) {
        mvprintw(9, who, "COCTs's Tabel");
    }
    if (who == 101) {
        mvprintw(9, who, "CHCTs's Tabel");
    }
    while (p->next != NULL) {
        p = p->next;
        mvprintw(i++, who, "parent: %s", p->parent->name);
        mvprintw(i++, who, "%s", p->name);
        if (p->process->next != NULL) {
            pr = p->process->next;
            mvprintw(i++, who+5, "running: %s", pr->p_name);
        }
        if (p->waitinglist->next != NULL) {
            pr = p->waitinglist;
            pr = pr->next;
            i--;
            mvprintw(i++, who+20, "waiting: %s", pr->p_name);
            i++;
            while (pr->next != NULL) {
                pr = pr->next;
                mvprintw(i++, who+20, "         %s", pr->p_name);
            }
        }
    }
}

void 
draw_input(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs)
{
    char buf[80];
    mvprintw(5,0,"root@ root 0^0 # ");
    getstring(buf);
    deal_string(buf, CHCTs, COCTs, DCTs, SDTs);
}

void 
draw_useage()
{
    mvprintw(40,0,"Useage: ");
    mvprintw(41,6,"apply [process_name] [dev]");
    mvprintw(42,6,"ret [dev] [dev_name]");
    mvprintw(43,6,"add [ch_num] [co_name] [dev] [dev_name]");
    mvprintw(44,6,"rm [dev] [dev_name]");
}

void 
draw_forever(struct Node *CHCTs, struct Node *COCTs, struct Node *DCTs, struct SDT *SDTs)
{
    clear();
    mvprintw(0, 30, "Dev Control System");
    draw_node(DCTs, 1);
    draw_node(COCTs, 51);
    draw_node(CHCTs, 101);
    refresh();
}

/* end scr drawer */
