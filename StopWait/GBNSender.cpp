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

	//ά�����ڵ�ͷbase��nextseqnum
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	if (!waitingState) {
		//��װһ����Ҫ���͵����ݰ����ð������ڻ����nextseqnumλ��
		this->cache[nextseqnum].acknum = -1; //���Ը��ֶ�
		this->cache[nextseqnum].seqnum = nextseqnum;
		this->cache[nextseqnum].checksum = 0;
		memcpy(this->cache[nextseqnum].payload, message.data, sizeof(message.data));
		this->cache[nextseqnum].checksum = pUtils->calculateCheckSum(this->cache[nextseqnum]);

		pUtils->printPacket("���ͷ����ͱ���", this->cache[nextseqnum]);

		if (base == nextseqnum){pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);}//���base=nextseqnum��ʼ��ʱ
		pns->sendToNetworkLayer(RECEIVER, this->cache[nextseqnum]);//������㽻�����ݰ�
	}

	nextseqnum++;
	if (nextseqnum >= NUMBER_SIZE)nextseqnum = 0;//���nextseqnum�������������next��Ϊ0


	//ά��waitingState��ֵ
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
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
		int  clock_num=base;
		
		if (checkSum == ackPkt.checksum) {
			base = ackPkt.acknum + 1;
			if (base >= (NUMBER_SIZE))base = base - NUMBER_SIZE;
			this->waitingState = false;

			if (base == nextseqnum)pns->stopTimer(SENDER, clock_num);//�������������� �رն�ʱ��
			else {
				pns->stopTimer(SENDER, clock_num);//�رն�ʱ��
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//������ʱ��
			}
		}
}

		


void GBNSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	int i = base;
	while (i!=nextseqnum) {//�ط�����
		printf("debug debug debug debug debug debug\n");
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->cache[i]);
		pns->sendToNetworkLayer(RECEIVER, this->cache[i]);//������㽻�����ݰ�
		i++;
		if (i >= NUMBER_SIZE)i = 0;
		
	}
	

}
