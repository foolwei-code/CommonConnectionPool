#pragma once
#include<string>
#include<queue>
#include"Connection.h"
#include<mutex>
#include<atomic>
#include<thread>
#include<memory>
#include<functional>
#include<condition_variable>
//实现数据库连接池
class ConnectionPool
{
public:
	//获取连接池对象的实例
	static ConnectionPool* getConnectionPool();
	//给外部提供接口，从连接池中获得一个空闲连接
	  std::shared_ptr<Connection>getConnection();
private:
	ConnectionPool();//单例1：构造函数的私有化
	//运行在独立的线程中，负责生产新连接
	void produceConnectionTask();
	//扫描多余的空闲连接，进行对多余的连接进行回收
	void scannerConnectionTask();
	bool  loadConfigFile();//从配置文件中加载配置项
	std::string ip_; //mysql的ip地址
	unsigned short port_; //mysql的端口号3306
	std::string username_;//mysql的登录用户名
	std::string password_;//mysql的登陆密码
	std::string dbname_;  //连接的数据库的名称
	int initSize_;  //连接池的初始连接量
	int maxSize_;   //连接池的最大连接量
	int maxFreeTime_; //连接池的最大空闲时间
	int connectionTimeOut_;//连接的超时时长
	std::queue<Connection*>connection_queue_;//存储mysql的连接的队列
	std::mutex queue_mutex_;  //维护连接队列的线程安全的互斥锁
	std::atomic_int connection_Count_;  //记录连接所创建的connection连接的总数量
	std::condition_variable cv; //设置条件变量，用于连接生产线程和消费线程的通信
};
