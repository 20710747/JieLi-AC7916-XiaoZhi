#include "gtest/gtest.h"
#include "csv_file.h"
#include "tkc/utils.h"

TEST(csv_file, row) {
  csv_row_t r;

  csv_row_init(&r, (char*)"aaa\0", 4, FALSE);
  ASSERT_EQ(csv_row_count_cols(&r), 1);
  ASSERT_STREQ(csv_row_get(&r, 0), "aaa");
  ASSERT_EQ(csv_row_get(&r, 1) == NULL, true);
  csv_row_reset(&r);

  csv_row_init(&r, (char*)"a\0b\0", 4, FALSE);
  ASSERT_EQ(csv_row_count_cols(&r), 2);
  ASSERT_STREQ(csv_row_get(&r, 0), "a");
  ASSERT_STREQ(csv_row_get(&r, 1), "b");
  csv_row_reset(&r);
}

TEST(csv_file, csv_row_to_str) {
  csv_row_t r;
  str_t str;
  str_init(&str, 0);
  memset(&r, 0x00, sizeof(r));

  ASSERT_EQ(csv_row_set_data(&r, "1,2,3", ','), RET_OK);
  ASSERT_EQ(csv_row_to_str(&r, &str, ','), RET_OK);
  ASSERT_STREQ(str.str, "1,2,3\r\n");

  ASSERT_EQ(csv_row_set_data(&r, "1,2,3,,,4", ','), RET_OK);
  ASSERT_EQ(csv_row_count_cols(&r), 6);
  ASSERT_EQ(csv_row_to_str(&r, &str, ','), RET_OK);
  ASSERT_STREQ(str.str, "1,2,3,,,4\r\n");

  ASSERT_EQ(csv_row_set_data(&r, "1,2,3,,", ','), RET_OK);
  ASSERT_EQ(csv_row_count_cols(&r), 5);
  ASSERT_EQ(csv_row_to_str(&r, &str, ','), RET_OK);
  ASSERT_STREQ(str.str, "1,2,3,,\r\n");

  ASSERT_EQ(csv_row_set_data(&r, "1,2,3,,,,", ','), RET_OK);
  ASSERT_EQ(csv_row_to_str(&r, &str, ','), RET_OK);
  ASSERT_STREQ(str.str, "1,2,3,,,,\r\n");

  str_reset(&str);
  csv_row_reset(&r);
}

TEST(csv_file, row_set_data1) {
  csv_row_t r;

  memset(&r, 0x00, sizeof(r));
  ASSERT_EQ(csv_row_set_data(&r, "1,2,3", ','), RET_OK);
  ASSERT_STREQ(csv_row_get(&r, 0), "1");
  ASSERT_STREQ(csv_row_get(&r, 1), "2");
  ASSERT_STREQ(csv_row_get(&r, 2), "3");

  csv_row_reset(&r);
}

TEST(csv_file, row_set_data2) {
  csv_row_t r;

  memset(&r, 0x00, sizeof(r));
  ASSERT_EQ(csv_row_set_data(&r, "\"1\",2,\"3\"", ','), RET_OK);
  ASSERT_STREQ(csv_row_get(&r, 0), "1");
  ASSERT_STREQ(csv_row_get(&r, 1), "2");
  ASSERT_STREQ(csv_row_get(&r, 2), "3");

  csv_row_reset(&r);
}

TEST(csv_file, row_set_data3) {
  csv_row_t r;

  memset(&r, 0x00, sizeof(r));
  ASSERT_EQ(csv_row_set_data(&r, "\"1,1\",2,\"3,3\"", ','), RET_OK);
  ASSERT_STREQ(csv_row_get(&r, 0), "1,1");
  ASSERT_STREQ(csv_row_get(&r, 1), "2");
  ASSERT_STREQ(csv_row_get(&r, 2), "3,3");

  csv_row_reset(&r);
}

TEST(csv_file, row_set_data4) {
  csv_row_t r;

  memset(&r, 0x00, sizeof(r));
  ASSERT_EQ(csv_row_set_data(&r, "\"1,\\\"1\",2,\"3,\\\\3\"", ','), RET_OK);
  ASSERT_STREQ(csv_row_get(&r, 0), "1,\"1");
  ASSERT_STREQ(csv_row_get(&r, 1), "2");
  ASSERT_STREQ(csv_row_get(&r, 2), "3,\\3");

  csv_row_reset(&r);
}

TEST(csv_file, rows) {
  uint32_t i = 0;
  csv_rows_t rows;
  csv_row_t* r = NULL;
  ASSERT_EQ(csv_rows_init(&rows, 10), RET_OK);

  for (i = 0; i < 100; i++) {
    r = csv_rows_append(&rows);
    ASSERT_EQ(r != NULL, true);
    r->size = i;
    ASSERT_EQ(rows.size, i + 1);
    r = csv_rows_get(&rows, i);
    ASSERT_EQ(r->size, i);
  }

  ASSERT_EQ(csv_rows_reset(&rows), RET_OK);
}

TEST(csv_file, rows1) {
  csv_rows_t rows;
  csv_row_t* r = NULL;
  ASSERT_EQ(csv_rows_init(&rows, 10), RET_OK);

  r = csv_rows_insert(&rows, 0);
  r->size = 0;

  r = csv_rows_insert(&rows, 0);
  r->size = 1;

  r = csv_rows_insert(&rows, 0);
  r->size = 2;

  r = csv_rows_get(&rows, 0);
  ASSERT_EQ(r->size, 2);

  r = csv_rows_get(&rows, 1);
  ASSERT_EQ(r->size, 1);

  r = csv_rows_get(&rows, 2);
  ASSERT_EQ(r->size, 0);

  ASSERT_EQ(csv_rows_remove(&rows, 0), RET_OK);
  r = csv_rows_get(&rows, 0);
  ASSERT_EQ(r->size, 1);

  ASSERT_EQ(csv_rows_reset(&rows), RET_OK);
}

TEST(csv_file, basic) {
  const char* str = "11,12,13\n21,22,23";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');
  ASSERT_EQ(csv_file_get_rows(csv), 2);
  ASSERT_EQ(csv_file_get_cols(csv), 3);

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "11");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "12");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "13");

  ASSERT_STREQ(csv_file_get(csv, 1, 0), "21");
  ASSERT_STREQ(csv_file_get(csv, 1, 1), "22");
  ASSERT_STREQ(csv_file_get(csv, 1, 2), "23");

  csv_file_destroy(csv);
}

TEST(csv_file, title) {
  const char* str = "aa,bb,cc\n11,12,13\n21,22,23";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');
  ASSERT_EQ(csv_file_get_rows(csv), 3);
  ASSERT_EQ(csv_file_get_cols(csv), 3);

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "aa");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "bb");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "cc");

  ASSERT_STREQ(csv_file_get(csv, 1, 0), "11");
  ASSERT_STREQ(csv_file_get(csv, 1, 1), "12");
  ASSERT_STREQ(csv_file_get(csv, 1, 2), "13");

  ASSERT_STREQ(csv_file_get(csv, 2, 0), "21");
  ASSERT_STREQ(csv_file_get(csv, 2, 1), "22");
  ASSERT_STREQ(csv_file_get(csv, 2, 2), "23");

  ASSERT_STREQ(csv_file_get_by_name(csv, 1, "aa"), "11");
  ASSERT_STREQ(csv_file_get_by_name(csv, 1, "bb"), "12");
  ASSERT_STREQ(csv_file_get_by_name(csv, 1, "cc"), "13");

  ASSERT_STREQ(csv_file_get_by_name(csv, 2, "aa"), "21");
  ASSERT_STREQ(csv_file_get_by_name(csv, 2, "bb"), "22");
  ASSERT_STREQ(csv_file_get_by_name(csv, 2, "cc"), "23");

  ASSERT_EQ(csv_file_get_col_of_name(csv, "aa"), 0);
  ASSERT_EQ(csv_file_get_col_of_name(csv, "bb"), 1);
  ASSERT_EQ(csv_file_get_col_of_name(csv, "cc"), 2);

  csv_file_destroy(csv);
}

TEST(csv_file, modify) {
  const char* str = "aa,bb,cc\n";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');
  ASSERT_EQ(csv_file_get_rows(csv), 1);
  ASSERT_EQ(csv_file_get_cols(csv), 3);
  ASSERT_EQ(csv->has_title, FALSE);

  ASSERT_EQ(csv_file_append_row(csv, "21,22,23"), RET_OK);
  ASSERT_EQ(csv_file_insert_row(csv, 1, "11,12,13"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 1, 0), "11");
  ASSERT_STREQ(csv_file_get(csv, 2, 0), "21");
  ASSERT_EQ(csv_file_get_rows(csv), 3);
  csv_file_save(csv, "test.csv");
  csv_file_destroy(csv);
}

TEST(csv_file, main_title) {
  const char* str = "awtk\naa,bb,cc\n";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');
  ASSERT_EQ(csv_file_get_rows(csv), 2);
  ASSERT_EQ(csv_file_get_cols(csv), 3);
  ASSERT_EQ(csv->has_title, TRUE);
  ASSERT_STREQ(csv_file_get_title(csv), "awtk");

  csv_file_destroy(csv);
}

TEST(csv_file, set) {
  const char* str = "11,22,33\n";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "11");
  ASSERT_EQ(csv_file_set(csv, 0, 0, "aa"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 0), "aa");

  ASSERT_EQ(csv_file_set(csv, 0, 1, "a"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "a");

  ASSERT_EQ(csv_file_set(csv, 0, 1, "aaaaaa"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "aaaaaa");

  ASSERT_EQ(csv_file_set(csv, 0, 0, "1111"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 0), "1111");

  ASSERT_EQ(csv_file_set(csv, 0, 1, "2222222"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "2222222");

  ASSERT_EQ(csv_file_set(csv, 0, 1, "3333333333"), RET_OK);
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "3333333333");

  csv_file_destroy(csv);
}

TEST(csv_file, reload) {
  csv_file_t* csv = csv_file_create("tests/test.csv", ',');
  ASSERT_EQ(csv_file_get_rows(csv), 3);
  ASSERT_EQ(csv_file_get_cols(csv), 3);
  ASSERT_EQ(csv->has_title, FALSE);

  ASSERT_EQ(csv_file_append_row(csv, "31,32,33"), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 4);
  
  ASSERT_EQ(csv_file_reload(csv), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 3);

  csv_file_destroy(csv);
}

TEST(csv_file, checked) {
  const char* str = "aa,bb,cc\n11,12,13\n21,22,23\n31,32,33\n41,42,43\n";
  csv_file_t* csv = csv_file_create_with_buff(str, strlen(str), ',');
  ASSERT_EQ(csv_file_get_rows(csv), 5);

  ASSERT_EQ(csv_file_get_checked_rows(csv), 0);
  ASSERT_EQ(csv_file_set_row_checked(csv, 0, TRUE), RET_OK);
  ASSERT_EQ(csv_file_is_row_checked(csv, 0), TRUE);
  ASSERT_EQ(csv_file_get_checked_rows(csv), 1);
  
  ASSERT_EQ(csv_file_set_row_checked(csv, 2, TRUE), RET_OK);
  ASSERT_EQ(csv_file_is_row_checked(csv, 2), TRUE);
  ASSERT_EQ(csv_file_get_checked_rows(csv), 2);
  
  ASSERT_EQ(csv_file_set_row_checked(csv, 4, TRUE), RET_OK);
  ASSERT_EQ(csv_file_is_row_checked(csv, 4), TRUE);
  ASSERT_EQ(csv_file_get_checked_rows(csv), 3);

  ASSERT_EQ(csv_file_remove_checked_rows(csv), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 2);
  
  ASSERT_STREQ(csv_file_get(csv, 0, 0), "11");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "12");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "13");
  
  ASSERT_STREQ(csv_file_get(csv, 1, 0), "31");
  ASSERT_STREQ(csv_file_get(csv, 1, 1), "32");
  ASSERT_STREQ(csv_file_get(csv, 1, 2), "33");
  
  ASSERT_EQ(csv_file_set_row_checked(csv, 0, TRUE), RET_OK);
  ASSERT_EQ(csv_file_is_row_checked(csv, 0), TRUE);
  
  ASSERT_EQ(csv_file_set_row_checked(csv, 1, TRUE), RET_OK);
  ASSERT_EQ(csv_file_is_row_checked(csv, 1), TRUE);
  
  ASSERT_EQ(csv_file_remove_checked_rows(csv), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 0);

  csv_file_destroy(csv);
}

static ret_t filter_keep_1(void* ctx, csv_file_t* csv, uint32_t index, csv_row_t* row) {
  uint32_t i = tk_pointer_to_int(ctx);

  if(index < i) {
    return RET_FAIL;
  } else {
    return RET_STOP;
  }
}

TEST(csv_file, filter_1) {
  const char* str = "11,12,13\n21,22,23\n31,32,33";
  csv_file_t* csv = csv_file_create_empty(',', filter_keep_1, tk_pointer_from_int(0));

  ASSERT_EQ(csv_file_load_buff(csv, str, strlen(str)), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 1);
  ASSERT_EQ(csv_file_get_cols(csv), 3);

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "11");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "12");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "13");

  csv_file_set_filter(csv, filter_keep_1, tk_pointer_from_int(1));
  ASSERT_EQ(csv_file_load_buff(csv, str, strlen(str)), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 1);
  ASSERT_EQ(csv_file_get_cols(csv), 3);

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "21");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "22");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "23");
  
  csv_file_set_filter(csv, filter_keep_1, tk_pointer_from_int(2));
  ASSERT_EQ(csv_file_load_buff(csv, str, strlen(str)), RET_OK);
  ASSERT_EQ(csv_file_get_rows(csv), 1);
  ASSERT_EQ(csv_file_get_cols(csv), 3);

  ASSERT_STREQ(csv_file_get(csv, 0, 0), "31");
  ASSERT_STREQ(csv_file_get(csv, 0, 1), "32");
  ASSERT_STREQ(csv_file_get(csv, 0, 2), "33");

  csv_file_destroy(csv);
}
