/*************************************************************************
	> File Name: page.c
	> Author: 郑凌弢
	> Mail: zltningx@outlook.com
	> Created Time: 2016年03月23日 星期三 10时58分37秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<curses.h>

#define MAX 80

/*Data AREA*/

typedef struct Page {
    int page_num;
    int block_num;
    int status;
}Page_array;

typedef struct Memory {
    int m_page_num;
    struct Memory *prior;
    struct Memory *next;
}MemList;

struct current_messge {
    int physical_addr;
    int change_num;
    int miss_page_num;
};
/*End of Data*/

static struct current_messge fifo;
static struct current_messge lru;


/* 原型声明 */
int init_bitmap(int bit_array[][8]); MemList *init_mem(MemList *memlist);
void Init_page(Page_array page[],int page_size); void append(MemList *memlist,int page_num); int  pop(MemList *memlist);
void draw_clear(Page_array page_fifo[],Page_array page_lru[],MemList *memlist_fifo,MemList *memlist_lru,int page_size);
void draw_page_fifo(Page_array page_fifo[],int page_num);
void draw_page_lru(Page_array page_lru[],int page_num);
void draw_fifo(MemList *memlist_fifo);
void draw_lru(MemList *memlist_lru);
int get_page_size();
int get_mem_size();
void change_mem(Page_array page[], MemList *memlist,int page_num);
int read_bit_chart(int bit_array[][8]);
int loop_get_addr();
int re_physical_addr(int physical_block,int page_in_addr);
void lake_page(Page_array page[],MemList *memlist,int page_num);
void init_message();
void draw_message();
/* End of 原型声明*/

/*Main*/
int 
main(void)
{
    //def
    int bit_array[8][8];
    MemList *memlist_fifo,*memlist_lru;
    Page_array page_fifo[MAX],page_lru[MAX];
    int page_size,memory_size;
    int addr,page_num,page_in_addr;

    //init
    init_bitmap(bit_array);
    memlist_fifo = init_mem(memlist_fifo);
    memlist_lru = init_mem(memlist_lru);
    init_message();
    initscr();

    //geter
    page_size = get_page_size();
    Init_page(page_fifo,page_size);
    Init_page(page_lru,page_size);
    draw_clear(page_fifo,page_lru,memlist_fifo,memlist_lru,page_size);
    memory_size = get_mem_size();

    //Loop
    do{
        draw_clear(page_fifo,page_lru,memlist_fifo,memlist_lru,page_size);
        addr = loop_get_addr();
        page_in_addr = addr % 1024;
        page_num = addr / 1024;

        if (page_num >= page_size){
            mvprintw(6,40,"%s","Out Of Memory!");
            refresh();
            sleep(2);
            continue;
        }
        //命中
        if (page_fifo[page_num].status == 1){
            fifo.physical_addr = re_physical_addr(page_fifo[page_num].block_num,page_in_addr);
            fifo.change_num += 1;
        }
        if (page_lru[page_num].status == 1) {
            lru.physical_addr = re_physical_addr(page_lru[page_num].block_num,page_in_addr);
            lru.change_num += 1;
            change_mem(page_lru, memlist_lru,page_num);
        }
        //内存未满从位视图读块号
        if (page_fifo[page_num].status == 0 && memlist_fifo->m_page_num < memory_size && page_lru[page_num].status == 0 && memlist_lru->m_page_num < memory_size) {
            page_fifo[page_num].status = 1;
            page_lru[page_num].status = 1;
            page_fifo[page_num].block_num = read_bit_chart(bit_array);
            page_lru[page_num].block_num = page_fifo[page_num].block_num;
            append(memlist_fifo,page_num);
            append(memlist_lru,page_num);

            fifo.physical_addr = re_physical_addr(page_fifo[page_num].block_num,page_in_addr);
            lru.physical_addr = re_physical_addr(page_lru[page_num].block_num,page_in_addr);
            fifo.change_num += 1;
            fifo.miss_page_num += 1;
            lru.change_num += 1;
            lru.miss_page_num += 1;
        }
        //未命中
        if (page_fifo[page_num].status == 0) {
            fifo.physical_addr = re_physical_addr(memlist_fifo->m_page_num,page_in_addr);
            fifo.change_num += 1;
            fifo.miss_page_num += 1;
            page_fifo[page_num].status = 1;
            lake_page(page_fifo,memlist_fifo,page_num);
        }
        if (page_lru[page_num].status == 0) {
            lru.physical_addr = re_physical_addr(memlist_lru->m_page_num,page_num);
            lru.change_num += 1;
            lru.miss_page_num += 1;
            page_lru[page_num].status = 1;
            lake_page(page_lru,memlist_lru,page_num);
        }
    }while(1);

    endwin();
    exit(0);
}
/*End of Main*/

/* Init */
void 
init_message()
{
    fifo.physical_addr = 0;
    fifo.miss_page_num = 0;
    fifo.change_num = 0;
}

int
init_bitmap(int bit_array[][8])
{
    int i = 0,j = 0;
    for (;i < 8;i++){
        for (;j < 8;j++){
            bit_array[i][j] = rand() % 2;
        }
    }
}

MemList *
init_mem(MemList *memlist)
{
    memlist = (MemList *)malloc(sizeof(struct Memory));
    //头节点存队列的数量便于查找
    memlist->m_page_num = 0;
    memlist->next = NULL;
    memlist->prior = NULL;
}


void
Init_page(Page_array page[],int page_size)
{
    int i = 0;
    for (;i < page_size; i++){
        page[i].page_num = i;
        page[i].block_num = 0;
        page[i].status = 0;
    }
}

void
append(MemList *memlist,int page_num)
{
    MemList *p = memlist,*new;
    while (p->next != NULL) {
        p = p->next;
    }
    new = (MemList *)malloc(sizeof(struct Memory));
    p->next = new;
    new->prior = p;
    new->next = NULL;

    new->m_page_num = page_num;
    memlist->m_page_num++;
}

int 
pop(MemList *memlist)
{
    int tmp;
    MemList *p = memlist;
    memlist->m_page_num--;
    if (p->next != NULL) {
        p = p->next;
        memlist->next = p->next;
        p->next->prior = memlist;
        tmp = p->m_page_num;
    }
    free(p);
    return tmp;
}
/* End of Init */

/* Draw AREA*/
void
draw_clear(Page_array page_fifo[],Page_array page_lru[],MemList *memlist_fifo,MemList *memlist_lru,int page_size)
{
    clear();

    mvprintw(2,20,"%s","Page Change System");

    draw_page_fifo(page_fifo,page_size);
    draw_page_lru(page_lru,page_size);
    draw_fifo(memlist_fifo);
    draw_lru(memlist_lru);
    draw_message();
    
    refresh();
}

void 
draw_page_fifo(Page_array page_fifo[],int page_num)
{
    int i;
    for (i = 0;i < page_num;i++){
        mvprintw(10 + i,5,"%d :",i,page_fifo[i].page_num);
        if (page_fifo[i].block_num != 0) {
            mvprintw(10 + i,15,"%d",page_fifo[i].block_num);
        } else {
            mvprintw(10 + i,15,"%s","  ");
        }
        mvprintw(10 + i,20,"%d",page_fifo[i].status);
    }
    refresh();
}

void 
draw_page_lru(Page_array page_lru[],int page_num)
{
    int i;
    for (i = 0;i < page_num;i++){
        mvprintw(10 + i,30,"%d: ",i,page_lru[i].page_num);
        if (page_lru[i].block_num != 0){
            mvprintw(10 + i,40,"%d",page_lru[i].block_num);
        } else {
            mvprintw(10 + i,40,"%s","  ");
        }
        mvprintw(10 + i,45,"%d",page_lru[i].status);
    }
    refresh();
}

void
draw_fifo(MemList *memlist_fifo)
{
    int i = 0;
    MemList *p = memlist_fifo;
    if (!(memlist_fifo->next == NULL)) {
        while (p->next != NULL) {
            p = p->next;
            mvprintw(40 - i,10,"%d: %d",i,p->m_page_num);
            i++;
        }
    }
    refresh();
}

void
draw_lru(MemList *memlist_lru)
{
    int i = 0;
    MemList *p = memlist_lru;
    if (!(memlist_lru->next == NULL)) {
        while (p->next != NULL) {
            p = p->next;
            mvprintw(40 - i,30,"%d: %d",i,p->m_page_num);
            i++;
        }
    }
    refresh();
}

void
draw_message()
{
    mvprintw(10,50,"%d",fifo.physical_addr);
    mvprintw(11,50,"%d",fifo.miss_page_num);
    //mvprintw(12,50,"%f",fifo.miss_page_num /fifo.change_num);
    refresh();
}
/*End of Draw*/

/*Mini Function*/
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

int 
re_physical_addr(int physical_block,int page_in_addr)
{
    return physical_block * 1024 + page_in_addr;
}

int hex_to_dec(char*s)
{
    int i,t;
    int sum=0;
    for(i=0;s[i];i++)
    {
        if(s[i]<='9')t=s[i]-'0';
        else  t=s[i]-'a'+10;
        sum=sum*16+t;
    }
    return sum;
}

int 
read_bit_chart(int bit_array[][8])
{
    int i,j,block_num;
    for (i = 0;i < 8; i++) {
        for (j = 0;j < 8;j++) {
            if (bit_array[i][j] == 0) {
                bit_array[i][j] = 1;
                block_num = i*8+j;
                return block_num;
            }
        }
    }
}
/*End of Mini Function*/

/* Function */
int 
get_page_size()
{
    int screen_row = 6;
    int screen_col = 10;
    int size;
    char buf[20];
    mvprintw(2,20,"%s","Page Change System");

    mvprintw(screen_row,screen_col,"Enter this Process's size>>> ");
    getstring(buf);
    size = str_to_int(buf);
    screen_row++;
    refresh();
    return size;
}

int
get_mem_size()
{
    int screen_row = 6;
    int screen_col = 10;
    int size;
    char buf[20];

    mvprintw(screen_row,screen_col,"Enter memory size>>> ");
    getstring(buf);
    size = str_to_int(buf);
    screen_row++;
    refresh();
    return size;
}

int 
loop_get_addr()
{
    int ADDR;
    char buf[20];
    mvprintw(6,10,"Enter ADDR>>> ");
    getstring(buf);
    ADDR = hex_to_dec(buf);
    refresh();
    return ADDR;
}

void
change_mem(Page_array page[], MemList *memlist,int page_num)
{
    MemList *tmp = memlist;
    while (tmp->next != NULL) {
        tmp = tmp->next;
        if (tmp->m_page_num == page_num) {
            if (tmp->next != NULL) {
                tmp->prior->next = tmp->next;
                tmp->next->prior = tmp->prior;
                append(memlist,page_num);
                memlist->m_page_num--;
                break;
            }
        }
    }
}

void 
lake_page(Page_array page[],MemList *memlist,int page_num)
{
    int num;
    num = pop(memlist);
    page[page_num].block_num = page[num].block_num;
    page[num].status = 0;
    page[num].block_num = 0;
    append(memlist,page_num);
}
/*End of Function*/
