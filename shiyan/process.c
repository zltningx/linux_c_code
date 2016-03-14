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
#define MAX_BLOCK 1024 * 1024

#define MAINLINE 6

/*Data 区*/

struct PCB {
    char name[10];
    int kb;
    struct PCB *next;
};

char *none_process_menu[] = {
    "Add new process",
    "Draw block mem",
    "Quit",
    "",
    "",
    "",
    0,
};

char *no_running_menu[] = {
    "Add new process",
    "Wake_blocked_process",
    "Clean all Process",
    "Draw block mem",
    "Quit",
    0,
};

char *menu[] = {
    "Add new process",
    "Time slice",
    "Process blocking",
    "Wake up the process",
    "End of the process",
    "Clean all process",
    "Draw block mem",
    "Quit",
    0,
};

//全局
static char current_process[10] = "\0";
static int memory[65];

/*原型区*/

void clean_all_screen(struct PCB *ready,struct PCB *blocking);
int getchoice(char *greet,char *choices[],struct PCB *ready,struct PCB *blocking);
void draw_menu(char *options[], int current_row, int start_screenrow, int start_screencol);
void add(struct PCB *head, struct PCB *process);
struct PCB * creat_process();
struct PCB * creat_head();
void getstring(char *string);
void into_running(struct PCB *ready,struct PCB *running);
int is_empty(struct PCB *head);
void add(struct PCB *head, struct PCB *process);
void ready_list_adder(struct PCB *ready,struct PCB *blocking);
void set_process_blocking_or_time_slice(struct PCB *running,struct PCB *list);
void free_running_process(struct PCB *running);
void wake_blocked_process(struct PCB *ready,struct PCB *blocking);
void print_process_name(struct PCB *head,int col);
void free_all_process(struct PCB *head);
void draw_block(struct PCB *ready,struct PCB *blocking);
void init_memory();
/*main 函数*/

int 
main()
{
    int choice;
    initscr();
    init_memory();
    struct PCB *ready,*blocking,*running;
    ready = creat_head();
    blocking = creat_head();
    running = creat_head();

    do{
        into_running(ready,running);
        if(is_empty(ready) && !is_empty(blocking) && is_empty(running)){
            strcpy(current_process,"\0");
            choice = getchoice("Options:",no_running_menu,ready,blocking);
        } else {
            choice = getchoice("Options:",current_process[0] ? menu : none_process_menu,ready,blocking);
        }
        switch(choice){
            case 'A':
                ready_list_adder(ready,blocking);
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
                free_running_process(running);
                break;
            case 'C':
                free_running_process(running);
                free_all_process(ready);
                free_all_process(blocking);
                break;
            case 'D':
                draw_block(ready,blocking);
                break;
        }
    }while (choice != 'Q');
    endwin();
    exit(EXIT_SUCCESS);
}

/*curses 界面绘制区*/

int
getchoice(char *greet,char *choices[],struct PCB *ready,struct PCB *blocking)
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

    clean_all_screen(ready,blocking);
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
clean_all_screen(struct PCB *ready,struct PCB *blocking)
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
            mvprintw(23 + i,10 + col,"%s",tmp->name);
            mvprintw(23 + i,10 + col + 10,"%d",tmp->kb);
            i++;
        }
    }
}

/*进程功能辅助函数区*/
void
init_memory()
{
    int i = 0;
    for(; i < 65; i++){
        memory[i] = 0;
    }
}

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

void
enter_memory(int size)
{
    if (size < 0)
        return;
}

/*功能实现区*/

void
ready_list_adder(struct PCB *ready,struct PCB *blocking)
{
    int screen_row = MAINLINE;
    int screen_col = 10;
    char buf[10];

    clean_all_screen(ready,blocking);

    struct PCB *pro;
    pro = (struct PCB *)malloc(sizeof(struct PCB));

    mvprintw(screen_row,screen_col,"Enter this process's name: ");
    getstring(pro->name);
    screen_row++;
    mvprintw(screen_row,screen_col,"Enter this process's block: ");
    getstring(buf);
    pro->kb = str_to_int(buf);
    add(ready,pro);
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
free_running_process(struct PCB *running)
{
    if(!is_empty(running)){
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
free_all_process(struct PCB *head)
{
    struct PCB *tmp = head,*ptr;
    if(!is_empty(head)){
        while(tmp->next != NULL){
            ptr = tmp->next;
            free(tmp);
            tmp = ptr;
        }
        head->next = NULL;
    }
}

/*内存回收绘图区*/
void
draw_block(struct PCB *ready,struct PCB *blocking)
{
    WINDOW *box_window_ptr;
    WINDOW *sub_window_ptr;
    int screen_line = 1;

    clean_all_screen(ready,blocking);

    box_window_ptr = subwin(stdscr,13,32,7,1);
    if(!box_window_ptr)
        return;
    box(box_window_ptr,ACS_VLINE,ACS_HLINE);
    sub_window_ptr = subwin(stdscr,11,30,8,2);
    if(!sub_window_ptr)
        return;
    werase(sub_window_ptr);
    touchwin(stdscr);
    clrtoeol();
    refresh();
    sleep(2);
}
