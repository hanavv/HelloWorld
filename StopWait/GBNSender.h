#ifndef GBN_SENDER_H
#define GBN_SENDER_H
#include "RdtSender.h"

#define WINDOWS_SIZE 4
#define NUMBER_SIZE 8


class GBNSender :public RdtSender
{
private:
	bool waitingState;				// 是否处于等待Ack的状态
	int windows_size;
	int number_size;
	int base;
	int nextseqnum;
	Packet cache[NUMBER_SIZE];

public:

	bool getWaitingState();
	bool send(Message &message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(Packet &ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNSender();
	virtual ~GBNSender();
};

#endif

