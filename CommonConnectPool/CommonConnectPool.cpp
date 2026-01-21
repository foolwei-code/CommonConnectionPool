#include"CommonConnectionPool.h"
#include"public.h"
#include<string>
//线程安全的懒汉单例模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;//lock和unlock
	return &pool;
}
//连接池的构造
ConnectionPool::ConnectionPool(){
	if (!loadConfigFile())
		return;
	//创建初始数量的连接
	for (int i = 0; i < initSize_; i++)
	{
		Connection* p = new Connection();
		p->connect(ip_, port_, username_, password_, dbname_);
		p->refreshAliveTime(); //刷新一下开始空闲的起始时间
		connection_queue_.push(p);
		connection_Count_++;
	}
	//启动一个新的线程作为新的生产者
	std::thread producer{ std::bind(&ConnectionPool::produceConnectionTask,this)};
	producer.detach();
	//启动一个新的定时线程，扫描多余的空闲连接，进行对多余的连接进行回收
	std::thread scanner{ std::bind(&ConnectionPool::scannerConnectionTask,this) };
	scanner.detach();
}
//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
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
		std::string value = str.substr(idx + 1, endidx - idx - 1);
		if (key == "ip")
			ip_ = value;
		else if (key == "port")
			port_ = atoi(value.c_str());
		else if (key == "username")
			username_ = value;
		else if (key == "password")
			password_ = value;
		else if (key == "initSize")
			initSize_ = atoi(value.c_str());
		else if (key == "maxSize")
			maxSize_ = atoi(value.c_str());
		else if (key == "maxFreeTime")
			maxFreeTime_ = atoi(value.c_str());
		else if (key == "connectionTimeOut")
			connectionTimeOut_ = atoi(value.c_str());
		else if (key == "dbname")
			dbname_ = value;
	}
	return true;
}
//运行在独立的线程中，专门负责生产新的连接
void ConnectionPool::produceConnectionTask()
{
	while (true)
	{
		std::unique_lock<std::mutex>lock{ queue_mutex_ };
		while (!connection_queue_.empty())
		{
			cv.wait(lock);  //队列不空，此处生产者线程进入等待状态
		}
		//连接数量没有到达上限，继续创建新的连接
		if (connection_Count_ < maxSize_)
		{
			Connection* p = new Connection();
			p->connect(ip_, port_, username_, password_, dbname_);
			p->refreshAliveTime(); //刷新一下开始空闲的起始时间
			connection_queue_.push(p);
			connection_Count_++;
		}
		//通知消费者线程，可以进行消费了
		cv.notify_all();
	}
}

//给外部提供接口，从连接池中获得一个空闲连接
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
	std::unique_lock<std::mutex>lock{ queue_mutex_ };
	while(connection_queue_.empty())
	{
		//不要写sleep
		if (std::cv_status::timeout == cv.wait_for(lock, std::chrono::milliseconds(connectionTimeOut_)))
		{
			if (connection_queue_.empty())
			{
				LOG("获取空闲时间超时了...获取连接失败");
				return nullptr;
			}
		}
	}
	/*
	 shared_ptr智能指针析构时，会把connection资源直接delete掉，
	 相当于调用connection的析构函数，connection就被close掉了，
	 这里需要自定义shared_ptr的释放资源方式，把connection直接归还到queue中
	*/
	std::shared_ptr<Connection>sp{ connection_queue_.front(),[&](Connection* pcon)->void
		{
			//这里是在服务器应用，所以一定要考虑到队列的线程安全操作
			std::unique_lock<std::mutex>lock{queue_mutex_};
			pcon->refreshAliveTime(); //刷新一下开始空闲的起始时间
			connection_queue_.push(pcon);
        }
	};
	connection_queue_.pop();
	cv.notify_all(); //消费了队列中的最后一个connection,通知一下生产者生产连接
	return sp;
}
//扫描多余的空闲连接，进行对多余的连接进行回收
void ConnectionPool::scannerConnectionTask()
{
	while (true)
	{
		//通过sleep模拟定时效果
		std::this_thread::sleep_for(std::chrono::seconds(maxFreeTime_));
		//扫描整个队列，释放多余的连接
		std::unique_lock<std::mutex>lock{ queue_mutex_ };
		while (connection_Count_ > initSize_)
		{
			Connection* p = connection_queue_.front();
			if (p->getAliveTime() >= (maxFreeTime_ * 1000))
			{
				connection_queue_.pop();
				connection_Count_--;
				delete p; //调用~Connection释放连接
			}
			else
				break;  //队头的连接都没超过maxFreeTime,其他连接肯定没超过
        }
	}
}
