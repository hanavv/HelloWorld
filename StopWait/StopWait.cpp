// StopWait.cpp : 定义控制台应用程序的入口点。
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
	printf("请选择需要运的方案：\n1.StopWait\n2.GBN\n3.SR\n4.简易TCP\n");
	printf("请输入你的方案：");
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
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
    
	
	return 0;
}

