#include "vlabel/vlabel.h"
#include "gtest/gtest.h"

TEST(vlabel, basic) {
  widget_t* w = vlabel_create(NULL, 10, 20, 30, 40);
  vlabel_set_right_to_left(w, TRUE);
  ASSERT_EQ(VLABEL(w)->right_to_left, TRUE);

  vlabel_set_right_to_left(w, FALSE);
  ASSERT_EQ(VLABEL(w)->right_to_left, FALSE);

  ASSERT_EQ(widget_set_prop_bool(w, VLABEL_PROP_RIGHT_TO_LEFT, TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, VLABEL_PROP_RIGHT_TO_LEFT, FALSE), TRUE);

  widget_destroy(w);
}

TEST(vlabel, line_parser1) {
  vlabel_line_parser_t p;
  const wchar_t* text = L"hello";
  ASSERT_EQ(vlabel_line_parser_init(&p, text, wcslen(text), 20, 100), RET_OK);
  ASSERT_EQ(p.total_lines, 1);
  ASSERT_EQ(p.total_paragraphs, 1);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 5);
  ASSERT_EQ(p.is_force_break, FALSE);
  ASSERT_EQ(wcscmp(p.line, L"hello"), 0);

  ASSERT_NE(vlabel_line_parser_next(&p), RET_OK);
}

TEST(vlabel, line_parser2) {
  vlabel_line_parser_t p;
  const wchar_t* text = L"hello\rworld";
  ASSERT_EQ(vlabel_line_parser_init(&p, text, wcslen(text), 20, 100), RET_OK);
  ASSERT_EQ(p.total_lines, 2);
  ASSERT_EQ(p.total_paragraphs, 2);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 6);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"hello\r", p.line_size), 0);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 5);
  ASSERT_EQ(p.is_force_break, FALSE);
  ASSERT_EQ(wcsncmp(p.line, L"world", p.line_size), 0);

  ASSERT_NE(vlabel_line_parser_next(&p), RET_OK);
}

TEST(vlabel, line_parser3) {
  vlabel_line_parser_t p;
  const wchar_t* text = L"hello\rworld\n";
  ASSERT_EQ(vlabel_line_parser_init(&p, text, wcslen(text), 20, 100), RET_OK);
  ASSERT_EQ(p.total_lines, 2);
  ASSERT_EQ(p.total_paragraphs, 2);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 6);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"hello\r", p.line_size), 0);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 6);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"world\n", p.line_size), 0);

  ASSERT_NE(vlabel_line_parser_next(&p), RET_OK);
}

TEST(vlabel, line_parser4) {
  vlabel_line_parser_t p;
  const wchar_t* text = L"hello\r\nworld\r";
  ASSERT_EQ(vlabel_line_parser_init(&p, text, wcslen(text), 20, 100), RET_OK);
  ASSERT_EQ(p.total_lines, 2);
  ASSERT_EQ(p.total_paragraphs, 2);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 7);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"hello\r\n", p.line_size), 0);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 6);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"world\r", p.line_size), 0);

  ASSERT_NE(vlabel_line_parser_next(&p), RET_OK);
}

TEST(vlabel, line_parser5) {
  vlabel_line_parser_t p;
  const wchar_t* text = L"helloworld\r";
  ASSERT_EQ(vlabel_line_parser_init(&p, text, wcslen(text), 20, 100), RET_OK);
  ASSERT_EQ(p.total_lines, 2);
  ASSERT_EQ(p.total_paragraphs, 1);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 5);
  ASSERT_EQ(p.is_force_break, FALSE);
  ASSERT_EQ(wcsncmp(p.line, L"hello", p.line_size), 0);

  ASSERT_EQ(vlabel_line_parser_next(&p), RET_OK);
  ASSERT_EQ(p.line_size, 6);
  ASSERT_EQ(p.is_force_break, TRUE);
  ASSERT_EQ(wcsncmp(p.line, L"world\r", p.line_size), 0);

  ASSERT_NE(vlabel_line_parser_next(&p), RET_OK);
}
