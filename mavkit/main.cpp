#include"MavlinkUDP.h"
using namespace std;


void main()
{
	MavlinkUDP mavlinkudp(14550);

	mavlinkudp.start();

	while (1)
	{
		//int system_id = 1;
		//int component_id = 1;
		////必须是1,1才能发送成功目标点
		//mavlink_message_t msg;

		//mavlink_msg_heartbeat_pack(system_id, component_id, &msg, MAV_TYPE_SUBMARINE, MAV_AUTOPILOT_GENERIC, MAV_MODE_PREFLIGHT, 0, MAV_STATE_STANDBY);
		//mavlinkudp.send_message(msg);
		//Sleep(1000);
	}
}