#pragma once
#include<mysql.h>
#include<string>
#include<ctime>
class Connection
{
public:
	//初始化数据库连接
	Connection();
	//释放数据库连接
	~Connection();
	//与数据库建立一条连接
	bool connection(std::string ip, unsigned short port, std::string username, std::string password, std::string dbname);
	//更新操作
	bool update(std::string sql);
	//查询操作
	MYSQL_RES* query(std::string sql);
	//刷新一下连接的起始的空闲时间点
	void refreshAliveTime();
	//返回存活的时间
	std::clock_t getAliveTime()const;

private:
	MYSQL* conn_;  //表示与mysqlserver的一天连接
	std::clock_t aliveTime_;  //记录进入空闲状态后的起始存活时间
};