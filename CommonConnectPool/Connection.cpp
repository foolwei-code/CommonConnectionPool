#include"Connection.h"
#include"public.h"
//初始化数据库连接
Connection::Connection()
{
	conn_ = mysql_init(nullptr);
}
//释放数据库连接资源
Connection::~Connection()
{
	if (conn_!=nullptr)
		mysql_close(conn_);
}
//与数据库建立一条连接
bool Connection::connection(std::string ip, unsigned short port, std::string username, std::string password, std::string dbname)
{
	MYSQL* p = mysql_real_connect(conn_, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port,nullptr,0);
	return p != nullptr;
}
//修改数据库DML
bool Connection::update(std::string sql)
{
	if (mysql_query(conn_, sql.c_str()))
	{
		LOG("更新失败"+sql);
		return false;
	}
	return true;
}
MYSQL_RES* Connection::query(std::string sql)
{
	if (mysql_query(conn_, sql.c_str()))
	{
		LOG("查询失败" + sql);
		return nullptr;
	}
	return mysql_use_result(conn_);
}
//刷新一下连接的起始的空闲时间点
void Connection::refreshAliveTime() { aliveTime_ = clock(); }
//返回存活的时间
std::clock_t Connection::getAliveTime()const { return clock() - aliveTime_; }
