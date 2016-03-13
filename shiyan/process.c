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

/*Data 区*/

struct PCB {
    char name[10];
    struct PCB *next;
};

char *none_process_menu[] = {
    "Add new process",
    "Quit"
    0,
};

char *menu[] = {
    "Add new process",
    "Time slice",
    "Process blocking",
    "Wake up the process",
    "End of the process",
    "Quit",
    0,
};

//目标进程
static char current_process = "\0";

/*原型区*/

void clean_all_screen();
int getchoice(char *greet,char *choices[]);
void draw_menu(char *options[], int current_row, int start_screenrow, int start_screencol);
void add(struct PCB *head, struct PCB *process);
struct PCB * creat_process();
struct PCB * creat_head();
void getstring(char *string);

/*main 函数*/

int 
main()
{
    int choice;
    initscr();
    struct PCB *ready,*blocking,*running;
    ready = creat_head();
    blocking = creat_head;
    running = creat_head;

    do{
        choice = getchoice("Options:",menu);
    }while (choice != 'Q');
    endwin();
    exit(EXIT_SUCCESS);
}

/*curses 界面绘制区*/

int
getchoice(char *greet,char *choices[])
{
    static int screen_row = 0;
    int key;
    char **options = choices;
    int max_row = 0;
    int selected;
    int start_screenrow = 6;
    int start_screencol = 10;

    while (*options){
        options++;
        max_row++;
    }

    if (screen_row >= max_row)
        selected = 0;

    clean_all_screen();
    mvprintw(start_screenrow-2, start_screencol,greet);

    keypad(stdscr,TRUE);
    cbreak();
    noecho();
    key = 0;

    while (key != 'Q' && key != KEY_ENTER && key != '\n'){
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

    if (key == 'Q'){
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
    //mvprintw(start_screenrow + 13, 0,"Current in CPU process is: %s",current_process);

    refresh();

}

void
clean_all_screen()
{
    clear();

    mvprintw(2,20,"%s","Process CTRL System");
    if (current_process[0]){
        mvprintw(22,0,"Current process is: %s",current_process);
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

/*创建进程区*/

struct PCB *
creat_head()
{
    struct PCB *head;
    head = (struct PCB *)malloc(sizeof(struct PCB));
    return head;
}

void
ready_list_adder(struct PCB *ready)
{
    struct PCB *pro;
    pro = (struct PCB *)malloc(sizeof(struct PCB));
    getstring(pro.name);
    add(ready,pro);
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
        return 0
    else 
        return 1
}

void
into_running()
{
    
}
