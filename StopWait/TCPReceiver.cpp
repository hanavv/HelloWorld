#include "stdafx.h"
#include "Global.h"
#include "TCPReceiver.h"

#define WINDOWS_SIZE 3
#define NUMBER_SIZE 8

TCPReceiver::TCPReceiver() :expectSequenceNumberRcvd(1)
{
	int i = 0;
	while (i < NUMBER_SIZE) {
		flag[i] = 0;
		i++;
	}
	base = 1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.seqnum = -1;//���Ը��ֶ�
}


TCPReceiver::~TCPReceiver()
{
}

void TCPReceiver::receive(Packet &packet) {
	Message msg;
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	int in_WINDOWS = 0;//�ڲ��ڴ��ڵķ�Χ֮�ڵı�ʶ��
	int base_1;
	if (base <= packet.seqnum) {
		if ((base + WINDOWS_SIZE) > packet.seqnum)in_WINDOWS = 1;
		else in_WINDOWS = 0;

	}
	else if (base > packet.seqnum) {
		if ((base + WINDOWS_SIZE - NUMBER_SIZE) > packet.seqnum)in_WINDOWS = 1;
		else in_WINDOWS = 0;
	}
	
	
	if ((checkSum == packet.checksum) && (in_WINDOWS) && (flag[packet.seqnum] != 1)) {//������ڲ��ڴ��ڷ�Χ֮�� �Լ�����У����
		cache[packet.seqnum].seqnum = packet.seqnum; flag[packet.seqnum] = 1;//����������д��cache֮��
		memcpy(cache[packet.seqnum].payload, packet.payload, sizeof(packet.payload));

		if (packet.seqnum == base) {//��base��ֵ��packet��ֵ��ͬʱ
			flag[packet.seqnum] = 0;
			memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));//��������ݽ�����Ӧ�ò�
			pns->delivertoAppLayer(RECEIVER, msg);
			base++; if (base >= NUMBER_SIZE)base = 0;
			//printf("���ܷ�������һһλ base��ֵΪ��%d\n", base);

			while (flag[base]) {//�����������Ҳ��ȷ��ʱ ������Ӧ�ò�
				memcpy(msg.data, cache[base].payload, sizeof(cache[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				flag[base] = 0;
				base++; if (base >= NUMBER_SIZE)base = 0;
				//printf("���ܷ�������һһλ base��ֵΪ��%d  \n", base);
			}
		}
		if (base ==0) {
			base_1 = (NUMBER_SIZE - 1);
		}
		else {
			base_1 = base - 1;
		}
		lastAckPkt.acknum = base_1; //����һ��ack acknum��ֵΪ�ƶ���֮��δȷ�ϵĵ�һλ
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	}
	else if (checkSum == packet.checksum) {
		if (base == 0) {
			base_1 = (NUMBER_SIZE - 1);
		}
		else {
			base_1 = base - 1;
		}
		lastAckPkt.acknum = base_1; //����һ��ack acknum��ֵΪ�ƶ���֮��δȷ�ϵĵ�һλ
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
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