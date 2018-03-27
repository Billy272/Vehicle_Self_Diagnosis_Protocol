/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	定义各接口函数
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



//全局变量，用于退出死循环
bool g_bCancelWaitDlg = true;
/*************************************************
Description:	注册发送命令回调函数
Input:
	receive_cmd_callback	发送命令函数指针，由应用程序定义
Return:	void
Others:
*************************************************/
void regist_send_cmd_callback( SEND_CMD_CALLBACK send_cmd_callback )
{
	send_cmd = send_cmd_callback;
}

/*************************************************
Description:	注册接收命令回调函数
Input:
	receive_cmd_callback	接收命令函数，由应用程序定义
Return:	void
Others:
*************************************************/
void regist_receive_cmd_callback( RECEIVE_CMD_CALLBACK receive_cmd_callback )
{
	receive_cmd = receive_cmd_callback;
}

/*************************************************
Description:	注册接收所有带帧头命令回调函数
Input:
receive_cmd_callback	接收命令函数，由应用程序定义
Return:	void
Others:
*************************************************/
void regist_receive_all_cmd_callback( RECEIVE_ALL_CMD_CALLBACK receive_all_cmd_callback )
{
	receive_all_cmd = receive_all_cmd_callback;
}

/*************************************************
Description:	注册延时回调函数
Input:
	time_delay_callback		接收命令函数，由应用程序定义
Return:	void
Others:
*************************************************/
void regist_time_delay( TIME_DELAY time_delay_callback )
{
	time_delay_ms = time_delay_callback;
}

/*************************************************
Description:	设置VCI
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:	尝试发送三次若失败输出FAIL
*************************************************/
int setting_vci( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte *pcOutTemp = ( byte * )pOut;

	int iVciConfigOffset = 0;
	int iProtocolConfigOffset = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iVciConfigOffset = atoi( pstParam->pcData );//获取VCI配置偏移
	g_p_stVCI_params_config = g_p_stVciParamsGroup[iVciConfigOffset];

	g_CANoffset = g_p_stVCI_params_config->cCANFrameMode * 2;

	pstParam = pstParam->pNextNode;//获取VCI配置模板号

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	pstParam = pstParam->pNextNode;//获取协议配置偏移

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iProtocolConfigOffset = atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode;//获取协议配置偏移

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	g_stInitXmlGobalVariable.m_cProtocolConfigType = ( byte )atoi( pstParam->pcData ); //获取协议配置模板号(类型)

	//选择协议配置，根据相应协议选择相应配置
	select_protocol_config( iProtocolConfigOffset, g_stInitXmlGobalVariable.m_cProtocolConfigType );

	if( package_and_send_vci_config() ) //如果接收成功且设置成功则返回SUCCESS
	{
		general_return_status( SUCCESS, NULL, 0, pcOutTemp );
		pcOutTemp[2] = 0;//不提示
	}
	else
	{
		special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pcOutTemp );
	}

	return 0;
}
/*************************************************
Description:	从XML获取数据，直接发送给VDI
Input:	pIn		输入参数
Output:	pOut	
Return:	保留
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
		((byte *)pOut)[2] = 0;//不提示
	}

	return bReturnStatus;
}
/*************************************************
Description:	激活ECU
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int active_ECU( void* pIn, void* pOut )
{
	process_active_ECU( pIn, pOut );

	return 0;
}

//扫描ECU版本
void process_ECU_version(void* pIn, void* pOut)
{
	process_ECU_version_function( pIn, pOut );
}

/*************************************************
Description:	读取当前故障码
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_current_Dtc( void* pIn, void* pOut )
{
	process_read_current_Dtc( pIn, pOut );

	return 0;
}

/*************************************************
Description:	读取历史故障码
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_history_Dtc( void* pIn, void* pOut )
{
	process_read_history_Dtc( pIn, pOut );

	return 0;
}


/*************************************************
Description:	清除故障码
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int clear_Dtc( void* pIn, void* pOut )
{

	process_clear_Dtc( pIn, pOut );

	return 0;
}


/*************************************************
Description:	读冻结帧故障码
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_freeze_frame_DTC( void* pIn, void* pOut )
{
	process_read_freeze_frame_DTC( pIn, pOut );

	return 0;
}


/*************************************************
Description:	读冻结帧数据流
Input:	pIn		列表中DTC的ID
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_freeze_frame_DS( void* pIn, void* pOut )
{
	process_read_freeze_frame_DS( pIn, pOut );

	return 0;
}

/*************************************************
Description:	读数据流
Input:	pIn		DS的ID
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_data_stream( void* pIn, void* pOut )
{

	process_read_data_stream( pIn, pOut );

	return 0;
}

/*************************************************
Description:	进入读数据流
Input:	pIn		DS的ID
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int enter_read_data_stream( void* pIn, void* pOut )
{


	return 0;
}
/*************************************************
Description:	退出读数据流
Input:	pIn		DS的ID
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int quit_read_data_stream( void* pIn, void* pOut )
{


	return 0;
}

/*************************************************
Description:	读版本信息
Input:	pIn		版本信息组的ID
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_Ecu_information( void* pIn, void* pOut )
{
	process_read_ECU_information( pIn, pOut );

	return 0;
}

/*************************************************
Description:	执行器测试
Input:	pIn		测试项的ID（带输入型的附加输入值）
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int actuator_test( void* pIn, void* pOut )
{
	process_actuator_test( pIn, pOut );

	return 0;
}

/*************************************************
Description:	特殊功能
Input:	pIn		与执行功能有关的命令数据
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int special_function( void* pIn, void* pOut )
{

	process_special_function( pIn, pOut );

	return 0;
}

/*************************************************
Description:	扫描车型VIN
Input:	pIn		输入参数
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int read_VIN( void* pIn, void* pOut )
{
	process_readVIN_function( pIn, pOut );

	return 0;
}

/*************************************************
Description:	清除故障码
Input:	pIn		输入参数
Output:	pOut	输出数据地址
Return:	保留
Others:	
*************************************************/
int Car_CLDTC( void* pIn, void* pOut )
{
	process_Car_ClearDTC( pIn, pOut );

	return 0;
}

/*************************************************
Description:	读取车辆配置
Input:	pIn		输入参数
Output:	pOut	输出数据地址
Return:	保留
Others:	
*************************************************/
int read_Car_CFG( void* pIn, void* pOut )
{
	return SUCCESS;
}

/*************************************************
Description:	退出系统
Input:	pIn		保留
Output:	pOut	输出数据地址
Return:	保留
Others:
*************************************************/
int quit_system( void* pIn, void* pOut )
{

	process_quit_system( pIn, pOut );

	return 0;
}
/*************************************************
Description:	从xml获取系统配置
Input:
		iConfigType		配置类型
		PIn				具体配置内容
Output:	保留
Return:	保留
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
Description:	获取处理激活函数
Input:
	cType		配置类型
Output:	保留
Return:	pf_general_function 函数指针
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
Description:	 设置配置命令
Input:	PIn		保留
Output:	保留
Return:	保留
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
		iConfigOffset = atoi( pstParam->pcData ); //获得处理函数配置ID

		pstParam = pstParam->pNextNode;
		if (pstParam == NULL)
		{
			break;
		}
		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );
		cConfigType = ( byte )atoi( pstParam->pcData ); //获得处理函数配置模板号

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
Description:	 等待框中加按钮，此函数用于退出死循环
Input:	PIn		保留
Output:	保留
Return:	保留
Others:
*************************************************/
void cancelWaitDlg(void* pIn, void* pOut)
{
	g_bCancelWaitDlg = false;
}

/*************************************************
Description:	读取故障码（两种方式：通过命令1901或命令1902）
Input:	pIn		输入参数（保留）
Output:	pOut	输出数据地址
Return:	保留
Others:	尝试发送三次若失败输出FAIL
*************************************************/
int read_dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte *pcOutTemp = ( byte * )pOut;
	int iActiveCmdSum = 0;
	byte cBufferOffset = 0;//缓存偏移
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
	int iReceiveValidLen = 0;//接收到的有效字节长度

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCType = atoi( pstParam->pcData );//获取故障码类型

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCNumberStartOffset = atoi( pstParam->pcData );//故障码个数的偏移

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCNumberBytes = atoi( pstParam->pcData );//故障码号个数由几个字节组成

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCStartOffset = atoi( pstParam->pcData );//从SID开始，故障码的起始偏移

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCBytesInCmd = atoi( pstParam->pcData );//几个字节表示一个故障码

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCStatusoffset = atoi( pstParam->pcData );//故障码状态在一个完整故障码中的偏移

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	DTCBytesdisplay = atoi( pstParam->pcData );//一个故障码需要显示几个字节

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//选择故障码掩码，根据XML配置按十进制还是16进制进行判断选择
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
		DTCSupportmask =  atoi(pstParam->pcData) ;//当前系统故障码掩码,使用条件是参数配置为十进制
	}

	pstParam = pstParam->pNextNode;//读码及读取系统供应商标示符命令 CMD1901，CMD1902，CMD22F187

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//获取激活命令条数、命令内容
	iActiveCmdSum = get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stCmdList ) );
	g_stInitXmlGobalVariable.m_iSpecificCmdGetFromXmlSum = iActiveCmdSum;

	if( 0 == iActiveCmdSum )
	{
		general_return_status( SUCCESS, NULL, 0, pcOutTemp );
		pcOutTemp[2] = 0;//不提示

		free_specific_command_config_space();

		return 0;
	}

	pstParam = pstParam->pNextNode;//读取系统标示符 8 不需要判断系统

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	bSystemidentifier = pstParam->pcData[0] -0x30;
	if(bSystemidentifier != 8)//判断系统
	{
		//读取系统标示符
		cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ 2 ].cBufferOffset;
		bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( 2, pOut );
		if( !bProcessSingleCmdStatus )
		{
			(( byte* )pOut)[0] = 2;  //失败,统一提示超时；（0x7F会提示fail，所以在此统一修改为timeout）
			return false;
		}

		switch(bSystemidentifier)
		{
		case 0:
			bProcessSingleCmdStatus = memcmp(g_stBufferGroup[cBufferOffset].cBuffer + 3, "A010C00072", 10);//恒隆
			break;
		case 1:
			bProcessSingleCmdStatus = memcmp(g_stBufferGroup[cBufferOffset].cBuffer + 3, "A023C01672", 10);//龙润
			break;
		default:
			break;
		}
		if (bProcessSingleCmdStatus)
		{
			(( byte* )pOut)[0] = 2;  //失败,统一提示超时；（0x7F会提示fail，所以在此统一修改为timeout）
			return false;
		}
	}
	//读取故障码数目
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

	//读取故障码
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ 1 ].cBufferOffset;

	bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( 1, pOut );
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;

	if( !bProcessSingleCmdStatus )
	{
		return false;
	}
		process_self_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen,DTCStartOffset ,DTCBytesInCmd,
								DTCStatusoffset, DTCBytesdisplay, DTCSupportmask ,pOut);
	

	//停止发送空闲
	set_idle_link(0);	

	return 0;
}
/*************************************************
Description:	处理故障码数据
Input:
pcDctData	故障码回复命令存储地址
iValidLen	有效长度
cDtcMask	故障码状态

Output:	pstDtc	输出链表指针
Return:	int		故障码个数
Others:
*************************************************/
int process_self_Dtc_data( byte* pcDctData, int iValidLen,int cDtcStartOffset,int cDtcBytesInCmd ,
						  int cStatusOffset,int cDtcBytesInDisplay, byte cDtcMask, void* pOut)
{
	int i = 0;
	byte iDtcNum[4] ={ 0 };
	int iDataLen;
	byte *pcDtcStart = NULL;//故障码起始存放地址
	byte cDtcStatusCache[256] = {0};
	//byte cDtcStartOffset = 0, cDtcBytesInCmd = 0, cStatusOffset = 0, cDtcBytesInDisplay = 0;

	
	if (iValidLen >= cDtcStartOffset)//有效数据比偏移大
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
		//处理故障码状态相关
		get_Dtc_status( pcDtcStart[cStatusOffset + i * cDtcBytesInCmd], cDtcStatusCache, cDtcMask );

		iDataLen = New_add_data_and_controlsID( iDataLen, pcDtcStart + ( i * cDtcBytesInCmd ), cDtcBytesInDisplay, PCBU_PRINT, cDtcStatusCache, pOut  );
	}
	return iDtcNum[0];
}

//将大写字母转为小写字母；
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

