#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"

#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8
SRReceiver::SRReceiver() :expectSequenceNumberRcvd(0)
{
	int i=0;
	while (i<NUMBER_SIZE) {
		flag[i] = 0;
		i++;
	}
	base = 1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.seqnum = -1;//忽略该字段
}


SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(Packet &packet) {

	Message msg;
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	int in_WINDOWS = 0;
	int in_front_windows = 0;

	if (base <= packet.seqnum) {
		if (packet.seqnum < (base + WINDOWS_SIZE))in_WINDOWS = 1;
		else in_WINDOWS = 0;
		if ((base != packet.seqnum) && (base + NUMBER_SIZE - WINDOWS_SIZE <= packet.seqnum))in_front_windows = 1;
		else in_front_windows = 0;

	}
	else if (base > packet.seqnum) {
		if (base + WINDOWS_SIZE - NUMBER_SIZE > packet.seqnum)in_WINDOWS = 1;
		else in_WINDOWS = 0;
		if (base - WINDOWS_SIZE <= packet.seqnum)in_front_windows = 1;
		else in_front_windows = 0;
	}



	if ((checkSum == packet.checksum) && in_front_windows == 1) {
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	}
	else if ((checkSum == packet.checksum) && (in_WINDOWS) && (flag[packet.seqnum] != 1)) {//检测其在不在窗口范围之内 以及测试校验和

		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		//将包的内容写到cache之中
		cache[packet.seqnum].seqnum = packet.seqnum;
		memcpy(cache[packet.seqnum].payload, packet.payload, sizeof(packet.payload));
		flag[packet.seqnum] = 1;
		//当base的值与packet的值相同时
		if (packet.seqnum == base) {
			flag[packet.seqnum] = 0;
			//将这个数据交付给应用层
			memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			base++;
			if (base >= NUMBER_SIZE)base = 0;
			printf("接受方窗口左一一位 base的值为：%d\n", base);
			while (flag[base]) {//当后面的数据也被确认时 交付给应用层
				memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				flag[base] = 0;
				base++;
				
				if (base >= NUMBER_SIZE)base = 0;
				printf("接受方窗口左一一位 base的值为：%d  \n", base);
			}
		}
		else {//校验和和窗口检测失败
			if (checkSum != packet.checksum) {
				pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
			}
			else if (in_WINDOWS == 0) {
				pUtils->printPacket("序列号在窗口之外", packet);
			}
			else if (flag[packet.seqnum] = 1) {
				pUtils->printPacket("重复发送了数据", packet);
			}

		}
	}
}