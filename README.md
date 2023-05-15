# My-SkipList
An implementation of skiplist

# 提供的接口
insert_element（插入数据）  

delete_element（删除数据）  

search_element（查询数据）  

save_file（数据落盘）  

load_file（加载数据）  

display_list（展示已存数据）  

size（返回数据规模）  

# 项目运行方式

```bash
make
./main
```

如果想在自己的程序中使用这个kv存储引擎，只需要在你的CPP文件中include skiplist.h 即可。  

可以运行如下脚本测试kv存储引擎的性能（当然你可以根据自己的需求进行修改）

```bash
cd stress_test
sh stress_test.sh
```