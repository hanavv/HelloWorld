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
		if (base == nextseqnum) { pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); }//���base=nextseqnum��ʼ��ʱ
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


void TCPSender::receive(Packet &ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	int  clock_num = base;

	if (checkSum == ackPkt.checksum) {
		
		int in_windows = 0;//�ڲ��ڴ��ڵķ�Χ֮�ڵı�ʶ��
		int front_base = 0;//�Ƿ�Ϊbase��ǰһλ�ı�ʶ��
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

		printf("��ʱ��nextseqnum=%d base=%d in��windows=%d front��base=%d\n",nextseqnum,base,in_windows,front_base);
		if (in_windows) {//�ڴ���֮��
			base = ackPkt.acknum + 1;
			if (base >= (NUMBER_SIZE))base = 0;//����NUMBER_SIZE��Ҫ�ص�0
			more_ack = 1;//����ack������ �ƶ����Ͷ˴���֮��ᱻ��Ϊ1
			this->waitingState = false;//�ƶ�����֮�� ʹ�÷��Ͷ˲�Ϊ�ȴ�̬����ʣ��Ŀռ䣩
			if (base == nextseqnum)pns->stopTimer(SENDER, clock_num);//�������������� �رն�ʱ��
			else {
				pns->stopTimer(SENDER, clock_num);//�رն�ʱ��
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//������ʱ��
			}
		}

		else if (front_base) {//acknumΪbase��ǰһλ
			more_ack++;
			if (more_ack == 3) {//����ack����3��ʱ�����TCP�Ŀ����ش�
				int i = base;
				while (i != nextseqnum) {//TCP�Ŀ����ش�
					pUtils->printPacket("�յ�3�������ack�����п����ش�", this->cache[base]);
					pns->sendToNetworkLayer(RECEIVER, this->cache[base]);//������㽻�����ݰ�
					i++;	
				}
				more_ack = 0;//�����ش�һ��֮����������ack����
			}
		}
	}
	else {
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", ackPkt);
	}	
}



void TCPSender::timeoutHandler(int seqNum) {
	///Ψһһ����ʱ��,���迼��seqNum
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
    pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->cache[seqNum]);
    pns->sendToNetworkLayer(RECEIVER, this->cache[seqNum]);               //������㽻�����ݰ� TCP�н��ش������һ��û�б�ȷ�ϵİ�
}
