#include "stdafx.h"
#include "Global.h"
#include "TCPSender.h"
#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8

TCPSender::TCPSender() :expectSequenceNumberSend(0), waitingState(false)
{
	base = 1;
	nextseqnum = 1;
	more_ack = 1;
}

TCPSender::~TCPSender()
{
}


bool TCPSender::getWaitingState() {
	return waitingState;
}


bool TCPSender::send(Message &message) {
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
		if (base == nextseqnum) { pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); }//如果base=nextseqnum则开始计时
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


void TCPSender::receive(Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	int  clock_num = base;

	if (checkSum == ackPkt.checksum) {
		
		int in_windows = 0;//在不在窗口的范围之内的标识符
		int front_base = 0;//是否为base的前一位的标识符
		if (base <= ackPkt.acknum) {
			if (ackPkt.acknum == (NUMBER_SIZE-1)) front_base = 1;
			else front_base = 0;

			//if (packet.seqnum < (base + WINDOWS_SIZE))in_WINDOWS = 1;
			if ((base + WINDOWS_SIZE) > ackPkt.acknum)in_windows = 1;
			else in_windows = 0;

		}
		else if (base > ackPkt.acknum) {
			if ((base - 1)==ackPkt.acknum) front_base = 1;
			else front_base = 0;
			//if (base + WINDOWS_SIZE - NUMBER_SIZE > packet.seqnum)in_WINDOWS = 1;
			if ((base+WINDOWS_SIZE-NUMBER_SIZE) > ackPkt.acknum)in_windows = 1;
			else in_windows = 0;	
			
		}

		printf("此时的nextseqnum=%d base=%d in—windows=%d front—base=%d\n",nextseqnum,base,in_windows,front_base);
		if (in_windows) {//在窗口之中
			base = ackPkt.acknum + 1;
			if (base >= (NUMBER_SIZE))base = 0;//超过NUMBER_SIZE需要回到0
			more_ack = 1;//冗余ack的数量 移动发送端窗口之后会被置为1
			this->waitingState = false;//移动窗口之后 使得发送端不为等待态（有剩余的空间）
			if (base == nextseqnum)pns->stopTimer(SENDER, clock_num);//发送完所有数据 关闭定时器
			else {
				pns->stopTimer(SENDER, clock_num);//关闭定时器
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//重启计时器
			}
		}

		else if (front_base) {//acknum为base的前一位
			more_ack++;
			if (more_ack == 3) {//冗余ack等于3的时候进行TCP的快速重传
				int i = base;
				while (i != nextseqnum) {//TCP的快速重传
					pUtils->printPacket("收到3个冗余的ack，进行快速重传", this->cache[base]);
					pns->sendToNetworkLayer(RECEIVER, this->cache[base]);//向网络层交付数据包
					i++;	
				}
				more_ack = 0;//快速重传一次之后重置冗余ack数量
			}
		}
	}
	else {
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", ackPkt);
	}	
}



void TCPSender::timeoutHandler(int seqNum) {
	///唯一一个定时器,无需考虑seqNum
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
    pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->cache[seqNum]);
    pns->sendToNetworkLayer(RECEIVER, this->cache[seqNum]);               //向网络层交付数据包 TCP中仅重传最早的一个没有被确认的包
}
