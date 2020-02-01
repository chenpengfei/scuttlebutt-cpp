# scuttlebutt-cpp
Scuttlebutt C++

## Usage
### unit test
```
export ENABLE_COVERAGE=OFF && export GTEST_LATEST=OFF && ./make_all.sh
```


## TODO
1. read/sink 中传递的值可以是泛型 T，也可以自己参考 boost 实现一个 Any 类型
2. 实现一个好用的日志函数
3. C++ 编译器支持尾递归优化
4. 实现一个 event loop

