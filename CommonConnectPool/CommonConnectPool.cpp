#include"CommonConnectionPool.h"
#include"public.h"
#include<string>
//线程安全的懒汉单例模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;//lock和unlock
	return &pool;
}
//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("my.ini", ":r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		std::string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)  //无效的配置项
		{
			continue;
		}
		int endidx = str.find('\n', idx);
		std::string key = str.substr(0, idx);
	}
}