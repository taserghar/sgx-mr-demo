#include "message.h"



void Message::dummySendMessage()
{
 

  // amqp_basic_publish(conn, KChannel, amqp_empty_bytes, /* routing key*/ queueName, false, false, nullptr, amqp_cstring_bytes("Message from SGX-MR Dev"));
  // std::cout << " [x] Sent 'Hello World!'" << std::endl;


}

Message::Message()
{
	std::cout << "[ SGX-MR-APP ] Message Queue Init" << std::endl;
	conn = amqp_new_connection();
  amqp_socket_t *socket = amqp_tcp_socket_new(conn);
  amqp_socket_open(socket, "localhost", AMQP_PROTOCOL_PORT);
  amqp_login(conn, "/", 0, AMQP_DEFAULT_FRAME_SIZE, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  amqp_channel_open(conn, KChannel);

 queueName = amqp_bytes_t(amqp_cstring_bytes("hello"));
  amqp_queue_declare(conn, KChannel, queueName, false, false, false, false, amqp_empty_table);

  // amqp_basic_publish(conn, KChannel, amqp_empty_bytes, /* routing key*/ queueName, false, false, nullptr, amqp_cstring_bytes("Message from SGX-MR Dev"));
	
	//For message dedup id
	//srand((unsigned)time(NULL));
}

Message::~Message()
{
	//std::cout << "Message Destructor" << std::endl;
  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);
}
	


bool Message::sendMessage(const char* msg)
{
	//This code is almost word-for-word from the example code
	bool success = true;

	amqp_basic_publish(conn, KChannel, amqp_empty_bytes, /* routing key*/ queueName, false, false, nullptr, amqp_cstring_bytes(msg));
  // std::cout << msg << std::endl;

	return success;
}

//This method does not seem to be need but I am going to keep it around just in case I need it later
std::string Message::getRandString(int len)
{
	std::string strReturn = "";
	const char characters[] = "abcdefghijklmnopqrstuvxyzABCDEFGHIGKLMNOPQRSTUVXYZ";
	for(int i = 0; i < len; i++)
		strReturn += characters[rand() % (sizeof(characters) - 1)];
	return strReturn;
}

