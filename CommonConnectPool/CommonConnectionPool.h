#pragma once
//实现数据库连接池
#include<memory>
#include"Connection.h"
#include<queue>
#include<thread>
#include<mutex>
#include<atomic>
class CommonConnectionPool
{
public:
	//获取连接池对象
	static CommonConnectionPool* getCommonConnectionPool();
	//给外部提供一个接口，从连接池中获取一条空闲数据库连接
	std::shared_ptr<Connection>getConnection();
private:
	//懒汉式单例模式实现数据库连接池
	//1.构造函数私有化
	CommonConnectionPool();
	//2.拷贝函数删除
	CommonConnectionPool(const CommonConnectionPool&) = delete;
	CommonConnectionPool& operator=(const CommonConnectionPool&) = delete;
	std::string ip_; //mysql的ip地址
	unsigned short port_;  //mysql的端口号
	std::string dbname_;  //连接的数据库的名称
	std::string password_; //用户密码
	std::string username_;  //用户名
	std::size_t initSize_;   //初始连接量
	std::size_t maxSize_;   //最大连接量
	std::size_t maxFreeTime_; //连接池最大空余时间
	std::size_t maxTimeOut_;  //连接池的超时时长
	std::queue<Connection*>connectionQueue_;  //存放数据库连接的线程安全的队列
	std::mutex queueMutex_; //维护队列线程安全的互斥锁
	std::condition_variable cv_;  //设置条件变量，用来实现生产线程和消费线程的通信
	std::atomic_int connectionCount_;  //队列当中连接的数量
	bool loadConfigureFile();  //加载配置文件中的数据项
	void produceConnection(); //运行在独立的线程中，生产新的连接
	//扫描多余的空闲连接，进行对多余的连接进行回收
	void scannerConnectionTask();
};
