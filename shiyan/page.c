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

typedef struct pagelist {
    int s_page_num;
    struct pagelist *next;
}page_list;

struct current_messge {
    int physical_addr;
    int change_num;
    int miss_page_num;
};
/*End of Data*/

static struct current_messge fifo;
static struct current_messge lru;
static struct current_messge opt;


/* 原型声明 */
int init_bitmap(int bit_array[][8]); MemList *init_mem(MemList *memlist);
void Init_page(Page_array page[],int page_size); void append(MemList *memlist,int page_num); int  pop(MemList *memlist);
void draw_clear(Page_array *page_fifo,Page_array *page_lru,Page_array *page_opt,MemList *memlist_fifo,MemList *memlist_lru,MemList *memlist_opt,int page_size);
int get_page_size();
int get_mem_size();
void get_null();
void change_mem(Page_array page[], MemList *memlist,int page_num);
int read_bit_chart(int bit_array[][8]);
int loop_get_addr();
int re_physical_addr(int physical_block,int page_in_addr,int block_size);
void lake_page(Page_array page[],MemList *memlist,int page_num);
void init_message();
void f_main_func(Page_array *page_fifo,Page_array *page_lru,MemList  *memlist_fifo,MemList *memlist_lru,int page_num,int page_in_addr,int memory_size,int bit_array[][8],int block_size);
page_list * init_page_list(page_list *pl);
void p_append(page_list *pl,int num);
int p_pop(page_list *pl);
void opt_change_mem(Page_array *page_opt,MemList *memlist_opt,page_list *pl,int page_num,int memory_size);
void s_main_func(Page_array *page_opt,MemList *memlist_opt,page_list *pl,int memory_size,int getnum);
void draw_page_list(page_list *pl);
int get_page_block_size();
/* End of 原型声明*/

/*Main*/
int 
main(void)
{
    //def
    int bit_array[8][8];
    MemList *memlist_fifo,*memlist_lru,*memlist_opt;
    Page_array page_fifo[MAX],page_lru[MAX],page_opt[MAX];
    page_list *pl;
    int page_size,memory_size;
    int addr,page_num,page_in_addr;
    int coun;
    int block_size;

    //init
    init_bitmap(bit_array);
    memlist_fifo = init_mem(memlist_fifo);
    memlist_lru = init_mem(memlist_lru);
    memlist_opt = init_mem(memlist_opt);
    pl = init_page_list(pl);
    init_message();
    initscr();

    //geter
    page_size = get_page_size();
    Init_page(page_fifo,page_size);
    Init_page(page_lru,page_size);
    Init_page(page_opt,page_size);
    draw_clear(page_fifo,page_lru,page_opt,memlist_fifo,memlist_lru,memlist_opt,page_size);
    memory_size = get_mem_size();
    block_size = get_page_block_size();

    //Loop
    do{
        draw_clear(page_fifo,page_lru,page_opt,memlist_fifo,memlist_lru,memlist_opt,page_size);
        addr = loop_get_addr();
        if (addr == -1) {
            break;
        }
        page_in_addr = addr % (1024 * block_size);
        page_num = addr / (1024 * block_size);

        if (page_num >= page_size){
            mvprintw(6,40,"%s","Out Of Memory!");
            refresh();
            sleep(2);
            continue;
        }
        p_append(pl,page_num);
        //LRU  && FIFO
        f_main_func(page_fifo,page_lru,memlist_fifo,memlist_lru,page_num,page_in_addr,memory_size,bit_array,block_size);

    } while(1);

    //New loop
    do {
        draw_clear(page_fifo,page_lru,page_opt,memlist_fifo,memlist_lru,memlist_opt,page_size);
        draw_page_list(pl);
        get_null();
        if (pl->next != NULL){
            if ((coun = p_pop(pl)) != -1) {
                s_main_func(page_opt,memlist_opt,pl,memory_size,coun);
            }
        } 
    } while (1);

    endwin();
    exit(0);
}
/*End of Main*/

void 
s_main_func(Page_array *page_opt,MemList *memlist_opt,page_list *pl,int memory_size,int getnum)
{
    int page_num;
    page_num = getnum;
        if (page_opt[page_num].status == 1) {
            opt.change_num++;
        }
        if (page_opt[page_num].status == 0 && memlist_opt->m_page_num < memory_size) {
            opt.change_num++;
            opt.miss_page_num++;
            page_opt[page_num].status = 1;
            append(memlist_opt,page_num);
        }
        if (page_opt[page_num].status == 0) {
            opt.change_num++;
            opt.miss_page_num++;
            opt_change_mem(page_opt,memlist_opt,pl,page_num,memory_size);
        }
}

int 
find_last_u(page_list *pl,int m_page_num)
{
    int i = 0;
    page_list *p = pl;
    if (p->next == NULL) {
        return -1;
    }
    while (p->next != NULL) {
        p = p->next;
        i++;
        if (p->s_page_num == m_page_num) {
            return i;
        }
    }
    return 65535;
}

void
opt_change_mem(Page_array *page_opt,MemList *memlist_opt,page_list *pl,int page_num,int memory_size)
{
    int max;
    int current = 0;
    MemList *p = memlist_opt,*max_p;
    while (p->next != NULL) {
        p = p->next;
        max = find_last_u(pl,p->m_page_num);
        if (current < max) {
            current = max;
            max_p = p;
        }
    }
    if (max == -1)
        max_p = p;

    page_opt[max_p->m_page_num].status = 0;
    page_opt[page_num].status = 1;
    max_p->m_page_num = page_num;
}

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

page_list *
init_page_list(page_list *pl)
{
    pl = (page_list *)malloc(sizeof(struct pagelist));
    pl->s_page_num = 0;
    pl->next = NULL;
    return pl; 
}

void 
p_append(page_list *pl,int num)
{
    page_list *new,*p = pl;
    while (p->next != NULL) {
        p = p->next;
    }
    new = (page_list *)malloc(sizeof(struct pagelist));
    p->next = new;
    new->next = NULL;

    new->s_page_num = num;
}

int
p_pop(page_list *pl)
{
    int tmp;
    page_list *p = pl;
    if (p->next != NULL) {
        p = p->next;
        pl->next = p->next;
        tmp = p->s_page_num;
    } else return -1;
    free(p);
    return tmp;
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
    } else return -1;
    free(p);
    return tmp;
}

/* End of Init */

/* Draw AREA*/
void 
draw_page_fifo(Page_array page_fifo[],int page_num)
{
    int i;
    mvprintw(9,5,"%s","Page for FIFO");
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
    mvprintw(9,30,"%s","Page for LRU");
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
draw_page_opt(Page_array page_opt[],int page_num)
{
    int i;
    mvprintw(29,50,"%s","Page for OPT");
    for (i = 0;i < page_num;i++){
        mvprintw(30 + i,50,"%d: ",i,page_opt[i].page_num);
        /*
        if (page_lru[i].block_num != 0){
            mvprintw(30 + i,50,"%d",page_lru[i].block_num);
        } else {
            mvprintw(30 + i,60,"%s","  ");
        }
        */
        mvprintw(30 + i,55,"%d",page_opt[i].status);
    }
    refresh();
}

void
draw_fifo(MemList *memlist_fifo)
{
    int i = 0;
    MemList *p = memlist_fifo;
    mvprintw(41,10,"%s","Mem for FIFO");
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
    mvprintw(41,30,"%s","Mem for LRU");
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
draw_opt(MemList *memlist_opt)
{
    int i = 0;
    MemList *p = memlist_opt;
    mvprintw(41,65,"%s","Mem for OPT");
    if (!(memlist_opt->next == NULL)) {
        while (p->next != NULL) {
            p = p->next;
            mvprintw(30 - i,65,"%d: %d",i,p->m_page_num);
            i++;
        }
    }
    refresh();
}

void
draw_message()
{
    float tmp;
    mvprintw(9,50,"FIFO message");
    mvprintw(10,50,"%s%x","Local:",fifo.physical_addr);
    mvprintw(11,50,"%s%d","Visited:",fifo.change_num);
    mvprintw(12,50,"%s%d","Absent:",fifo.miss_page_num);
    if (fifo.change_num != 0){
        tmp = fifo.change_num;
        mvprintw(13,50,"%s%f%s","Rate:",(fifo.miss_page_num /tmp)*100,"%");
    }

    mvprintw(9,80,"LRU message");
    mvprintw(10,80,"%s%x","Local:",lru.physical_addr);
    mvprintw(11,80,"%s%d","Visited:",lru.change_num);
    mvprintw(12,80,"%s%d","Absent:",lru.miss_page_num);
    if (lru.change_num != 0){
        tmp = lru.change_num;
        mvprintw(13,80,"%s%f%s","Rate:",(lru.miss_page_num /tmp)*100,"%");
    }

    mvprintw(9,110,"OPT message");
    mvprintw(11,110,"%s%d","Visited:",opt.change_num);
    mvprintw(12,110,"%s%d","Absent:",opt.miss_page_num);
    if (opt.change_num != 0){
        tmp = opt.change_num;
        mvprintw(13,110,"%s%f%s","Rate:",(opt.miss_page_num /tmp)*100,"%");
    }
    refresh();
}

void 
draw_page_list(page_list *pl)
{
    page_list *p = pl;
    int i = 0;
    while (p->next != NULL) {
        p = p->next;
        mvprintw(20,50 + i,"%d",p->s_page_num);
        i += 2;
    }
    refresh();
}

void
draw_clear(Page_array *page_fifo,Page_array *page_lru,Page_array *page_opt,MemList *memlist_fifo,MemList *memlist_lru,MemList *memlist_opt,int page_size)
{
    clear();

    mvprintw(2,20,"%s","Page Change System");

    draw_page_fifo(page_fifo,page_size);
    draw_page_lru(page_lru,page_size);
    draw_page_opt(page_opt,page_size);
    draw_fifo(memlist_fifo);
    draw_lru(memlist_lru);
    draw_opt(memlist_opt);
    draw_message();
    
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
re_physical_addr(int physical_block,int page_in_addr,int block_size)
{
    return physical_block * 1024 * block_size + page_in_addr;
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
void
get_null()
{
    char buf[20];
    mvprintw(6,10,"Enter to Continue show Opt# ");
    getstring(buf);
    refresh();
}
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
get_page_block_size()
{
    int screen_row = 6;
    int screen_col = 10;
    int size;
    char buf[20];

    mvprintw(screen_row,screen_col,"Enter Page Block size>>> ");
    getstring(buf);
    size = str_to_int(buf);
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
    if (buf[0] == '-')
        return -1;
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

/*Main Function*/
void 
f_main_func(Page_array *page_fifo,Page_array *page_lru,MemList  *memlist_fifo,MemList *memlist_lru,int page_num,int page_in_addr,int memory_size,int bit_array[][8],int block_size)
{
    //命中
    if (page_fifo[page_num].status == 1){
        fifo.physical_addr = re_physical_addr(page_fifo[page_num].block_num,page_in_addr,block_size);
        fifo.change_num += 1;
    }
    if (page_lru[page_num].status == 1) {
        lru.physical_addr = re_physical_addr(page_lru[page_num].block_num,page_in_addr,block_size);
        lru.change_num += 1;
        change_mem(page_lru, memlist_lru,page_num);
    }
    //内存未满从位视图读块号
    if (page_fifo[page_num].status == 0 && memlist_fifo->m_page_num < memory_size && page_lru[page_num].status == 0 && memlist_lru->m_page_num < memory_size)   {
        page_fifo[page_num].status = 1;
        page_lru[page_num].status = 1;
        page_fifo[page_num].block_num = read_bit_chart(bit_array);
        page_lru[page_num].block_num = page_fifo[page_num].block_num;
        append(memlist_fifo,page_num);
        append(memlist_lru,page_num);

        fifo.physical_addr = re_physical_addr(page_fifo[page_num].block_num,page_in_addr,block_size);
        lru.physical_addr = re_physical_addr(page_lru[page_num].block_num,page_in_addr,block_size);
        fifo.change_num += 1;
        fifo.miss_page_num += 1;
        lru.change_num += 1;
        lru.miss_page_num += 1;
    }
    //未命中
    if (page_fifo[page_num].status == 0) {
        fifo.physical_addr = re_physical_addr(memlist_fifo->m_page_num,page_in_addr,block_size);
        fifo.change_num += 1;
        fifo.miss_page_num += 1;
        page_fifo[page_num].status = 1;
        lake_page(page_fifo,memlist_fifo,page_num);
    }
    if (page_lru[page_num].status == 0) {
        lru.physical_addr = re_physical_addr(memlist_lru->m_page_num,page_num,block_size);
        lru.change_num += 1;
        lru.miss_page_num += 1;
        page_lru[page_num].status = 1;
        lake_page(page_lru,memlist_lru,page_num);   
    }
}
/*End of Main Function*/
