#ifndef SR_RECEIVER_H
#define SR_RECEIVER_H
#include "RdtReceiver.h"
#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8
class SRReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet cache[NUMBER_SIZE];
	Packet lastAckPkt;
	int flag[NUMBER_SIZE];
	int base;
public:
	SRReceiver();
	virtual ~SRReceiver();

public:

	void receive(Packet &packet);	//接收报文，将被NetworkService调用
};

#endif

