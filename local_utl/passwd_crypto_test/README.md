# Password Crypto Algorithm Test

在这个测试中，试验是否可以将 `crypt.c` 从 `libytht` 中移除，使用 `crypt(3)` （或其变种）验证密码，从而

- 降低维护难度
- 在未来升级加密算法

## 使用方法

```bash
# 生成 `num` 条记录并保存在文件 `passwd` 中
./generator -n num -out /path/to/passwd

# 使用 `crypt1_p` 验证密码
./validator_in_tree -in /path/to/passwd

# 使用 `crypt(3)` 验证密码
./validator_libcrypt -in /path/to/passwd
```

## 测试结果

设定 `num` 分别为 `100` 以及 `1000000` 生成测试数据，均验证通过。
