#include<iostream>
#include"CommonConnectionPool.h"
#include"Connection.h"
#include<string>
int main()
{
	/*
	Connection conn;
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
		"xiaoli", 19, "female");
	conn.connect("127.0.0.1", 3306, "root", "123456ttyl", "test");
	conn.update(sql);
	*/
	ConnectionPool* cp = ConnectionPool::getConnectionPool();
}