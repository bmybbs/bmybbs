test_ytht_del_from_file
=======================

这是一个用于测试 `libytht::fileop::ytht_del_from_file()` 的程序。使用方法：

```bash
echo "abcde" > test.txt
/path/to/test_ytht_del_from_file test.txt "bc"
cat test.txt # <-- "ade"
```

