#include "stdafx.h"
#include "Global.h"
#include "GBNSender.h"
#define WINDOWS_SIZE 4
#define NUMBER_SIZE 8


GBNSender::GBNSender() : waitingState(false)
{
	windows_size = WINDOWS_SIZE;
	number_size = NUMBER_SIZE;
	base = 1;
	nextseqnum = 1;
}

GBNSender::~GBNSender()
{
}

bool GBNSender::getWaitingState() {
	return waitingState;
}


bool GBNSender::send(Message &message) {

	//维护窗口的头base和nextseqnum
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}

	if (!waitingState) {
		//封装一个将要发送的数据包将该包保存在缓存的nextseqnum位置
		this->cache[nextseqnum].acknum = -1; //忽略该字段
		this->cache[nextseqnum].seqnum = nextseqnum;
		this->cache[nextseqnum].checksum = 0;
		memcpy(this->cache[nextseqnum].payload, message.data, sizeof(message.data));
		this->cache[nextseqnum].checksum = pUtils->calculateCheckSum(this->cache[nextseqnum]);

		pUtils->printPacket("发送方发送报文", this->cache[nextseqnum]);

		if (base == nextseqnum){pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);}//如果base=nextseqnum则开始计时
		pns->sendToNetworkLayer(RECEIVER, this->cache[nextseqnum]);//向网络层交付数据包
	}

	nextseqnum++;
	if (nextseqnum >= NUMBER_SIZE)nextseqnum = 0;//如果nextseqnum超过报文序号则next变为0


	//维护waitingState的值
	if (base <= nextseqnum) {
		if (nextseqnum < base + WINDOWS_SIZE)waitingState = true;
		else waitingState = false; 
	}			
	else {
		if (nextseqnum < base + WINDOWS_SIZE - NUMBER_SIZE)waitingState = true;
		else waitingState = false; 
	}
	return true;
}

void GBNSender::receive(Packet &ackPkt) {
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		int  clock_num=base;
		
		if (checkSum == ackPkt.checksum) {
			base = ackPkt.acknum + 1;
			if (base >= (NUMBER_SIZE))base = base - NUMBER_SIZE;
			this->waitingState = false;

			if (base == nextseqnum)pns->stopTimer(SENDER, clock_num);//发送完所有数据 关闭定时器
			else {
				pns->stopTimer(SENDER, clock_num);//关闭定时器
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//重启计时器
			}
		}
}

		


void GBNSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	int i = base;
	while (i!=nextseqnum) {//重发数据
		printf("debug debug debug debug debug debug\n");
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->cache[i]);
		pns->sendToNetworkLayer(RECEIVER, this->cache[i]);//向网络层交付数据包
		i++;
		if (i >= NUMBER_SIZE)i = 0;
		
	}
	

}
