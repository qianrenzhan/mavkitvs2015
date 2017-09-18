#include <iostream>
#include "MavlinkUDP.h"
//using namespace std;

//----------------------------------------------------------------------------//
MavlinkUDP::MavlinkUDP(string target_ip, int target_port)
	: reading_thread(NULL),
	buffering_thread(NULL)
{
	//int result = pipe(storage_pipe);
	//if (result == -1)
	//	throw std::logic_error(std::string("cannot open internal pipe for fast buffering: ") + strerror(errno));

	if (target_port < 0 || target_port > 65535)
		throw logic_error(string("Port outside range."));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&gcAddr, 0, sizeof(struct sockaddr_in));
	gcAddr.sin_family = AF_INET;
	gcAddr.sin_port = htons(target_port);
	gcAddr.sin_addr.s_addr = inet_addr(target_ip.c_str());

	cout << "to " << target_ip << ":" << target_port << endl;

	reading_thread = new thread(&MavlinkUDP::read_loop, this);
	buffering_thread = new thread(&MavlinkUDP::bufferize, this);
}
//----------------------------------------------------------------------------//
MavlinkUDP::MavlinkUDP(int local_port)
	: reading_thread(NULL),
	buffering_thread(NULL)
{
	/////////////////////////////////////////////////////////////////////////////////////////////
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 1);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		printf("WSA error! \n");
	}

	struct sockaddr_in locAddr;
	memset(&locAddr, 0, sizeof(struct sockaddr_in));
	memset(&gcAddr, 0, sizeof(struct sockaddr_in));
	locAddr.sin_family = AF_INET;    
	locAddr.sin_port = htons(local_port);
	locAddr.sin_addr.s_addr = INADDR_ANY;      //本地服务器端

	//bind socket to local_port
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (::bind(sock, (sockaddr *)&locAddr, sizeof(locAddr)) == -1)
	{
		//close(sock);
		printf("bind error !");
		throw std::logic_error(std::string("error bind failed: ") + strerror(errno));
	}

	std::cout << "from " << inet_ntoa(locAddr.sin_addr) << ":" << local_port << std::endl;

	reading_thread = new thread(&MavlinkUDP::read_loop, this);
	buffering_thread = new thread(&MavlinkUDP::bufferize, this);
}
//----------------------------------------------------------------------------//
MavlinkUDP::~MavlinkUDP()
{
	//close(sock);
	//close(storage_pipe[0]);
	//close(storage_pipe[1]);
	//TODO stop bufferize thread
}
//----------------------------------------------------------------------------//
bool MavlinkUDP::is_valid_ip(const char* ip)
{
	//char* dst[INET_ADDRSTRLEN];
	//return inet_pton(AF_INET, ip, dst) == 1;
	return 0;
}
//----------------------------------------------------------------------------//
void MavlinkUDP::append_listener(MavMessengerInterface* listener)
{
	if (listener != NULL)
		listeners.push_back(listener);
}
//----------------------------------------------------------------------------//
void MavlinkUDP::start()
{
	if (reading_thread == NULL && buffering_thread == NULL)
	{
		reading_thread = new std::thread(&MavlinkUDP::read_loop, this);
		buffering_thread = new std::thread(&MavlinkUDP::bufferize, this);
	}
}
//----------------------------------------------------------------------------//
void MavlinkUDP::join()
{
	reading_thread->join();
}
//----------------------------------------------------------------------------//
void MavlinkUDP::read_loop()
{
	int fromlen = sizeof(gcAddr);
	char buffer[1024] = "\0";
	mavlink_status_t status;
	mavlink_message_t message;
	while (true)
	{
		int nb_read = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&gcAddr, &fromlen);
		if (nb_read > 0)
		{
			for (int i = 0; i < nb_read; i++)
			{
				if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status))
				{
					printf("received message with id %d,sequence %d,from component %d of system %d!\n", message.msgid, message.seq, message.compid, message.sysid);
					//printf("message ID:%d,system ID:%d\r\n", message.msgid, message.sysid);
				}
			}
		}
	}
}
//----------------------------------------------------------------------------//
// Thread loop to move data from RX buffer to RAM as fast as possible
// so as to avoid buffer overflows if RX buffer is small.
void MavlinkUDP::bufferize()
{
	while (1);
}
//----------------------------------------------------------------------------//
bool MavlinkUDP::send_message(mavlink_message_t &msg)
{
	uint8_t buf[512];
	int bytes_sent;
	uint16_t len;
	len = mavlink_msg_to_send_buffer(buf, &msg);

	mutex.lock();
	bytes_sent = sendto(sock, (const char*)buf, len, 0, (struct sockaddr*)&gcAddr, sizeof(struct sockaddr_in));
	mutex.unlock();

	return len == bytes_sent;
}
//----------------------------------------------------------------------------//
