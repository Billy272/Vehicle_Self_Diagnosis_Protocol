#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "RapidDiagnosis.h"
#include "..\interface\protocol_define.h"
#include "..\SpecialFunction\special_function.h"
#include "..\protocol\iso_15765.h"

/*********************************************************************************
����:  �����б�BCM
*********************************************************************************/	
byte bCmdActiveECU[] = { 0x07, 0x25, 0x02, 0x10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};  //��������
byte bCmdIdlelinks[] = { 0x07, 0x25, 0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //��������
byte bCmdMulFrames[] = { 0x07, 0x25, 0x30, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};  //30֡����
byte bCmdScanedVIN[] = { 0x07, 0x25, 0x03, 0x22, 0xF1, 0x90, 0x00, 0x00, 0x00, 0x00};  //��ȡVIN����
byte bCmdClearDTCs[] = { 0x07, 0x25, 0x04, 0x14, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00};  //�������������

/*********************************************************************************
����:  ϵͳ��ͨ��ģʽ0�������ܽ�1�����͹ܽ�2��ͨ�Ų�����3��4��5���˲�ID����6���˲�ID 7��8��9��10  ���ο���λ����VDIͨ��Э����3.1
*********************************************************************************/	
byte bVDI_SET[11] = { 0x80, 0x03, 0x0B, 0x01, 0xE8, 0x48, 0x01, 0x00, 0x00, 0x07, 0xA5};

/*********************************************************************************
����:		�����б�����ƫ�ơ������ֽڡ�����ȡ���������ָ��
*********************************************************************************/
STRUCT_CMD g_stCmdList[] =
{
	{0, 0, 10, bCmdActiveECU},  //��������
	{1, 0, 10, bCmdIdlelinks},  //��������
	{2, 0, 10, bCmdMulFrames},  //30H֡����
	{3, 0, 10, bCmdScanedVIN},  //ɨ��VIN
	{4, 0, 10, bCmdClearDTCs},  //���������
};

/*********************************************************************************
����:		����ϵͳ��ͨ�����š�ͨ�Ų�����(K)��ͨ��ID(���޸ģ�ֻ��Ҫ�޸Ķ�Ӧϵͳ����
�ţ���Ҫ�ı�ϵͳ��ǰ��˳��),����չ(��δ�����ο�X35�Ŀ������)
*********************************************************************************/
uint32 uCarSystemData[][5] = 
{
	{ 6, 14, 500, 0x7E0, 0x7E8},  //EMS
	{ 3, 11, 125, 0x729, 0x7A9},  //PEPS
	{ 3, 11, 125, 0x725, 0x7A5},  //BCM
	{ 3, 11, 125, 0x724, 0x7A4},  //ICM 1��9
	{ 3, 11, 125, 0x720, 0x7A0},  //ICE 1��9
	{ 3, 11, 125, 0x721, 0x7A1},  //SDM
	{ 6, 14, 500, 0x710, 0x790},  //ESC
	{ 3, 11, 125, 0x723, 0x7A3},  //ACC
	{ 6, 14, 500, 0x715, 0x795},  //EPS_HL
	{ 6, 14, 500, 0x715, 0x795},  //EPS_LR
	{ 3, 11, 125, 0x72A, 0x7AA},  //ESCL
	{ 6, 14, 500, 0x7E1, 0x7E9},  //CVT
//����ϵͳM50F����Ҫ
	{ 6, 14, 500, 0x710, 0x790},  //ESP
	{ 6, 14, 500, 0x715, 0x795},  //EPS
	{ 3, 11, 500, 0x720, 0x7A0},  //HUM 1��9
	{ 3, 11, 500, 0x73A, 0x7BA},  //DVR 1��9
	{ 3, 11, 500, 0x737, 0x7B7},  //GW
	{ 3, 11, 500, 0x728, 0x7A8},  //PAS
	{ 6, 14, 500, 0x727, 0x7A7},  //AVM
	{ 3, 11, 500, 0x730, 0x7B0},  //APC

};
/*********************************************************************************
����:		�벻Ҫ�޸�ϵͳ��ǰ��˳����Ҫ���ϵͳ�����Խ�����������
*********************************************************************************/
enum _CarSystem_DTC
{
	EMS_SYS = 0,
	PEPS_SYS,
	BCM_SYS,
	ICM_SYS,
	ICE_SYS,
	SDM_SYS,
	ESC_SYS,
	ACC_SYS,
	EPS_HL_SYS,
	EPS_LR_SYS,
	ESCL_SYS,
	CVT_SYS,
//����ϵͳM50F����Ҫ
	ABS_SYS,
	ESP_SYS,
	EPS_SYS,
	HUM_SYS,
	DVR_SYS,
	GW_SYS,
	PAS_SYS,
	AVM_SYS,
	APC_SYS,
};
/*********************************************************************************
����:    ��ʼ����������ϵͳ(Ĭ��ͨ������Ϊ0x80���˲�����Ϊ1���˲�IDΪ��׼CAN)
*********************************************************************************/
void initialization_Car_system(byte bCarSystem)
{
	//��ʼ��VDI������
	//ͨ������
	bVDI_SET[1] = uCarSystemData[bCarSystem][0];
	bVDI_SET[2] = uCarSystemData[bCarSystem][1];
	//ECUͨ�Ų�����
	bVDI_SET[3] = uCarSystemData[bCarSystem][2] * 1000 >> 16 & 0xFF;
	bVDI_SET[4] = uCarSystemData[bCarSystem][2] * 1000 >> 8 & 0xFF;
	bVDI_SET[5] = uCarSystemData[bCarSystem][2] * 1000 & 0xFF;
	//�˲�ID
	bVDI_SET[9] = uCarSystemData[bCarSystem][4] >> 8 &0xFF;
	bVDI_SET[10] = uCarSystemData[bCarSystem][4] & 0xFF;

	//��ʼ������(��Ҫ��ʼ��ϵͳ��ͨ��ID,)
	bCmdActiveECU[1] = uCarSystemData[bCarSystem][3] & 0xFF;
	bCmdIdlelinks[1] = uCarSystemData[bCarSystem][3] & 0xFF;
	bCmdMulFrames[1] = uCarSystemData[bCarSystem][3] & 0xFF;
	bCmdScanedVIN[1] = uCarSystemData[bCarSystem][3] & 0xFF;
	bCmdClearDTCs[1] = uCarSystemData[bCarSystem][3] & 0xFF;
}

/*********************************************************************************
����:    ���ݴ�������ϵͳ������VDI
*********************************************************************************/
bool function_setting_VDI(void)
{
	g_p_stVCI_params_config->cVoltage = 1;
	g_p_stVCI_params_config->cLevel = 0;
	g_p_stVCI_params_config->cLogic = 0;
	//CAN֡ģʽ
	g_p_stVCI_params_config->cCANFrameMode = 0;
	//����λ��������115200
	g_p_stVCI_params_config->cVCIBaudRate[0] =0x01;     
	g_p_stVCI_params_config->cVCIBaudRate[1] =0xC2;
	g_p_stVCI_params_config->cVCIBaudRate[2] =0x00;
	//CAN����������
	g_p_stVCI_params_config->cCanSamplingConfig[0] =0;
	g_p_stVCI_params_config->cCanSamplingConfig[1] =0;
	g_p_stVCI_params_config->cCanSamplingConfig[2] =0;
	g_p_stVCI_params_config->cCanSamplingConfig[3] =0;

	g_p_stVCI_params_config->cCommunicationType=bVDI_SET[0];
	g_p_stVCI_params_config->cReceivePin = bVDI_SET[1];
	g_p_stVCI_params_config->cSendPin    = bVDI_SET[2];
	//ECUͨ�Ų�����
	g_p_stVCI_params_config->cECUBaudRate[0] = bVDI_SET[3];
	g_p_stVCI_params_config->cECUBaudRate[1] = bVDI_SET[4];
	g_p_stVCI_params_config->cECUBaudRate[2] = bVDI_SET[5];
	//�˲�ID����
	g_p_stVCI_params_config->cCanFilterIDGroupNum = bVDI_SET[6]; 
	//�˲�ID
	memcpy( g_p_stVCI_params_config->cCanFilterID,&bVDI_SET[7],  bVDI_SET[6]*4 );
	if( !package_and_send_vci_config() )
	{
		return false;
	}

	return true;
}

/*********************************************************************************
����:    ���ݴ�������ϵͳ������VDI
*********************************************************************************/
bool function_readVIN( byte * VINData )
{
	byte cBufferOffset = 0;//���������λ��ƫ����
	int piCmdIndex[2] = {1,3};
	int iReturnStatus = TIME_OUT;

	cBufferOffset = g_stCmdList[piCmdIndex[1]].cBufferOffset ;
	iReturnStatus = rapid_send_and_receive_cmd_by_iso_15765( piCmdIndex );
	if ( iReturnStatus!=SUCCESS )
	{
		return false;

	}

	if ( 0 == memcmp(g_stBufferGroup[cBufferOffset].cBuffer+3,"LNB",3) )
	{
		memcpy( VINData,g_stBufferGroup[cBufferOffset].cBuffer+3, 17 );
		return true;
	}
	else
	{
		return false;
	}	
}
/*********************************************************************************
����:    ����30֡�������ͽ�������
*********************************************************************************/
int function_Active_CAN()
{
	int piCmdIndex[] = {1,0};
	int iReturnStatus = TIME_OUT;

	iReturnStatus = bSend_FC_Frame_Cmd( &g_stCmdList[2], 4, 0, 0x10 );  //����30֡

	iReturnStatus = rapid_send_and_receive_cmd_by_iso_15765( piCmdIndex );
	if(iReturnStatus!= SUCCESS)
	{
		return false;
	}

	return true;
}
/*************************************************
Description: ��ʼ������������
*************************************************/
int Clear_DTC_start(int sysDTC)
{
	int bStatus = 0;
	int iCmdIndex[2] = {1,4};

	bStatus = function_setting_VDI();
	if (!bStatus)
	{
		return 1;
	}

	//�����������������
	bStatus = rapid_send_and_receive_cmd_by_iso_15765( iCmdIndex );
	if (bStatus)
	{
		return 2;
	}

	return SUCCESS;
}

/*************************************************
Description:	ɨ��BCM,PEPS,EMS����ϵͳ��VIN
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_readVIN_function( void* pIn, void* pOut )
{
	byte bVIN[2][17]={0};
	byte cAcativeType = 0;
	byte bScanVINsys[] = { BCM_SYS, EMS_SYS};
	int iStatus = false;
	int i = 0;
	int iDataLen = 0;

	//���Ϊ�����¿��ٿռ�
	if( g_p_stVCI_params_config == NULL )
		g_p_stVCI_params_config = ( STRUCT_VCI_PARAMS * )malloc( sizeof( STRUCT_VCI_PARAMS ) );
	if ( g_p_stGeneralActiveEcuConfig == NULL)
		g_p_stGeneralActiveEcuConfig = (STRUCT_ACTIVE_ECU_CONFIG *)malloc( sizeof(STRUCT_ACTIVE_ECU_CONFIG) );

	for ( i = 0; i < 2; i++ )
	{
		initialization_Car_system( bScanVINsys[i] );
		iStatus = function_setting_VDI();//����VDI
		if(iStatus)
		{
			iStatus = function_Active_CAN();//���÷���30֡�����ͽ�������
			if (iStatus)
			{
				iStatus = function_readVIN( bVIN[i] );
			}
		}

		if (!iStatus)
			break;
	}

	if (iStatus)
	{
		if ( !memcmp( bVIN[0], bVIN[1], 17) )
		{
			iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
			add_data_and_controlsID( iDataLen, bVIN[0], 17, ORIGINAL, "ID_TEXT_BAICVIN", pOut );
		}
		else
			iStatus = false;
	}

	if (!iStatus)
	{
		general_return_status( FAIL, 0, 0, pOut );
	}

	free(g_p_stVCI_params_config);
	free(g_p_stGeneralActiveEcuConfig);
}

/*************************************************
Description:	����й������ϵͳ�Ĺ�����
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_Car_ClearDTC( void* pIn, byte* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte bDTCSys[100] = {0};
	byte bDTCSysNumber = 0;
	byte bStatus = 0;
	byte bClearDtcFail[30] = {0};
	byte p_system_location[30] = {0};
	int iDataLen = 0;
	int i = 0, j = 0;

	uint32 uTemp = 0;

	if( g_p_stVCI_params_config == NULL )
		g_p_stVCI_params_config = ( STRUCT_VCI_PARAMS * )malloc( sizeof( STRUCT_VCI_PARAMS ) );

	//������д��ڹ������ϵͳ
	for ( i = 0; i < pstParam->iLen;)
	{
		if ( ',' != pstParam->pcData[i] )
		{
			p_system_location[bDTCSysNumber] = i;
			bDTCSysNumber++;
			switch (pstParam->pcData[i])
			{
			case '0':
				bDTCSys[bDTCSysNumber-1] = EMS_SYS;
				break;
			case '1':
				bDTCSys[bDTCSysNumber-1] = PEPS_SYS;
				break;
			case '2':
				bDTCSys[bDTCSysNumber-1] = BCM_SYS;
				break;
			case '3':  
				bDTCSys[bDTCSysNumber-1] = ICM_SYS;
				break;
			case '4':
				bDTCSys[bDTCSysNumber-1] = ICE_SYS;
				break;
			case '5':
				bDTCSys[bDTCSysNumber-1] = SDM_SYS;
				break;
			case '6':
				bDTCSys[bDTCSysNumber-1] = ESC_SYS;
				break;
			case '7':
				bDTCSys[bDTCSysNumber-1] = ACC_SYS;
				break;
			case '8':
				bDTCSys[bDTCSysNumber-1] = EPS_HL_SYS;
				break;
			case '9':
				bDTCSys[bDTCSysNumber-1] = EPS_LR_SYS;
				break;
			case 'A':
				bDTCSys[bDTCSysNumber-1] = ESCL_SYS;
				break;
			case 'B':
				bDTCSys[bDTCSysNumber-1] = CVT_SYS;
				break;
			//����ϵͳM50F����Ҫ
// 			case 'C':
// 				bDTCSys[bDTCSysNumber-1] = DVR_SYS;
// 				break;
// 			case 'D':
// 				bDTCSys[bDTCSysNumber-1] = GW_SYS;
// 				break;
// 			case 'E':
// 				bDTCSys[bDTCSysNumber-1] = PAS_SYS;
// 				break;
// 			case 'F':
// 				bDTCSys[bDTCSysNumber-1] = AVM_SYS;
// 				break;
// 			case 'G':
// 				bDTCSys[bDTCSysNumber-1] = APC_SYS;
// 				break;
// 			case 'H':
// 				bDTCSys[bDTCSysNumber-1] = EPS_SYS;
// 				break;
// 			case 'I':
// 				bDTCSys[bDTCSysNumber-1] = EPS_SYS;
// 				break;
// 			case 'J':
// 				bDTCSys[bDTCSysNumber-1] = EPS_SYS;
// 				break;
// 			case 'K':
// 				bDTCSys[bDTCSysNumber-1] = ACC_SYS;
// 				break;
			default:
				return;
				break;
			}
			while( ',' != pstParam->pcData[i] && i < pstParam->iLen )
			{
				i++;
			}
		}
		i++;
	}

	//���ݴ��ڹ������ϵͳ���������룬����¼����ʧ�ܵ�ϵͳ
	for ( i = 0; i < bDTCSysNumber; i++ )
	{
		initialization_Car_system( bDTCSys[i] );
		bStatus = Clear_DTC_start(bDTCSys[i]);
		if ( 1 == bStatus )
		{
			bClearDtcFail[j] = i;
			j++;
		}
		else if ( 2 == bStatus )
		{
			bClearDtcFail[j] = i;
			j++;
		}
	}

	//���������������������ظ���λ��
	if ( 0 == j)
	{
		iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
		iDataLen = add_data_and_controlsID( iDataLen, "END", 3, ORIGINAL, NULL,pOut);
		uTemp = pOut[3] * 16777216 + pOut[4] * 65536 + pOut[5] * 256 + pOut[6] - 2;
		pOut[3] = uTemp >> 24 & 0xFF;
		pOut[4] = uTemp >> 16 & 0xFF;
		pOut[5] = uTemp >> 8 & 0xFF;
		pOut[6] = uTemp & 0xFF;
	}
	else
	{
		iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
		for ( i = 0; i < j; i++ )
		{
			iDataLen = add_data_and_controlsID( iDataLen, &pstParam->pcData[p_system_location[bClearDtcFail[i]]], 1, ORIGINAL, NULL,pOut);
			uTemp = pOut[3] * 16777216 + pOut[4] * 65536 + pOut[5] * 256 + pOut[6] - 2;
			pOut[3] = uTemp >> 24 & 0xFF;
			pOut[4] = uTemp >> 16 & 0xFF;
			pOut[5] = uTemp >> 8 & 0xFF;
			pOut[6] = uTemp & 0xFF;
			iDataLen -= 2;
		}
	}
}

