// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNSender.h"
#include "GBNReceiver.h"
#include "SRReceiver.h"
#include "SRSender.h"
#include "TCPSender.h"
#include "TCPReceiver.h"


int main(int argc, char** argv[])
{
	int choice = 0;
	printf("��ѡ����Ҫ�˵ķ�����\n1.StopWait\n2.GBN\n3.SR\n4.����TCP\n");
	printf("��������ķ�����");
	scanf("%d", &choice);
	RdtSender *ps;
	RdtReceiver * pr;

	if (choice == 1)
	{
		ps = new StopWaitRdtSender();
		pr = new StopWaitRdtReceiver();
	}
	else if (choice == 2)
	{
		ps = new GBNSender();
		pr = new GBNReceiver();
	}
	else if (choice == 3)
	{
		ps = new SRSender();
		pr = new SRReceiver();
	}
	else if (choice == 4)
	{
		ps = new TCPSender();
		pr = new TCPReceiver();
	}
	else
	{
		ps = new StopWaitRdtSender();
		pr = new StopWaitRdtReceiver();
	}
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\VS\\rdt-win-student\\rdt-win-student\\input.txt");
	pns->setOutputFile("C:\\VS\\rdt-win-student\\rdt-win-student\\output.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
    
	
	return 0;
}

