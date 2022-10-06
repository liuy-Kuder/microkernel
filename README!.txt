开发记录
2022.9.5
分析xboot源码并移植到STM32

2022.9.10 ~ 2022.9.12：
完成KOBJ device driver的移植工作

2022.9.17
开始filesystem移植

2022.9.18
1、filesystem移植完成
2、实现filesystem对驱动的读写测试；如下
//打开
  fdreg = vfs_open("/device/adc/adc_test/vreference", O_RDONLY, 0);
  if(fdreg < 0)
    LL_mDelay(1000);
//读取
  ret = vfs_read(fdreg,buf,5);
  if(ret < 0)
    LL_mDelay(1000);
//关闭
  ret = vfs_close(fdreg);
  if(ret < 0)
    LL_mDelay(1000);

2022.9.19
1、完善mkernel
2022.9.22
1、开始driver架构搭建

driver 注册总线，设备调用总线

2022.9.23
需求：
1、建立red和green两个LED灯驱动
2、建立gpio总线,是一个还是两个

2022.9.26
添加设备或者驱动的LOG日志功能

添加github功能
