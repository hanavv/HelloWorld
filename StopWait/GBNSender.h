#ifndef GBN_SENDER_H
#define GBN_SENDER_H
#include "RdtSender.h"

#define WINDOWS_SIZE 4
#define NUMBER_SIZE 8


class GBNSender :public RdtSender
{
private:
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	int windows_size;
	int number_size;
	int base;
	int nextseqnum;
	Packet cache[NUMBER_SIZE];

public:

	bool getWaitingState();
	bool send(Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	GBNSender();
	virtual ~GBNSender();
};

#endif

