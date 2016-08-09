# KafkaClient
Kafka producer and consumer clients developped based on librdkafka 0.8 and some articles

本文基于librdkafka_0.8.

librdkafka主要分为config，topic，produce，consume几个模块

# config
依据kafka相应的config文件字段定义
先通过默认配置文件创建confg对象，再通过confg的set方法设置需要覆盖的字段
参考： [http://blog.csdn.net/beitiandijun/article/details/40582541](http://blog.csdn.net/beitiandijun/article/details/40582541)

#topic
支持一个producer创建多个topic（比如bid，wn等类型的日志）
创建主题方法rd_kafka_topic_new逻辑：
检查是否有同名可用topic ==> 检查tconf为空则以默认配置创建 ==>
检查partitioner未配置则配置为随机方法（从0，pcnt-1中选取partition） ==> 创建分区号为默认值```RD_KAFKA_PARTITION_UA``` 的分区，并将topic插入队尾
 
partitioner类回调方法需自己实现

#produce

kafka对象创建方法

```
rd_kafka_t *rd_kafka_new (rd_kafka_type_t type, rd_kafka_conf_t *conf,
			  char *errstr, size_t errstr_size)
```
检查config对象，创建事件循环主线程```rd_kafka_thread_main```，通过```rd_kafka_brokers_add```创建io事件循环子线程```rd_kafka_broker_thread_main```，分发处理socket connect，send，recv事件


消息生产方法
```
int rd_kafka_produce (rd_kafka_topic_t *rkt, int32_t partition,
		      int msgflags,
		      void *payload, size_t len,
		      const void *key, size_t keylen,
		      void *msg_opaque) 
```
 参数key用于partitioner hash生成partition 值
 参数partition默认为```RD_KAFKA_PARTITION_UA```，即未分配状态，在创建消息方法```rd_kafka_msg_new```检查发现partition为默认值，则调用partitioner回调函数生成partition值

#consume
创建config，topic及kafka对象流程相同



#问题研究

# 1.验证kafka超出数据保存设置时间后数据的有效性
通过以下设置，可开启kafka数据超时删除功能：
### kafka server配置文件server.properties中设置
##### 开启超时删除（false值为删除）
log.cleaner.enable
##### 设置删除检测间隔
log.retention.check.interval.ms
##### 设置超时时间
log.retention.hours 或
log.retention.minutes

### producer中设置topic config属性
cleanup.policy=delete和
retention.ms

验证结果：超时数据被删除后不可再访问


# 2.kafka数据超时删除后，对新生产的数据访问方式
consumer offset输入参数设置为-2，即Topic::OFFSET_BEGINNING，读取有效数据起始位置

# 3.kafka删除超时数据并生产新数据后，消费者消费完新数据重启后获取新offset方式
consume recv时返回的offset，即为已消费的最后一个数据，resume重启后获取新的起始offset为：
offset_new=offset+1

# 4.一个connection是否可以配置多个topic？

支持，一个kafka实例可以创建多个topic

# 5.一个topic是否可以配置多个partition？

kafka支持一个topic配置多个partition，但是限于需要保证数据的顺序性，只能配置1个partition

# 6.发送数据时是否需要指定topic和partition以及如何指定？

首先，必须指定topic；其次，对于partition,有两种方式：
a. 明确指定，则数据被发送到指定partition
b. 设置为RD_KAFKA_PARTITION_UA，则kafka会回调partitioner进行均衡选取，partitioner方法需要自己实现。可以轮询或者传入key进行hash

# 7.接收数据时是否需要指定topic和partition以及如何指定？

都必须指定，且partition必须为topic对应的partitions之一。若未指定partition（RD_KAFKA_PARTITION_UA），消费调用将失败。超出partitions范围，则该partition将被设置为desired

# 8.Kafka是否可以保证消息顺序？

kafka仅支持单个partition上的顺序性，要保证整个topic在被消费时的顺序性，一个topic只能有一个partition，这也意味着每个group只有一个consumer。因为一个partition只能被同一group中
的一个consumer消费。

Kafka only provides a total order over messages within a partition, not between different partitions in a topic. Per-partition ordering combined with the ability to partition data by key is sufficient for most applications. However, if you require a total order over messages this can be achieved with a topic that has only one partition, though this will mean only one consumer process per consumer group.

[https://kafka.apache.org/documentation.html](https://kafka.apache.org/documentation.html)

# 9.Kafka的多个partition之间负载均衡由谁实现？

由producer负责实现。参照第6条
