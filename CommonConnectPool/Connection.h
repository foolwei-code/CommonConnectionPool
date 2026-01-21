#pragma once
#include<mysql.h>
#include<string>
#include<ctime>
//实现mysql数据库的操作
class Connection
{
public:
	//初始化数据库
	Connection();
	//释放数据库连接资源
	~Connection();
	//连接数据库
	bool connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname);
	//更新操作
	bool update(std::string sql);
	//查询操作
	MYSQL_RES* query(std::string sql);
	//刷新一下连接的起始的空闲时间点
	void refreshAliveTime() { aliveTime_ = clock(); }
	//返回存活的时间
	std::clock_t getAliveTime()const { return clock() - aliveTime_; }
private:
	MYSQL* _conn;//表示和MySQL建立一条连接
	std::clock_t aliveTime_;  //记录进入空闲状态后的起始存活时间
};
