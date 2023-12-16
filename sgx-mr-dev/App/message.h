#ifndef __MESSAGE__
#define __MESSAGE__


#include <iostream>
#include <ctime>
#include <stdlib.h>

#include <string.h>
#include <iostream>

#include <amqp.h>
#include <amqp_tcp_socket.h>


class Message
{
	public:
		Message();
		~Message();
		//Sends message with specified body
		bool sendMessage(const char* msg);
		//Returns a random string for dedup purposes
		std::string getRandString(int len);
		void dummySendMessage();

		amqp_connection_state_t conn;
		amqp_bytes_t queueName;
		const amqp_channel_t KChannel = 1;
	
};




#endif
