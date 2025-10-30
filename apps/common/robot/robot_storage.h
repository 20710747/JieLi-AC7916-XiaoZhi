#ifndef __ROBOT_STORAGE_H__
#define __ROBOT_STORAGE_H__


//存储空间 共256*1024
//共32页大空间，每个空间4k
//标号为100-131
//共128页小空间，每个空间1k
//标号为200-327
#define ROBOT_STORAGE_LARGE_ADDR_ST  (0)
#define ROBOT_STORAGE_LARGE_BLOCK_SZ (4096)
#define ROBOT_STORAGE_LARGE_PAGE_ST (100)
#define ROBOT_STORAGE_LARGE_PAGE_SZ (32)

#define ROBOT_STORAGE_SMALL_ADDR_ST (ROBOT_STORAGE_LARGE_BLOCK_SZ*ROBOT_STORAGE_LARGE_PAGE_SZ)
#define ROBOT_STORAGE_SMALL_BLOCK_SZ (1024)
#define ROBOT_STORAGE_SMALL_PAGE_ST (200)
#define ROBOT_STORAGE_SMALL_PAGE_SZ (128)
#define ROBOT_SRORAGE_ACTION_ADJUST_PAGE (ROBOT_STORAGE_SMALL_PAGE_ST+ROBOT_STORAGE_SMALL_PAGE_SZ-1)
#define ROBOT_STORAGE_ALL_PAGE_SZ (ROBOT_STORAGE_LARGE_PAGE_SZ+ROBOT_STORAGE_SMALL_PAGE_SZ)

void robot_storage_init();
void robot_storage_adata_write(int page,int fsz);
int robot_storage_adata_read(int page);
int robot_storage_offset2page(int offset);
void robot_storage_adata_set(uint8_t data[],int sz,int idx);
void robot_storage_adata_get(uint8_t data[],int sz,int idx);

void robot_storage_test_seedroll();
void robot_storage_test_write();
int robot_storage_test_read();
#endif
