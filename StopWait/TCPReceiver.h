#ifndef TCP_RECEIVER_H
#define TCP_RECEIVER_H
#include "RdtReceiver.h"
#define NUMBER_SIZE 8
class TCPReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet cache[NUMBER_SIZE];
	Packet lastAckPkt;
	int flag[NUMBER_SIZE];
	int base;



public:
	TCPReceiver();
	virtual ~TCPReceiver();

public:

	void receive(Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

