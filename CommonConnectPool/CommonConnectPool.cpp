#include"CommonConnectionPool.h"
#include"public.h"
#include<functional>
CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigureFile())
	{
		LOG("falied to create CommonConnectionPool");
		return;
	}
	for (int i = 0; i < initSize_; i++)
	{
		Connection* p = new Connection();
		p->connection(ip_, port_, username_, password_, dbname_);
		connectionCount_++;
		connectionQueue_.emplace(p);
	}
	//启动一个新的线程作为生产者线程生产连接
	std::thread producer{ std::bind(&CommonConnectionPool::produceConnection,this) };
	producer.detach();
	//启动一个新的定时线程，扫描多余的空闲连接，进行对多余的连接进行回收
	std::thread scanner{ std::bind(&CommonConnectionPool::scannerConnectionTask,this) };
	scanner.detach();

}
//获取连接池对象
CommonConnectionPool*CommonConnectionPool::getCommonConnectionPool()
{
	static CommonConnectionPool instance;
	return &instance;
}
//加载配置文件中的数据项
bool CommonConnectionPool::loadConfigureFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini is not exist");
		return false;
	}
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		std::string str = line;
		std::size_t index = str.find('=', 0);
		if (index == std::string::npos)
			continue;
		std::size_t endindex = str.find('\n', index);
		std::string key = str.substr(0, index);
		std::string value = str.substr(index + 1, endindex - index - 1);
		if (key == "ip")
			ip_ = value;
		else if (key == "port")
			port_ = atoi(value.c_str());
		else if (key == "username")
			username_ = value;
		else if (key == "dbname")
			dbname_ = value;
		else if (key == "password")
			password_ = value;
		else if (key == "initSize")
			initSize_ = atoi(value.c_str());
		else if (key == "maxSize")
			maxSize_ = atoi(value.c_str());
		else if (key == "maxFreeTime")
			maxFreeTime_ = atoi(value.c_str());
		else if (key == "maxTimeOut")
			maxTimeOut_ = atoi(value.c_str());
     }
	fclose(pf);
	return true;
}
void CommonConnectionPool::produceConnection()
{
	while (true)
	{
		std::unique_lock<std::mutex>lock{ queueMutex_ };
		while (!connectionQueue_.empty())
			cv_.wait(lock);
		if (connectionCount_ == maxSize_)
			return;
		//生产新的连接
		Connection* p = new Connection();
		p->connection(ip_, port_, username_, password_, dbname_);
		connectionCount_++;
		cv_.notify_all();
	}

}
std::shared_ptr<Connection>CommonConnectionPool::getConnection()
{
	std::unique_lock<std::mutex>lock{ queueMutex_ };
	while (connectionQueue_.empty())
	{
		//等待超时时长的的时间，若还是没有获得连接，那么直接返回失败
		if (std::cv_status::timeout == cv_.wait_for(lock, std::chrono::microseconds{ maxTimeOut_ }))
		{
			if (connectionQueue_.empty())
			{
				LOG("can not get connection");
				return nullptr;
			}
		}

	}
	//消费连接 
	std::shared_ptr<Connection>foods{ connectionQueue_.front(),[&](Connection* pcon) {
		std::unique_lock<std::mutex>lock{queueMutex_};
		connectionQueue_.push(pcon);
     }
	};
	connectionQueue_.pop();
	connectionCount_--;
	cv_.notify_all();
	return foods;
}
void CommonConnectionPool::scannerConnectionTask()
{
	while (true)
	{
		//通过sleep模拟定时效果
		std::this_thread::sleep_for(std::chrono::seconds(maxFreeTime_));
		//扫描整个队列，释放多余的连接
		std::unique_lock<std::mutex>lock{ queueMutex_ };
		while (connectionCount_ > initSize_)
		{
			Connection* p = connectionQueue_.front();
			if (p->getAliveTime() >= (maxFreeTime_ * 1000))
			{
				connectionQueue_.pop();
				connectionCount_--;
				delete p; //调用~Connection释放连接
			}
			else
				break;  //队头的连接都没超过maxFreeTime,其他连接肯定没超过
		}
	}

}