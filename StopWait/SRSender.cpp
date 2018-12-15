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
	//ά�����ڵ�ͷbase��nextseqnum
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬

		return false;
	}

	if (!waitingState) {
		//��װһ����Ҫ���͵����ݰ����ð������ڻ����nextseqnumλ��
		this->cache[nextseqnum].acknum = -1; //���Ը��ֶ�
		this->cache[nextseqnum].seqnum = nextseqnum;
		this->cache[nextseqnum].checksum = 0;
		flag[nextseqnum] = 0;
		memcpy(this->cache[nextseqnum].payload, message.data, sizeof(message.data));
		this->cache[nextseqnum].checksum = pUtils->calculateCheckSum(this->cache[nextseqnum]);

		pUtils->printPacket("���ͷ����ͱ���", this->cache[nextseqnum]);
		pns->stopTimer(SENDER,this->nextseqnum);
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->nextseqnum);
		pns->sendToNetworkLayer(RECEIVER, this->cache[nextseqnum]);//������㽻�����ݰ�
	}
	nextseqnum++;
	if (nextseqnum >= NUMBER_SIZE)nextseqnum = 0;//���nextseqnum�������������next��Ϊ0


	//ά��waitingState��ֵ
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
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	int  clock_num = base;
	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("��ȷ��ACK", ackPkt);
		pns->stopTimer(SENDER, ackPkt.acknum);
		flag[ackPkt.acknum] = 1;
		if (ackPkt.acknum == base) {
			flag[ackPkt.acknum] = 0;
			base++;
			waitingState = false;
			
			if (base >= NUMBER_SIZE)base = 0;
			printf("���ͷ�������һһλ base��ֵΪ��%d   next:%d\n", base,nextseqnum);
			while (flag[base]) {
				if (base == nextseqnum)break;
				flag[ackPkt.acknum] = 0;
				base++;
				if (base >= NUMBER_SIZE)base = 0;
				printf("���ͷ�������һһλ base��ֵΪ��%d   next:%d\n", base, nextseqnum);
			}
		}
    }
		
}

void SRSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	    pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
		printf("debug debug debug debug debug debug\n");
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->cache[seqNum]);
		pns->sendToNetworkLayer(RECEIVER, this->cache[seqNum]);//������㽻�����ݰ�
}
