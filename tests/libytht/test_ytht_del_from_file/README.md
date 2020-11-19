test_ytht_del_from_file
=======================

这是一个用于测试 `libytht::fileop::ytht_del_from_file()` 的程序。使用方法：

```bash
echo "abcde" > test.txt
/path/to/test_ytht_del_from_file test.txt "bc" false
cat test.txt # <-- "ade"

echo "abcde" > test.txt
echo "12345" >> test.txt
/path/to/test_ytht_del_from_file test.txt "abcde" true
cat test.txt # <-- "12345"
```

