#include "DemoLog.h"



bool DemoLog::updateClient(string  message)
{

	ocall_update_client(message.c_str());
	return true;
}
