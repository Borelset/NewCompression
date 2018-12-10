编译
```
cmake .
make
```

运行
```
//压缩  
./NC data.dat 0 1e-5  
//最后生成的时data.dat.nc  
   
//暂时gzip还没放到代码里，所以还需要gzip再压缩一下   
gzip -1 data.dat.nc   
//此时能看到最终生成的文件大小    

//解压
./NC data.dat.nc 1
//生成data.dat.nc.dc

```
