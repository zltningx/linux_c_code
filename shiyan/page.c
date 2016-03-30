/*************************************************************************
	> File Name: page.c
	> Author: 郑凌弢
	> Mail: zltningx@outlook.com
	> Created Time: 2016年03月23日 星期三 10时58分37秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<curses.h>

#define MAX_STRING 80

/*Date AREA*/
typedef struct Page {
    int page_num;
    int block_num;
    int status;
}Page_array;

typedef struct Memory {
    int m_page_num;
}MemList;

int 
main()
{

}

int
init_bitmap(int bit_array[8][8])
{
    int i = 0,j = 0;
    for (;i < 8;i++){
        for (;j < 8;j++){
            bit_array[i][j] = rand() % 2;
        }
    }
}
