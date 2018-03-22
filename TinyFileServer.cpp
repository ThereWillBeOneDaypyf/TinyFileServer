// TinyHttp.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include<thread>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<memory>
#include<thread>
#include<algorithm>
#include<boost/thread.hpp>
#include<functional>
#include"TinyFileServer.h"
//#include"tinyclient.h"
//#define _DEBUG
//#define _DEBUG_POST
struct delete_sock
{
	void operator()(boost::asio::ip::tcp::socket* sock)
	{
		sock->close();
	}
};

MyHttp::MyHttp() {}

MyHttp::~MyHttp(){}



void MyHttp::start_up(unsigned short port)
{
	TCP::acceptor acpt(ios, TCP::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port));
	std::cout << "start listen" << std::endl;
	for (;;)
	{
		auto client_ptr = boost::shared_ptr<TCP::socket>(new TCP::socket(ios),delete_sock());
		
		acpt.accept(*client_ptr);
		
		boost::thread t(&MyHttp::accept_request,this,client_ptr);
		t.detach();
	}
}

void MyHttp::accept_request(boost::shared_ptr<TCP::socket> client_ptr)
{
	try {
		for (;;)
		{
			char data[1024];
			boost::system::error_code error;
			size_t length = client_ptr->read_some(boost::asio::buffer(data), error);
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.
			std::for_each(data, data + length, [](char op) { std::cout << op << std::endl; });
			std::string wait_sol_str(data, data + length);
			std::cout << wait_sol_str << std::endl;
			auto pos = std::search(wait_sol_str.begin(), wait_sol_str.end(), GET_METHOD.begin(), GET_METHOD.end());
#ifdef _DEBUG_POST
			std::cout << std::boolalpha << (pos != wait_sol_str.end()) << std::endl;
#endif
			if (pos != wait_sol_str.end()) //get
			{
				std::string url(std::find_if(wait_sol_str.begin(), wait_sol_str.end(), [](char op) { return op == ' '; })+1,wait_sol_str.end());
				for(;;)
				{
					std::unique_lock<std::mutex> lck(map_mutex);
					if (is_vis[url] == 0)
					{
						is_vis[url] = 1;
						break;
					}
				}
				std::ifstream ifs(url,std::ios::in);
#ifdef _DEBUG
				std::cout << std::boolalpha << (ifs.is_open()) << std::endl;
				client_ptr->write_some(boost::asio::buffer(data, length));
#endif
				if (!ifs.is_open())
				{
					not_found(client_ptr);
					continue;
				}
#ifdef _DEBUG
				std::cout << "start headers" << std::endl;
#endif
				headers(client_ptr);
#ifdef _DEBUG
				std::cout << "start cat" << std::endl;
#endif
				cat(client_ptr, ifs);
				end_send(client_ptr);
				{
					std::unique_lock<std::mutex> lck(map_mutex);
					is_vis[url] = 0;
				}
				continue;
			}
			pos = std::search(wait_sol_str.begin(), wait_sol_str.end(), POST_METHOD.begin(), POST_METHOD.end());
			if (pos != wait_sol_str.end()) // post
			{
				auto first_blank = std::find_if(wait_sol_str.begin(), wait_sol_str.end(), [](char op) { return op == ' '; });
				auto second_blank = std::find_if(first_blank + 1, wait_sol_str.end(), [](char op) { return op == ' '; });
				std::string url(first_blank + 1, second_blank);
				std::string content(second_blank + 1, wait_sol_str.end());
#ifdef _DEBUG_POST
				std::cout << url << std::endl << content << std::endl;
#endif
				for (;;)
				{
					std::unique_lock<std::mutex> lck(map_mutex);
					if (is_vis[url] == 0)
					{
						is_vis[url] = 1;
						break;
					}
				}
				std::ofstream ofs(url);
				ofs << content;
				ofs.close();
				headers(client_ptr);
				end_send(client_ptr);
				{
					std::unique_lock<std::mutex> lck(map_mutex);
					is_vis[url] = 0;
				}
				continue;
			}
			cannot_execute(client_ptr);
		}
	}
	catch (boost::system::system_error&e)
	{
		std::cout << e.what() << std::endl;
	}
}

void MyHttp::cat(boost::shared_ptr<TCP::socket> client_ptr, std::ifstream& ifs)
{
	std::string msg;
	while (std::getline(ifs, msg))
	{
#ifdef _DEBUG
		std::cout << msg << std::endl;
#endif
		client_ptr->write_some(boost::asio::buffer(msg.c_str(), msg.size()));
	}
}

void MyHttp::cannot_execute(boost::shared_ptr<TCP::socket> client_ptr)
{
	using boost::asio::buffer;
	client_ptr->write_some(buffer(cannot_msg));
}

void MyHttp::end_send(boost::shared_ptr<TCP::socket>client_ptr)
{
	using boost::asio::buffer;
	client_ptr->write_some(buffer(end_msg));
}

void MyHttp::headers(boost::shared_ptr<TCP::socket> client_ptr)
{
	using boost::asio::buffer;
	client_ptr->write_some(buffer(ok_msg,strlen(ok_msg)));
}

void MyHttp::not_found(boost::shared_ptr<TCP::socket>client_ptr)
{
	using boost::asio::buffer;
	client_ptr->write_some(buffer(miss_msg));
}


int main()
{
	MyHttp serve;
	serve.start_up(1234);
    return 0;
}

