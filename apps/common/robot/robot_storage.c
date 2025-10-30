#include "robot_main.h"
#include "robot_control.h"
#include "robot_storage.h"
#include "robot_debug.h"
#include "robot_uart.h"
#include "robot_adapter.h"


extern void flash_read_write_init(void);
int flash_write_buff(u8 *buff, u32 addr, u32 len);
int flash_read_buff(u8 *buff, u32 addr, u32 len);



void robot_storage_init()
{
    flash_read_write_init();
}

void robot_storage_large_page_write(int page,uint8_t *data,int sz)
{
    if(sz>4000)
        sz = 4000;
    //DBG_PRINT("robot_storage_large_page_write addr = ",
    //          ROBOT_STORAGE_LARGE_ADDR_ST+page*ROBOT_STORAGE_LARGE_BLOCK_SZ);
    flash_write_buff(data,
                     ROBOT_STORAGE_LARGE_ADDR_ST+page*ROBOT_STORAGE_LARGE_BLOCK_SZ,
                     sz);
}

int robot_storage_large_page_read(int page,uint8_t *data)
{
    //DBG_PRINT("robot_storage_large_page_read addr = ",
    //          ROBOT_STORAGE_LARGE_ADDR_ST+page*ROBOT_STORAGE_LARGE_BLOCK_SZ);
    flash_read_buff(data,
                     ROBOT_STORAGE_LARGE_ADDR_ST+page*ROBOT_STORAGE_LARGE_BLOCK_SZ,
                     4000);
    return 0;
}

void robot_storage_small_page_write(int page,uint8_t *data,int sz)
{
    if(sz>1000)
        sz = 1000;
    //DBG_PRINT("robot_storage_large_page_write addr = ",
    //          ROBOT_STORAGE_SMALL_ADDR_ST+page*ROBOT_STORAGE_SMALL_BLOCK_SZ);
    flash_write_buff(data,
                     ROBOT_STORAGE_SMALL_ADDR_ST+page*ROBOT_STORAGE_SMALL_BLOCK_SZ,
                     sz);

}

int robot_storage_small_page_read(int page,uint8_t *data)
{
    //DBG_PRINT("robot_storage_small_page_read addr = ",
    //          ROBOT_STORAGE_SMALL_ADDR_ST+page*ROBOT_STORAGE_SMALL_BLOCK_SZ);
    flash_read_buff(data,
                     ROBOT_STORAGE_SMALL_ADDR_ST+page*ROBOT_STORAGE_SMALL_BLOCK_SZ,
                     1000);
}

extern uint8_t adata[200][20];
extern uint16_t aid;
void robot_storage_adata_write(int page,int fsz)
{

    if((page>=ROBOT_STORAGE_LARGE_PAGE_ST)&&(page<ROBOT_STORAGE_LARGE_PAGE_ST+ROBOT_STORAGE_LARGE_PAGE_SZ))
    {
        if(fsz==0)
            fsz = 200;
        return robot_storage_large_page_write(page-ROBOT_STORAGE_LARGE_PAGE_ST,(uint8_t*)adata,fsz*20);
    }
    if((page>=ROBOT_STORAGE_SMALL_PAGE_ST)&&(page<ROBOT_STORAGE_SMALL_PAGE_ST+ROBOT_STORAGE_SMALL_PAGE_SZ))
    {
        if(fsz==0)
            fsz = 50;
        return robot_storage_small_page_write(page-ROBOT_STORAGE_SMALL_PAGE_ST,(uint8_t*)adata,fsz*20);

    }

}

//一次性读入整个block的数据，然后通过checksum查看有效数据的大小，如果读取速度有问题，就优化

int robot_storage_adata_read(int page)
{
    int i;
    //DBG_PRINT("adata clean",3);
    memset(adata,0,sizeof(adata));
    DBG_PRINT("page = ",page);
    DBG_PRINT("adata =",(uint8_t*)adata);
    aid = 0;
    if((page>=ROBOT_STORAGE_LARGE_PAGE_ST)&&(page<ROBOT_STORAGE_LARGE_PAGE_ST+ROBOT_STORAGE_LARGE_PAGE_SZ))
        robot_storage_large_page_read(page-ROBOT_STORAGE_LARGE_PAGE_ST,(uint8_t*)adata);
    else if((page>=ROBOT_STORAGE_SMALL_PAGE_ST)&&(page<ROBOT_STORAGE_SMALL_PAGE_ST+ROBOT_STORAGE_SMALL_PAGE_SZ))
       robot_storage_small_page_read(page-ROBOT_STORAGE_SMALL_PAGE_ST,(uint8_t*)adata);
    else
        return 0;
    for(i=0;i<200;i++)
    {
        if(adata[i][19]!=robot_data_checksum_flash(adata[i],19))
            break;
//        DBG_PRINT("adata[i][19] = ",adata[i][19]);
//        DBG_PRINT("robot_data_checksum_flash(adata[i],19) = ",robot_data_checksum_flash(adata[i],19));
    }
    aid = i;
    DBG_PRINT("aid = ",aid);
    return aid;
}

void robot_storage_adata_set(uint8_t data[],int sz,int idx)
{
    int i;
    if(sz>19)
        sz = 19;
    for(i=0;i<19;i++)
    {
        if(i<sz)
            adata[idx][i] = data[i];
        else
            adata[idx][i] = 0;
    }
    adata[idx][19] = robot_data_checksum_flash(adata[idx],19);
}

void robot_storage_adata_get(uint8_t data[],int sz,int idx)
{
    int i;
    if(sz>19)
        sz = 19;
    for(i=0;i<sz;i++)
        data[i] = adata[idx][i];
}


int robot_storage_offset2page(int offset)
{
    if(offset < ROBOT_STORAGE_SMALL_PAGE_SZ)
        return ROBOT_STORAGE_SMALL_PAGE_ST + offset;
    if(offset < ROBOT_STORAGE_LARGE_PAGE_SZ + ROBOT_STORAGE_ALL_PAGE_SZ)
        return ROBOT_STORAGE_LARGE_PAGE_ST + (offset - ROBOT_STORAGE_SMALL_PAGE_SZ);
    return -1;
}

//for test
int test_seed = 0;

void robot_storage_test_seedroll()
{
    test_seed++;
    if(test_seed>20)
        test_seed = 1;
    DBG_PRINT("robot_storage_test_seedroll seed = ",test_seed);
}
void robot_storage_test_write()
{
    int i,j,k;
    for(i=0;i<200;i++)
    {
        for(j=0;j<20;j++)
            adata[i][j] = test_seed+i+j;
    }
    for(k=0;k<ROBOT_STORAGE_LARGE_PAGE_SZ;k++)
    {
        DBG_PRINT("write fidx = L",k);
        robot_storage_adata_write(ROBOT_STORAGE_LARGE_PAGE_ST+k,0);
    }

    for(k=0;k<ROBOT_STORAGE_SMALL_PAGE_SZ;k++)
    {
        DBG_PRINT("write fidx = S",k);
        robot_storage_adata_write(ROBOT_STORAGE_SMALL_PAGE_ST+k,0);
    }

    DBG_PRINT("robot_storage_test_write done seed = ",test_seed);
}

int robot_storage_test_read()
{
    int i,j,k;
    int seed = -2;
    for(k=0;k<ROBOT_STORAGE_LARGE_PAGE_SZ;k++)
    {

        robot_storage_adata_read(ROBOT_STORAGE_LARGE_PAGE_ST+k);
        if(seed == -2)
        {
            seed = adata[0][0];
            DBG_PRINT("robot_storage_test_read get_seed = ",seed);
        }

        if(seed == -1)
            break;
        for(i=0;i<200;i++)
        {
            if(seed == -1)
                break;
            for(j=0;j<20;j++)
            {
                if(adata[i][j]!=seed+i+j)
                {
                    seed = -1;
                    break;
                }
            }
            //if(seed == -1)
            //    DBG_PRINT("robot_storage_test_read fidx = L-",k);
            //else
            //    DBG_PRINT("robot_storage_test_read ok seed = ",seed);
        }
    }
    for(k=0;k<ROBOT_STORAGE_SMALL_PAGE_SZ;k++)
    {
        if(seed == -1)
            break;
        robot_storage_adata_read(ROBOT_STORAGE_SMALL_PAGE_ST+k);
        for(i=0;i<50;i++)
        {
            if(seed == -1)
                break;
            for(j=0;j<20;j++)
            {
                if(adata[i][j]!=seed+i+j)
                {
                    seed = -1;
                    break;
                }
            }
        }
        //if(seed == -1)
        //    DBG_PRINT("robot_storage_test_read fidx = S-",k);
        //else
        //    DBG_PRINT("robot_storage_test_read ok seed = ",seed);
    }
    if(seed>=0)
        test_seed = seed;
    DBG_PRINT("robot_storage_test_read seed = ",seed);
    return seed;
}


