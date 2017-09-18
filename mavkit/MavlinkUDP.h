#ifndef MAVLINK_UDP_H
#define MAVLINK_UDP_H

#include <thread>
#include <mutex>

#include <winsock2.h>
#pragma comment(lib,"WS2_32.lib")//Á¬½ÓÌ×½Ó×Ö¿â

#include <vector>
#include <string>
#include "MavMessengerInterface.h"
#include "mavlinklibrary/common/mavlink.h"
using namespace std;


class MavlinkUDP : public MavMessengerInterface
{
public:
	//Client constructor
	MavlinkUDP(string target_ip, int target_port);
	//Server constructor
	MavlinkUDP(int local_port);
	~MavlinkUDP();

	static bool is_valid_ip(const char* ip);
	bool send_message(mavlink_message_t &msg);
	void append_listener(MavMessengerInterface* listener);
	void start();
	void join();

private:
	thread *reading_thread;
	mutex mutex;
	vector<MavMessengerInterface*> listeners;
	void read_loop();


	SOCKET sock;
	sockaddr_in gcAddr;
	thread *buffering_thread;
	int storage_pipe[2];
	void bufferize();
};

#endif

