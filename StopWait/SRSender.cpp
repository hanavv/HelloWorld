#include "stdafx.h"
#include "Global.h"
#include "SRSender.h"


#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8

SRSender::SRSender() :waitingState(false)
{
	int i = 0;
	while (i < NUMBER_SIZE) {
		flag[i] = 0;
		i++;
	}
	base = 1;
	nextseqnum = 1;
}

SRSender::~SRSender()
{
}


bool SRSender::getWaitingState() {
	return waitingState;
}


bool SRSender::send(Message &message) {
	//维护窗口的头base和nextseqnum
	if (this->waitingState) { //发送方处于等待确认状态

		return false;
	}

	if (!waitingState) {
		//封装一个将要发送的数据包将该包保存在缓存的nextseqnum位置
		this->cache[nextseqnum].acknum = -1; //忽略该字段
		this->cache[nextseqnum].seqnum = nextseqnum;
		this->cache[nextseqnum].checksum = 0;
		flag[nextseqnum] = 0;
		memcpy(this->cache[nextseqnum].payload, message.data, sizeof(message.data));
		this->cache[nextseqnum].checksum = pUtils->calculateCheckSum(this->cache[nextseqnum]);

		pUtils->printPacket("发送方发送报文", this->cache[nextseqnum]);
		pns->stopTimer(SENDER,this->nextseqnum);
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->nextseqnum);
		pns->sendToNetworkLayer(RECEIVER, this->cache[nextseqnum]);//向网络层交付数据包
	}
	nextseqnum++;
	if (nextseqnum >= NUMBER_SIZE)nextseqnum = 0;//如果nextseqnum超过报文序号则next变为0


	//维护waitingState的值
	if (base <= nextseqnum) {
		if (nextseqnum < (base + WINDOWS_SIZE))waitingState = false;
		else waitingState = true;
	}
	else {
		if (nextseqnum < (base + WINDOWS_SIZE - NUMBER_SIZE))waitingState = false;
		else waitingState = true;
	}
	return true;
}


void SRSender::receive(Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	int  clock_num = base;
	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("已确认ACK", ackPkt);
		pns->stopTimer(SENDER, ackPkt.acknum);
		flag[ackPkt.acknum] = 1;
		if (ackPkt.acknum == base) {
			flag[ackPkt.acknum] = 0;
			base++;
			waitingState = false;
			
			if (base >= NUMBER_SIZE)base = 0;
			printf("发送方窗口左一一位 base的值为：%d   next:%d\n", base,nextseqnum);
			while (flag[base]) {
				if (base == nextseqnum)break;
				flag[ackPkt.acknum] = 0;
				base++;
				if (base >= NUMBER_SIZE)base = 0;
				printf("发送方窗口左一一位 base的值为：%d   next:%d\n", base, nextseqnum);
			}
		}
    }
		
}

void SRSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	    pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
		printf("debug debug debug debug debug debug\n");
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->cache[seqNum]);
		pns->sendToNetworkLayer(RECEIVER, this->cache[seqNum]);//向网络层交付数据包
}
