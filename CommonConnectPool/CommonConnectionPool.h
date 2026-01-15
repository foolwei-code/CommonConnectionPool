#pragma once
#include<string>
#include<queue>
#include"Connection.h"
#include<mutex>
//实现数据库连接池
class ConnectionPool
{
public:
	//获取连接池对象的实例
	static ConnectionPool* getConnectionPool();
private:
	ConnectionPool();//单例1：构造函数的私有化
	bool  loadConfigFile();//从配置文件中加载配置项
	std::string ip_; //mysql的ip地址
	unsigned short port_; //mysql的端口号3306
	std::string username_;//mysql的登录用户名
	std::string password_;//mysql的登陆密码
	int initSize_;  //连接池的初始连接量
	int maxSize_;   //连接池的最大连接量
	int maxFreeTime_; //连接池的最大空闲时间
	int connectionTimeOut_;//连接的超时时长
	std::queue<Connection*>connection_queue_;//存储mysql的连接的队列
	std::mutex queue_mutex_;  //维护连接队列的线程安全的互斥锁
};
