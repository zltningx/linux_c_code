/*************************************************************************
	> File Name: process.c
	> Author: 郑凌弢
	> Mail: zltningx@outlook.com
	> Created Time: 2016年03月12日 星期六 16时39分12秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<curses.h>
#include<string.h>
#include<stdlib.h>

#define MAX_STRING 80

#define MAINLINE 6

/*Data 区*/
struct Free_blk{
    int length;
    int addr;
    struct Free_blk *prior;
    struct Free_blk *next;
};

struct PCB {
    char name[10];
    int p_addr;
    long pid;
    int kb;
    struct PCB *next;
};

char *none_process_menu[] = {
    "Add new process",
    "Quit",
    "",
    "",
    "",
    "",
    0,
};

char *no_running_menu[] = {
    "Add new process",
    "Wake_blocked_process",
    "Clean all Process",
    "Quit",
    "",
    0,
};

char *menu[] = {
    "Add new process",
    "Time slice",
    "Process blocking",
    "Wake up the process",
    "End of the process",
    "Clean all process",
    "Quit",
    0,
};

//全局
static char current_process[10] = "\0";
static int memory[65];
static long pid_count = 1;

/*原型区*/

void prints(struct Free_blk *fb);
void clean_all_screen(struct PCB *ready,struct PCB *blocking,struct Free_blk *fb);
int getchoice(char *greet,char *choices[],struct PCB *ready,struct PCB *blocking,struct Free_blk *fb);
void draw_menu(char *options[], int current_row, int start_screenrow, int start_screencol);
void add(struct PCB *head, struct PCB *process);
struct PCB * creat_process();
struct PCB * creat_head();
void getstring(char *string);
void into_running(struct PCB *ready,struct PCB *running);
int is_empty(struct PCB *head);
void add(struct PCB *head, struct PCB *process);
void ready_list_adder(struct PCB *ready,struct PCB *blocking,struct Free_blk *fb);
void set_process_blocking_or_time_slice(struct PCB *running,struct PCB *list);
void free_running_process(struct Free_blk *fb,struct PCB *running);
void wake_blocked_process(struct PCB *ready,struct PCB *blocking);
void print_process_name(struct PCB *head,int col);
void free_all_process(struct PCB *head);
void free_all_fb(struct Free_blk *fb);
struct Free_blk * init_memory();
/*main 函数*/

int 
main()
{
    int choice;
    initscr();
    struct PCB *ready,*blocking,*running;
    struct Free_blk *fb;
    fb = init_memory();
    ready = creat_head();
    blocking = creat_head();
    running = creat_head();

    do{
        into_running(ready,running);
        if(is_empty(ready) && !is_empty(blocking) && is_empty(running)){
            strcpy(current_process,"\0");
            choice = getchoice("Options:",no_running_menu,ready,blocking,fb);
        } else {
            choice = getchoice("Options:",current_process[0] ? menu : none_process_menu,ready,blocking,fb);
        }
        switch(choice){
            case 'A':
                ready_list_adder(ready,blocking,fb);
                break;
            case 'P':
                set_process_blocking_or_time_slice(running,blocking);
                break;
            case 'W':
                wake_blocked_process(ready,blocking);
                break;
            case 'T':
                set_process_blocking_or_time_slice(running,ready);
                break;
            case 'E':
                free_running_process(fb,running);
                break;
            case 'C':
                free_running_process(fb,running);
                free_all_process(ready);
                free_all_process(blocking);
                free_all_fb(fb);
                pid_count = 1;
                break;
        }
    }while (choice != 'Q');
    endwin();
    exit(EXIT_SUCCESS);
}

/*curses 界面绘制区*/

int
getchoice(char *greet,char *choices[],struct PCB *ready,struct PCB *blocking,struct Free_blk *fb)
{
    static int screen_row = 0;
    int key;
    char **options = choices;
    int max_row = 0;
    int selected;
    int start_screenrow = MAINLINE;
    int start_screencol = 10;

    while (*options){
        options++;
        max_row++;
    }

    if (screen_row >= max_row)
        selected = 0;

    clean_all_screen(ready,blocking,fb);
    mvprintw(start_screenrow-2, start_screencol,greet);

    keypad(stdscr,TRUE);
    cbreak();
    noecho();
    key = 0;

    while (key != 'q' && key != KEY_ENTER && key != '\n'){
        if (key == KEY_UP){
            if (screen_row == 0)
                screen_row = max_row - 1;
            else
                screen_row--;
        }
        if (key == KEY_DOWN){
            if (screen_row == (max_row - 1))
                screen_row = 0;
            else
                screen_row++;
        }
        selected = *choices[screen_row];
        draw_menu(choices,screen_row,start_screenrow,start_screencol);
        key = getch();
    }

    keypad(stdscr,FALSE);
    nocbreak();
    echo();

    if (key == 'q'){
        selected = 'Q';
    }

    return selected;
}

void
draw_menu(char *options[], int current_row, int start_screenrow, int start_screencol)
{
    int current_rr = 0;
    char **option_ptr = options;
    char *txt_ptr;

    while (*option_ptr) {
        if (current_rr == current_row)
            attron(A_STANDOUT);
        txt_ptr = options[current_rr];

        mvprintw(start_screenrow + current_rr, start_screencol,"%s",txt_ptr);
        if (current_row == current_rr)
            attroff(A_STANDOUT);
        
        current_rr++;
        option_ptr++;
    }
    mvprintw (start_screenrow + current_rr + 3,start_screencol,"Move highlight then press return");
    refresh();
}

void
clean_all_screen(struct PCB *ready,struct PCB *blocking,struct Free_blk *fb)
{
    clear();

    mvprintw(2,20,"%s","Process CTRL System");
    if (current_process[0]){
        mvprintw(22,0,"Current process is: %s",current_process);
    } else {
        mvprintw(22,0,"No porcess running now!");
    }
    mvprintw(23,0,"ready: ");
    print_process_name(ready,0);
    mvprintw(23,30,"blocking: ");
    print_process_name(blocking,40);
    
    prints(fb);

    int i;
    int line = 40,col = 10;
    for(i = 0;i < 64;i++){
        mvprintw(line,col,"%d",memory[i]);
        if ((i+1) % 8 == 0){
            col += 2;
            mvprintw(line,col,"\n");
            line++;
            col = 10;
            continue;
        }
        col += 2;
    }
    refresh();
}

void
getstring(char *string)
{
    int len;
    wgetnstr(stdscr,string,10);
    len = strlen(string);
    if (len > 0 && string[len - 1] == '\n')
        string[len - 1] = '\0';
}

int 
str_to_int(char *string)
{
    if(*string == '\0'){
        return 0;
    }
    int sum = 0;
    sum = *string - '0';
    string++;
    while(*string != '\0'){
        sum *= 10;
        sum += (*string - '0');
        string++;
    }
    return sum;
}

void 
print_process_name(struct PCB *head,int col)
{
    struct PCB *tmp = head;
    int i = 0;
    if(!is_empty(head)){
        while (tmp->next != NULL){
            tmp = tmp->next;
            mvprintw(24 + i,5 + col,"%d",tmp->pid);
            mvprintw(24 + i,10 + col,"%s",tmp->name);
            mvprintw(24 + i,10 + col + 10,"%d",tmp->kb);
            i++;
        }
    }
}

void prints(struct Free_blk *fb)
{
    struct Free_blk *tmp = fb;
    int i = 0;
        while(tmp->next != NULL){
            tmp = tmp->next;
            mvprintw(23+i,65,"%d   %d",tmp->addr,tmp->length);
            i++;
        }
}


/*进程功能辅助函数区*/
struct PCB *
creat_head()
{
    struct PCB *head;
    head = (struct PCB *)malloc(sizeof(struct PCB));
    return head;
}

void 
add(struct PCB *head, struct PCB *process)
{
    struct PCB *tmp = head;

    while(tmp->next != NULL)
        tmp = tmp->next;

    tmp->next = process;
    process->next = NULL;
}

int
is_empty(struct PCB *head)
{
    if(head->next != NULL)
        return 0;
    else 
        return 1;
}

void 
into_running(struct PCB *ready,struct PCB *running)
{
    if(is_empty(running) && !is_empty(ready)){
        running->next = ready->next;
        ready->next = ready->next->next;
        strcpy(current_process,running->next->name);
    }
}

/*内存管理区*/

void
insert_free_blk(struct Free_blk *fb,struct Free_blk *new)
{
    int flag = 0;
    struct Free_blk *ptr = fb;
    while(ptr->next != NULL){
        if (ptr->next->length >= new->length){
            ptr->next->prior = new;
            new->next = ptr->next;
            new->prior = ptr;
            ptr->next = new;
            flag = 1;
            break;
        }
        ptr = ptr->next;
    }
    if (!flag){
        ptr->next = new;
        new->prior = ptr;
        new->next = NULL;
    }
}

struct Free_blk *
init_memory()
{
    int i = 0;
    for(; i < 64; i++){
        memory[i] = 0;
    }
    struct Free_blk *fb,*new;
    fb = (struct Free_blk *)malloc(sizeof(struct Free_blk));
    new = (struct Free_blk *)malloc(sizeof(struct Free_blk));
    fb->next = new;
    fb->prior = NULL;
    new->addr = 0;
    new->length = 64;
    new->next = NULL;
    new->prior = fb;
    return fb;
}

void
enter_mem(struct PCB *head)
{
    int i;
    for(i = head->p_addr; i < (head->p_addr + head->kb); i++)
        memory[i] = 1;
}

void
free_men(struct Free_blk *fb,struct PCB *process)
{
    int front,rear,flag = 0;
    struct Free_blk *tmp,*ptr = fb,*ptr2 = fb;
    front = process->p_addr;
    rear = front + process->kb - 1;
    if (front > 0 && memory[front-1] == 0){
        while (ptr->next != NULL){
            ptr = ptr->next;
            if ((ptr->addr + ptr->length) == front){
                ptr->length =ptr->length + process->kb;
                if (ptr->next == NULL){
                    ptr->prior->next = NULL;
                }
                else{
                    ptr->prior->next = ptr->next;
                    ptr->next->prior = ptr->prior;
                }
                flag = 1;
                break;
            }
            if(flag)
                break;
        }
    }
    if (rear < 63 && memory[rear+1] == 0){
        while (ptr2->next != NULL){
            ptr2 = ptr2->next;
            if (rear+1 == ptr2->addr){
                if(flag){
                    ptr->length += ptr2->length;
                    if(ptr2->next == NULL){
                        ptr2->prior->next = NULL;
                    } else {
                        ptr2->prior->next = ptr2->next;
                        ptr2->next->prior = ptr2->prior;
                    }
                    free(ptr2);
                    break;
                } else {
                    ptr2->addr = front;
                    ptr2->length =ptr2->length + process->kb;
                    if(ptr2->next == NULL){
                        ptr2->prior->next = NULL;
                    } else {
                        ptr2->prior->next = ptr2->next;
                        ptr2->next->prior = ptr2->prior;
                    }
                    flag = 2;
                    break;
                }
            }
        }
    }
    if (flag == 1)
        insert_free_blk(fb,ptr);
    else if (flag == 2)
        insert_free_blk(fb,ptr2);
    else {
        tmp = (struct Free_blk *)malloc(sizeof(struct Free_blk));
        tmp->addr = front;
        tmp->length = process->kb;
        insert_free_blk(fb,tmp);
    }

    int i;
    for (i = front;i < rear + 1;i++){
        memory[i] = 0;
    }
}

int
add_mem(struct Free_blk *fb,struct PCB *process)
{
    int tmp_addr;
    struct Free_blk *ptr = fb;
    if(process->kb > 64){
        mvprintw(30,60,"Out of Memory");
        refresh();
        sleep(2);
        return -1;
    }
    while (ptr->next != NULL){
        ptr = ptr->next;
        if ((ptr->length - process->kb) <= 2 && ptr->length > process->kb){
            process->kb = ptr->length;
        }
        if(ptr->length > process->kb){
            process->p_addr = ptr->addr;
            ptr->addr = process->p_addr + process->kb;
            ptr->length -= process->kb;
            return 0;
        } else if (ptr->length == process->kb){
            process->p_addr = ptr->addr;
            if (ptr->next == NULL)
                ptr->prior->next = NULL;
            else{
                ptr->prior->next = ptr->next;
                ptr->next->prior = ptr->prior;
                free(ptr);
            }
            return 0;
        }

    }
    mvprintw(30,60,"Memory Full now");
    refresh();
    sleep(2);
    return -1;
}

/*功能实现区*/

void
ready_list_adder(struct PCB *ready,struct PCB *blocking,struct Free_blk *fb)
{
    int screen_row = MAINLINE;
    int screen_col = 10;
    char buf[10];

    clean_all_screen(ready,blocking,fb);

    struct PCB *pro;
    pro = (struct PCB *)malloc(sizeof(struct PCB));

    mvprintw(screen_row,screen_col,"Enter this process's name: ");
    getstring(pro->name);
    screen_row++;
    mvprintw(screen_row,screen_col,"Enter this process's block: ");
    getstring(buf);
    pro->kb = str_to_int(buf);
    if (add_mem(fb,pro) < 0){
        free(pro);
        return;
    } else {
        pro->pid = pid_count++;
        add(ready,pro);
        enter_mem(pro);
    }
}

void 
set_process_blocking_or_time_slice(struct PCB *running,struct PCB *list)
{
    struct PCB *process;
    if (!is_empty(running)){
        process = running->next;
        running->next = NULL;
        add(list,process);
    }
}

void 
free_running_process(struct Free_blk *fb,struct PCB *running)
{
    if(!is_empty(running)){
        free_men(fb,running->next);
        free(running->next);
        running->next = NULL;
        strcpy(current_process,"\0");
    }
}

void 
wake_blocked_process(struct PCB *ready,struct PCB *blocking)
{
    struct PCB *process;
    if(!is_empty(blocking)){
        process = blocking->next;
        blocking->next = blocking->next->next;
        add(ready,process);
    }
}

void
free_all_fb(struct Free_blk *fb)
{
    struct Free_blk *tmp = fb,*ptr,*new;
    tmp = tmp->next;
    while (tmp->next != NULL){
        ptr = tmp->next;
        free(tmp);
        tmp = ptr;
    }
    new = (struct Free_blk *)malloc(sizeof(struct Free_blk));
    fb->next = new;
    new->prior = fb;
    new->addr = 0;
    new->length = 64;
    new->next = NULL;

    int i = 0;
    for(; i < 64; i++){
        memory[i] = 0;
    }
}

void 
free_all_process(struct PCB *head)
{
    struct PCB *tmp = head,*ptr;
    if(!is_empty(head)){
        tmp = tmp->next;
        while(tmp->next != NULL){
            ptr = tmp->next;
            free(tmp);
            tmp = ptr;
        }
        head->next = NULL;
    }
}
