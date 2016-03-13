/*************************************************************************
	> File Name: CD_v1.0.c
	> Author: 郑凌弢
	> Mail: zltningx@outlook.com
	> Created Time: 2016年03月09日 星期三 13时53分08秒
 ************************************************************************/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<curses.h>
#include<string.h>

#define MAX_STRING 80
#define MAX_ENTRY 1024

#define MESSAGE_LINE 6
#define ERROR_LINE 22
#define Q_LINE 20
#define PROMPT_LINE 18

static char current_cd[MAX_STRING] = "\0";
static char current_cat[MAX_STRING];

const char *title_file = "title.cdb";
const char *tracks_file = "tracks.cdb";
const char *temp_file = "cdb.tmp";

int getchoice(char *greet, char *choices[]);
void draw_menu(char *options[], int highlight, int start_row, int start_col);
void clear_all_screen(void);
int get_confirm(void);
void get_return(void);
void add_record();
void gets_string(char *string);
void insert_title(char *entry);
int get_confirm();
void update_cd();
void remove_cd();
void remove_tracks();

char
*main_menu[] = {
    "add New CD",
    "find CD",
    "count CDs and tracks in the catalog",
    "quit",
    0,
};

char 
*extended_menu[] = {
    "add New CD",
    "find CD",
    "count CDs and tracks in the catalog",
    "list track on current CD",
    "remove current CD",
    "update track information",
    "quit",
    0,
}; 

int
main(void)
{
    int choice;
    initscr();

    do{
        choice = getchoice("Options: ",current_cd[0] ? extended_menu : main_menu);
        switch(choice){
            case 'q': break;
            case 'a':
                add_record();
                break;
            case 'c':
                break;
            case 'f':
                break;
            case 'l':
                break;
            case 'r':
                remove_cd();
                break;
            case 'u':
                update_cd();
                break;
            default:
                break;
        }
    }while (choice != 'q');

    endwin();
    exit(EXIT_SUCCESS);
}

int 
getchoice(char *greet, char *choices[])
{
    static int select_row = 0;
    int key;
    char **options = choices;
    int max_row = 0;
    int selected;
    int start_screenrow = MESSAGE_LINE,start_screencol = 10;

    /*计算最大行*/
    while (*options){
        max_row++;
        options++;
    }
    /*循环选择*/
    if(select_row >= max_row)
        select_row = 0;

    /*调用清屏函数*/
    clear_all_screen();
    mvprintw(start_screenrow-2,start_screencol,greet);
    keypad(stdscr,TRUE);
    cbreak();
    noecho();
    key = 0;
    while (key != 'q' && key != KEY_ENTER && key != '\n'){
        if (key == KEY_UP){
            if(select_row == 0)
                select_row = max_row - 1;
            else
                select_row--;
        }
        if (key == KEY_DOWN){
            if(select_row == (max_row - 1))
                select_row = 0;
            else
                select_row++;
        }
        selected = *choices[select_row];
        /*菜单着色*/
        draw_menu(choices,select_row,start_screenrow,start_screencol);
        key = getch();
    }
    keypad(stdscr,FALSE);
    nocbreak();
    echo();

    if (key == 'q'){
        selected = 'q';
    }
    return (selected);
}

void 
draw_menu(char *options[], int current_highlight, int start_row, int start_col)
{
/*option_ptr 作为循环结束判断标识 而txt_ptr 接受菜单*/
    int current_row = 0;
    char **option_ptr;
    char *txt_ptr;
    option_ptr = options;
    
    while (*option_ptr) {
        if (current_row == current_highlight)
            attron(A_STANDOUT);
        txt_ptr = options[current_row];
        //txt_ptr++;

        mvprintw(start_row + current_row, start_col,"%s",txt_ptr);
        if (current_row == current_highlight)
            attroff(A_STANDOUT);
        current_row++;
        option_ptr++;
    }
    mvprintw(start_row + current_row + 3,start_col,"Move highlight then press return");

    refresh();
}

void 
clear_all_screen()
{
/*重新绘制界面*/
    clear();

    mvprintw(2,20,"%s","CD Player");
    if (current_cd[0]){
        mvprintw(ERROR_LINE,0,"current CDs: %s: %s\n",current_cat,current_cd);
    }

    refresh();
}

void
add_record()
{
    char catalog_number[MAX_STRING];
    char cd_title[MAX_STRING];
    char cd_type[MAX_STRING];
    char cd_artist[MAX_STRING];
    char cd_entry[MAX_STRING];

    int screenrow = MESSAGE_LINE;
    int screencol = 10;

    clear_all_screen();
    mvprintw(screenrow,screencol,"Enter new CD details");
    screenrow += 2;

    mvprintw(screenrow,screencol,"Catalog Number: ");
    gets_string(catalog_number);
    screenrow++;
    mvprintw(screenrow,screencol,"      CD Title: ");
    gets_string(cd_title);
    screenrow++;
    mvprintw(screenrow,screencol,"       CD Type: ");
    gets_string(cd_type);
    screenrow++;
    mvprintw(screenrow,screencol,"        Artist: ");
    gets_string(cd_artist);
    screenrow++;

    mvprintw(PROMPT_LINE - 2, 5, "About to add this new entry:");
    sprintf(cd_entry,"%s,%s,%s,%s",catalog_number,cd_title,cd_type,cd_artist);
    mvprintw(PROMPT_LINE, 5, "%s",cd_entry);

    refresh();

    move(PROMPT_LINE,0);
    if (get_confirm()){
        insert_title(cd_title);
        strcpy(current_cd,cd_title);
        strcpy(current_cat,catalog_number);
    }
}

void 
gets_string(char *string)
{
    int len;

    /*wgetnstr 从stdscr 读取最大MAX_STRING个字节到 buf - string里*/
    wgetnstr(stdscr,string,MAX_STRING);
    len = strlen(string);

    if(len > 0 && string[len-1] == '\n')
        string[len-1] = '\0';
}

void
insert_title(char *entry)
{
    FILE *fp;
    if ((fp = fopen(title_file,"a")) == NULL){
        mvprintw(ERROR_LINE,0,"Cannot open title file");
    } else {
        //加入\n另起一行.fgets函数将会读到\n符号停止,这样便可以顺利读取一行
        fprintf(fp,"%s\n",entry);
        fclose(fp);
    }
}
int
get_confirm()
{
    int confirmed = 0;
    char yn;
    mvprintw(Q_LINE,5,"Are you sure? ");
    clrtoeol();
    refresh();

    cbreak();
    yn = getch();
    if(yn == 'Y' || yn == 'y'){
        confirmed = 1;
    }
    nocbreak();

    if(!confirmed){
        mvprintw(Q_LINE,1,"    Cancelled");
        clrtoeol();
        refresh();
        sleep(1);
    }
    return confirmed;
}


void 
update_cd()
{
    FILE *tracks_fp;
    char track_name[MAX_STRING];
    int len;
    int track = 1;
    int screen_line = 1;
    WINDOW *box_window_ptr;
    WINDOW *sub_window_ptr;

    clear_all_screen();
    mvprintw(PROMPT_LINE, 0, "Re-entering tracks for CD.");
    if (!get_confirm())
        return;
    move(PROMPT_LINE, 0);
    clrtoeol();

    remove_tracks();

    mvprintw(PROMPT_LINE, 0, "Enter a blank tracks for CD. ");
    tracks_fp = fopen(tracks_file,"a");

}

void
remove_cd()
{
    FILE *titles_fp,*temp_fp;
    char entry[MAX_ENTRY];
    int cat_length;

    if (current_cd[0] == '\0')
        return;
    
    clear_all_screen();
    mvprintw(PROMPT_LINE, 0, "About to remove CD %s: %s. ",current_cat,current_cd);
    if(!get_confirm())
        return;

    cat_length = strlen(current_cat);

    titles_fp = fopen(title_file,"r");
    temp_fp = fopen(temp_file,"w");

    /*fgets 读到\n结束 相当与python 的readline*/
    while (fgets(entry,MAX_ENTRY,titles_fp)){
        if (strncmp(current_cat,entry,cat_length) != 0)
            fputs(entry,temp_fp);
    }
    fclose(titles_fp);
    fclose(temp_fp);

    unlink(title_file);
    rename(temp_file,title_file);

    remove_tracks();

    current_cd[0] = '\0';
}

void
remove_tracks()
{
    FILE *tracks_fp, *temp_fp;
    char entry[MAX_STRING];
    int cat_length;

    if (current_cd[0] == '\0')
        return;

    cat_length = strlen(current_cat);

    if ((tracks_fp = fopen(tracks_file,"r")) == (FILE *)NULL)
        return;

    temp_fp = fopen(temp_file,"w");

    while (fgets(entry,MAX_STRING,tracks_fp)){
        if (strncmp(current_cat,entry,cat_length) != 0)
            fputs(entry,temp_fp);
    }

    fclose(tracks_fp);
    fclose(temp_fp);

    unlink(tracks_file);
    rename(temp_file,tracks_file);
}
