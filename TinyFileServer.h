#pragma once
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<boost/asio.hpp>
#include<map>
#include<thread>
#include<mutex>
#include<boost/shared_ptr.hpp>
#include<boost/thread.hpp>
class MyHttp
{
	typedef boost::asio::ip::tcp TCP;
public:
	MyHttp();
	~MyHttp();
	void start_up(unsigned short);
private:
	void accept_request(boost::shared_ptr<TCP::socket>);
	void cat(boost::shared_ptr<TCP::socket>, std::ifstream&);
	void cannot_execute(boost::shared_ptr<TCP::socket>);
	void end_send(boost::shared_ptr<TCP::socket>);
	void headers(boost::shared_ptr<TCP::socket>);
	void not_found(boost::shared_ptr<TCP::socket>);
	boost::asio::io_service ios;
	const std::string GET_METHOD = "GET ";
	const std::string POST_METHOD = "POST ";
	const char ok_msg[5]  = "OK\r\n";
	const char miss_msg[7]  = "MISS\r\n";
	const char cannot_msg[9] = "CANNOT\r\n";
	const char end_msg[6] = "END\r\n";
	std::mutex map_mutex;
	std::map<std::string, bool> is_vis;
};