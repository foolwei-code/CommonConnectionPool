#include<iostream>
#include"Connection.h"
#include"CommonConnectionPool.h"
int main()
{
	//创建连接对象
	std::clock_t begin = clock();
	/*for (int i = 0; i < 1000; i++)
	{
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,sex) values('%s','%s')",
			"xiaoli", "female");
		conn.connection("127.0.0.1", 3306, "root", "123456ttyl", "test");
		conn.update(sql);
    }*/
	CommonConnectionPool* cp = CommonConnectionPool::getCommonConnectionPool();
	for (int i = 0; i < 1000; i++)
	{
		std::shared_ptr<Connection>sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,sex) values('%s','%s')",
			"xiaoli", "female");
	}
	std::clock_t end = clock();
	std::cout << end-begin <<"ms" << std::endl;
}