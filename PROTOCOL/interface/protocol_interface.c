/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	������ӿں���
History:
	<author>	<time>		<desc>

************************************************************************/

#define PROTOCOL_DLL_EXPORTS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*#define NDEBUG*/
#include <assert.h>

#include "protocol_interface.h"

#include "../function/dtc_lib.h"
#include "../function/clear_dtc_lib.h"
#include "../function/ds_lib.h"
#include "../function/infor_lib.h"
#include "../function/active_ecu_lib.h"
#include "../function/idle_link_lib.h"
#include "../function/freeze_lib.h"
#include "../function/actuator_test_lib.h"
#include "../function/scan_ecu_version.h"
#include "../SpecialFunction/special_function.h"
#include "../function/quit_system_lib.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
#include "../public/public.h"
#include "../RapidDiagnosis/RapidDiagnosis.h"



//ȫ�ֱ����������˳���ѭ��
bool g_bCancelWaitDlg = true;
/*************************************************
Description:	ע�ᷢ������ص�����
Input:
	receive_cmd_callback	���������ָ�룬��Ӧ�ó�����
Return:	void
Others:
*************************************************/
void regist_send_cmd_callback( SEND_CMD_CALLBACK send_cmd_callback )
{
	send_cmd = send_cmd_callback;
}

/*************************************************
Description:	ע���������ص�����
Input:
	receive_cmd_callback	�������������Ӧ�ó�����
Return:	void
Others:
*************************************************/
void regist_receive_cmd_callback( RECEIVE_CMD_CALLBACK receive_cmd_callback )
{
	receive_cmd = receive_cmd_callback;
}

/*************************************************
Description:	ע��������д�֡ͷ����ص�����
Input:
receive_cmd_callback	�������������Ӧ�ó�����
Return:	void
Others:
*************************************************/
void regist_receive_all_cmd_callback( RECEIVE_ALL_CMD_CALLBACK receive_all_cmd_callback )
{
	receive_all_cmd = receive_all_cmd_callback;
}

/*************************************************
Description:	ע����ʱ�ص�����
Input:
	time_delay_callback		�������������Ӧ�ó�����
Return:	void
Others:
*************************************************/
void regist_time_delay( TIME_DELAY time_delay_callback )
{
	time_delay_ms = time_delay_callback;
}

/*************************************************
Description:	����VCI
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:	���Է���������ʧ�����FAIL
*************************************************/
int setting_vci( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte *pcOutTemp = ( byte * )pOut;

	int iVciConfigOffset = 0;
	int iProtocolConfigOffset = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iVciConfigOffset = atoi( pstParam->pcData );//��ȡVCI����ƫ��
	g_p_stVCI_params_config = g_p_stVciParamsGroup[iVciConfigOffset];

	g_CANoffset = g_p_stVCI_params_config->cCANFrameMode * 2;

	pstParam = pstParam->pNextNode;//��ȡVCI����ģ���

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	pstParam = pstParam->pNextNode;//��ȡЭ������ƫ��

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iProtocolConfigOffset = atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode;//��ȡЭ������ƫ��

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	g_stInitXmlGobalVariable.m_cProtocolConfigType = ( byte )atoi( pstParam->pcData ); //��ȡЭ������ģ���(����)

	//ѡ��Э�����ã�������ӦЭ��ѡ����Ӧ����
	select_protocol_config( iProtocolConfigOffset, g_stInitXmlGobalVariable.m_cProtocolConfigType );

	if( package_and_send_vci_config() ) //������ճɹ������óɹ��򷵻�SUCCESS
	{
		general_return_status( SUCCESS, NULL, 0, pcOutTemp );
		pcOutTemp[2] = 0;//����ʾ
	}
	else
	{
		special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pcOutTemp );
	}

	return 0;
}
/*************************************************
Description:	��XML��ȡ���ݣ�ֱ�ӷ��͸�VDI
Input:	pIn		�������
Output:	pOut	
Return:	����
Others:
*************************************************/
int XML_to_VDI( void* pIn, void* pOut )
{
	byte FunctionSlect = 0;
	bool bReturnStatus = false;
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte pcCmd[255] = { 0 };
	byte cLen = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	
	cLen = get_string_type_data_to_byte( pcCmd, pstParam->pcData, pstParam->iLen );
	
	pcCmd[cLen - 1] = calculate_Checksum( pcCmd, cLen - 1 );
	pcCmd[2] = cLen;

	send_cmd( pcCmd, cLen );
	
	bReturnStatus = receive_confirm_byte( 3000 );
	if ( !bReturnStatus )
	{
		special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pOut );
	}
	else
	{
		general_return_status( SUCCESS, NULL, 0, pOut );
		((byte *)pOut)[2] = 0;//����ʾ
	}

	return bReturnStatus;
}
/*************************************************
Description:	����ECU
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int active_ECU( void* pIn, void* pOut )
{
	process_active_ECU( pIn, pOut );

	return 0;
}

//ɨ��ECU�汾
void process_ECU_version(void* pIn, void* pOut)
{
	process_ECU_version_function( pIn, pOut );
}

/*************************************************
Description:	��ȡ��ǰ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_current_Dtc( void* pIn, void* pOut )
{
	process_read_current_Dtc( pIn, pOut );

	return 0;
}

/*************************************************
Description:	��ȡ��ʷ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_history_Dtc( void* pIn, void* pOut )
{
	process_read_history_Dtc( pIn, pOut );

	return 0;
}


/*************************************************
Description:	���������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int clear_Dtc( void* pIn, void* pOut )
{

	process_clear_Dtc( pIn, pOut );

	return 0;
}


/*************************************************
Description:	������֡������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_freeze_frame_DTC( void* pIn, void* pOut )
{
	process_read_freeze_frame_DTC( pIn, pOut );

	return 0;
}


/*************************************************
Description:	������֡������
Input:	pIn		�б���DTC��ID
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_freeze_frame_DS( void* pIn, void* pOut )
{
	process_read_freeze_frame_DS( pIn, pOut );

	return 0;
}

/*************************************************
Description:	��������
Input:	pIn		DS��ID
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_data_stream( void* pIn, void* pOut )
{

	process_read_data_stream( pIn, pOut );

	return 0;
}

/*************************************************
Description:	�����������
Input:	pIn		DS��ID
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int enter_read_data_stream( void* pIn, void* pOut )
{


	return 0;
}
/*************************************************
Description:	�˳���������
Input:	pIn		DS��ID
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int quit_read_data_stream( void* pIn, void* pOut )
{


	return 0;
}

/*************************************************
Description:	���汾��Ϣ
Input:	pIn		�汾��Ϣ���ID
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_Ecu_information( void* pIn, void* pOut )
{
	process_read_ECU_information( pIn, pOut );

	return 0;
}

/*************************************************
Description:	ִ��������
Input:	pIn		�������ID���������͵ĸ�������ֵ��
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int actuator_test( void* pIn, void* pOut )
{
	process_actuator_test( pIn, pOut );

	return 0;
}

/*************************************************
Description:	���⹦��
Input:	pIn		��ִ�й����йص���������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int special_function( void* pIn, void* pOut )
{

	process_special_function( pIn, pOut );

	return 0;
}

/*************************************************
Description:	ɨ�賵��VIN
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int read_VIN( void* pIn, void* pOut )
{
	process_readVIN_function( pIn, pOut );

	return 0;
}

/*************************************************
Description:	���������
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:	
*************************************************/
int Car_CLDTC( void* pIn, void* pOut )
{
	process_Car_ClearDTC( pIn, pOut );

	return 0;
}

/*************************************************
Description:	��ȡ��������
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:	
*************************************************/
int read_Car_CFG( void* pIn, void* pOut )
{
	return SUCCESS;
}

/*************************************************
Description:	�˳�ϵͳ
Input:	pIn		����
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int quit_system( void* pIn, void* pOut )
{

	process_quit_system( pIn, pOut );

	return 0;
}
/*************************************************
Description:	��xml��ȡϵͳ����
Input:
		iConfigType		��������
		PIn				������������
Output:	����
Return:	����
Others:
*************************************************/

int init_config_from_xml( int iConfigType, void* pIn )
{
	process_init_config_from_xml( iConfigType, pIn );

	return 0;
}



STRUCT_SELECT_FUN stSetConfigFunGroup[] =
{
	{SET_CONFIG_FC_CAN, process_SET_CONFIG_FC_CMD_CAN},
	{SET_ECU_REPLAY, process_SET_ECU_REPLY},
	{SET_SEND_AND_RECEIVE, get_accord_ecu_cmdnum_send_cmdconfig_data },
	{SET_MUL_DS_MODE, process_ds_mul_mode},
	{SET_MUL_FRAME_OFFSET, set_mul_frame_offset},

};


/*************************************************
Description:	��ȡ�������
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_set_config_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stSetConfigFunGroup ) / sizeof( stSetConfigFunGroup[0] ); i++ )
		if( cType == stSetConfigFunGroup[i].cType )
			return stSetConfigFunGroup[i].pFun;

	return 0;
}

/*************************************************
Description:	 ������������
Input:	PIn		����
Output:	����
Return:	����
Others:
*************************************************/
int special_config_function( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	pf_general_function pFun = NULL;
	byte cConfigType = 0;
	int iConfigOffset = 0;

	do
	{
		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );
		iConfigOffset = atoi( pstParam->pcData ); //��ô���������ID

		pstParam = pstParam->pNextNode;
		if (pstParam == NULL)
		{
			break;
		}
		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );
		cConfigType = ( byte )atoi( pstParam->pcData ); //��ô���������ģ���

		u32Config_fuc = u32Config_fuc_Group[iConfigOffset];


		pFun = get_set_config_fun( cConfigType );

		assert( pFun );

		pFun( pstParam, pOut );

		if( 1 != *( byte * )pOut )
		{
			return 0;
		}
		if (pstParam != NULL)
		{
			pstParam = pstParam->pNextNode;
		}
	}
	while( pstParam != NULL );

	return 0;
}



/*************************************************
Description:	 �ȴ����мӰ�ť���˺��������˳���ѭ��
Input:	PIn		����
Output:	����
Return:	����
Others:
*************************************************/
void cancelWaitDlg(void* pIn, void* pOut)
{
	g_bCancelWaitDlg = false;
}

/*************************************************
Description:	��ȡ�����루���ַ�ʽ��ͨ������1901������1902��
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:	���Է���������ʧ�����FAIL
*************************************************/
int read_dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte *pcOutTemp = ( byte * )pOut;
	int iActiveCmdSum = 0;
	byte cBufferOffset = 0;//����ƫ��
	byte DTCNumbers[4]= {0};
	byte mask_all[5] = {0};
	
	byte i = 0;
	int DTCType = 0;
	int DTCNumberStartOffset = 0;
	int DTCNumberBytes = 0;
	int DTCStartOffset = 0;
	int DTCBytesInCmd = 0;
	int DTCStatusoffset = 0;
	int DTCBytesdisplay = 0;
	int DTCSupportmask = 0;
	byte bSystemidentifier = 0;

	bool bProcessSingleCmdStatus = false;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCType = atoi( pstParam->pcData );//��ȡ����������

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCNumberStartOffset = atoi( pstParam->pcData );//�����������ƫ��

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCNumberBytes = atoi( pstParam->pcData );//������Ÿ����ɼ����ֽ����

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCStartOffset = atoi( pstParam->pcData );//��SID��ʼ�����������ʼƫ��

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCBytesInCmd = atoi( pstParam->pcData );//�����ֽڱ�ʾһ��������

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCStatusoffset = atoi( pstParam->pcData );//������״̬��һ�������������е�ƫ��

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCBytesdisplay = atoi( pstParam->pcData );//һ����������Ҫ��ʾ�����ֽ�

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//ѡ����������룬����XML���ð�ʮ���ƻ���16���ƽ����ж�ѡ��
	for (i = 0;i< pstParam->iLen;i++)
	{
		mask_all[i] = pstParam->pcData[i];
	}
	if (mask_all[0] == '0' && mask_all[1] == 'x' || mask_all[1] == 'X' )
	{
		i = 2;
		for (;(mask_all[i]>= '0' && mask_all[i] <= '9') || (mask_all[i] >= 'a' && mask_all[i] <= 'z') || (mask_all[i] >= 'A' && mask_all[i] <= 'Z');++i)
		{
			if (big_to_small(mask_all[i]) > '9')
			{
				DTCSupportmask = 16 * DTCSupportmask +(10 + big_to_small(mask_all[i]) - 'a');
			}
			else
			{
				DTCSupportmask = 16 *DTCSupportmask +(big_to_small(mask_all[i]) - '0');
			}
		}
		
	}
	else
	{
		DTCSupportmask =  atoi(pstParam->pcData) ;//��ǰϵͳ����������,ʹ�������ǲ�������Ϊʮ����
	}

	pstParam = pstParam->pNextNode;//���뼰��ȡϵͳ��Ӧ�̱�ʾ������ CMD1901��CMD1902��CMD22F187

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	iActiveCmdSum = get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stCmdList ) );
	g_stInitXmlGobalVariable.m_iSpecificCmdGetFromXmlSum = iActiveCmdSum;

	if( 0 == iActiveCmdSum )
	{
		general_return_status( SUCCESS, NULL, 0, pcOutTemp );
		pcOutTemp[2] = 0;//����ʾ

		free_specific_command_config_space();

		return 0;
	}

	pstParam = pstParam->pNextNode;//��ȡϵͳ��ʾ�� 8 ����Ҫ�ж�ϵͳ

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	bSystemidentifier = pstParam->pcData[0] -0x30;
	if(bSystemidentifier != 8)//�ж�ϵͳ
	{
		//��ȡϵͳ��ʾ��
		cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ 2 ].cBufferOffset;
		bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( 2, pOut );
		if( !bProcessSingleCmdStatus )
		{
			(( byte* )pOut)[0] = 2;  //ʧ��,ͳһ��ʾ��ʱ����0x7F����ʾfail�������ڴ�ͳһ�޸�Ϊtimeout��
			return false;
		}

		switch(bSystemidentifier)
		{
		case 0:
			bProcessSingleCmdStatus = memcmp(g_stBufferGroup[cBufferOffset].cBuffer + 3, "A010C00072", 10);//��¡
			break;
		case 1:
			bProcessSingleCmdStatus = memcmp(g_stBufferGroup[cBufferOffset].cBuffer + 3, "A023C01672", 10);//����
			break;
		default:
			break;
		}
		if (bProcessSingleCmdStatus)
		{
			(( byte* )pOut)[0] = 2;  //ʧ��,ͳһ��ʾ��ʱ����0x7F����ʾfail�������ڴ�ͳһ�޸�Ϊtimeout��
			return false;
		}
	}
	//��ȡ��������Ŀ
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ 0 ].cBufferOffset;

	bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( 0, pOut );

	if( !bProcessSingleCmdStatus )
	{
		return false;
	}

	switch (DTCType)
	{
	case 0:
		memcpy( DTCNumbers, g_stBufferGroup[cBufferOffset].cBuffer + DTCNumberStartOffset, DTCNumberBytes );
//  		iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut);
//  		iDataLen = add_data_and_controlsID( iDataLen, DTCNumbers, DTCNumberBytes,ORIGINAL, NULL, pOut );
		break;
	case 1:
		DTCNumbers[0] = (g_stBufferGroup[cBufferOffset].iValidLen - DTCStartOffset)/DTCBytesInCmd;
//  		iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut);
//  		iDataLen = add_data_and_controlsID( iDataLen, DTCNumbers, 1,ORIGINAL, NULL, pOut );
		break;
	default:
		break;
	}

	//��ȡ������
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ 1 ].cBufferOffset;

	bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( 1, pOut );
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;

	if( !bProcessSingleCmdStatus )
	{
		return false;
	}
		process_self_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen,DTCStartOffset ,DTCBytesInCmd,
								DTCStatusoffset, DTCBytesdisplay, DTCSupportmask ,pOut);
	

	//ֹͣ���Ϳ���
	set_idle_link(0);	

	return 0;
}
/*************************************************
Description:	�������������
Input:
pcDctData	������ظ�����洢��ַ
iValidLen	��Ч����
cDtcMask	������״̬

Output:	pstDtc	�������ָ��
Return:	int		���������
Others:
*************************************************/
int process_self_Dtc_data( byte* pcDctData, int iValidLen,int cDtcStartOffset,int cDtcBytesInCmd ,
						  int cStatusOffset,int cDtcBytesInDisplay, byte cDtcMask, void* pOut)
{
	int i = 0;
	byte iDtcNum[4] ={ 0 };
	int iDataLen;
	byte *pcDtcStart = NULL;//��������ʼ��ŵ�ַ
	byte cDtcStatusCache[256] = {0};
	//byte cDtcStartOffset = 0, cDtcBytesInCmd = 0, cStatusOffset = 0, cDtcBytesInDisplay = 0;

	
	if (iValidLen >= cDtcStartOffset)//��Ч���ݱ�ƫ�ƴ�
	{
		iDtcNum[0] = ( iValidLen -  cDtcStartOffset ) / cDtcBytesInCmd;
	}
	else
	{
		iDtcNum[0] = 0;
	}
	iDataLen = New_special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	iDataLen = New_add_data_and_controlsID( iDataLen, iDtcNum, 1,ORIGINAL, NULL, pOut );
	iDataLen -= 2;

	pcDtcStart = pcDctData + cDtcStartOffset;

	for( i = 0; i < iDtcNum[0]; i++ )
	{
		//���������״̬���
		get_Dtc_status( pcDtcStart[cStatusOffset + i * cDtcBytesInCmd], cDtcStatusCache, cDtcMask );

		iDataLen = New_add_data_and_controlsID( iDataLen, pcDtcStart + ( i * cDtcBytesInCmd ), cDtcBytesInDisplay, PCBU_PRINT, cDtcStatusCache, pOut  );
	}
	return iDtcNum[0];
}

//����д��ĸתΪСд��ĸ��
int big_to_small(int c)
{
	if (c>= 'A' && c<= 'z')
	{
		return c + 'a' - 'A';
	}
	else
	{
		return c;
	}
}

