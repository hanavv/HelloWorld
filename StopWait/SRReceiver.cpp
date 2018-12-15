#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"

#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8
SRReceiver::SRReceiver() :expectSequenceNumberRcvd(0)
{
	int i=0;
	while (i<NUMBER_SIZE) {
		flag[i] = 0;
		i++;
	}
	base = 1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.seqnum = -1;//���Ը��ֶ�
}


SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(Packet &packet) {

	Message msg;
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	int in_WINDOWS = 0;
	int in_front_windows = 0;

	if (base <= packet.seqnum) {
		if (packet.seqnum < (base + WINDOWS_SIZE))in_WINDOWS = 1;
		else in_WINDOWS = 0;
		if ((base != packet.seqnum) && (base + NUMBER_SIZE - WINDOWS_SIZE <= packet.seqnum))in_front_windows = 1;
		else in_front_windows = 0;

	}
	else if (base > packet.seqnum) {
		if (base + WINDOWS_SIZE - NUMBER_SIZE > packet.seqnum)in_WINDOWS = 1;
		else in_WINDOWS = 0;
		if (base - WINDOWS_SIZE <= packet.seqnum)in_front_windows = 1;
		else in_front_windows = 0;
	}



	if ((checkSum == packet.checksum) && in_front_windows == 1) {
		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	}
	else if ((checkSum == packet.checksum) && (in_WINDOWS) && (flag[packet.seqnum] != 1)) {//������ڲ��ڴ��ڷ�Χ֮�� �Լ�����У���

		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

		//����������д��cache֮��
		cache[packet.seqnum].seqnum = packet.seqnum;
		memcpy(cache[packet.seqnum].payload, packet.payload, sizeof(packet.payload));
		flag[packet.seqnum] = 1;
		//��base��ֵ��packet��ֵ��ͬʱ
		if (packet.seqnum == base) {
			flag[packet.seqnum] = 0;
			//��������ݽ�����Ӧ�ò�
			memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			base++;
			if (base >= NUMBER_SIZE)base = 0;
			printf("���ܷ�������һһλ base��ֵΪ��%d\n", base);
			while (flag[base]) {//�����������Ҳ��ȷ��ʱ ������Ӧ�ò�
				memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				flag[base] = 0;
				base++;
				
				if (base >= NUMBER_SIZE)base = 0;
				printf("���ܷ�������һһλ base��ֵΪ��%d  \n", base);
			}
		}
		else {//У��ͺʹ��ڼ��ʧ��
			if (checkSum != packet.checksum) {
				pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
			}
			else if (in_WINDOWS == 0) {
				pUtils->printPacket("���к��ڴ���֮��", packet);
			}
			else if (flag[packet.seqnum] = 1) {
				pUtils->printPacket("�ظ�����������", packet);
			}

		}
	}
}