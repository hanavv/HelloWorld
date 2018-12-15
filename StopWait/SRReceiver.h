#ifndef SR_RECEIVER_H
#define SR_RECEIVER_H
#include "RdtReceiver.h"
#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8
class SRReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet cache[NUMBER_SIZE];
	Packet lastAckPkt;
	int flag[NUMBER_SIZE];
	int base;
public:
	SRReceiver();
	virtual ~SRReceiver();

public:

	void receive(Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

