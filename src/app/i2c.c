
#include "i2c.h"
#include "common.h"
#include "bsp.h"
#include "sys_tick.h"


/*
** 
** ������i2c ��ʼ�ź�
**
**
*/ 
bool i2c_start(void)
{
	SDA_HIGH();
	SCL_HIGH();
	delay(5);
	if(!SDA_READ())
	{
		return FALSE;		//SDA��Ϊ�͵�ƽ������æ,�˳�
	}
	SDA_LOW();
	if(SDA_READ()) 
	{
		return FALSE; 		//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
	}
	SDA_LOW();
	delay(5);
	return TRUE;
}

/*
** 
** ������i2c ֹͣ�ź�
**
**
*/ 
void i2c_stop(void)
{
	SCL_LOW();
	delay(5);
	SDA_LOW();
	delay(5);
	SCL_HIGH();
	SDA_HIGH();
	delay(5);
}

/*
** 
** ������i2c �ȴ�Ӧ��
**
**
*/ 
bool i2c_wait_ack(void) //����Ϊ:=1��ACK,=0��ACK
{
	SCL_LOW();
	delay(5);
	SDA_HIGH();
	delay(5);
	SCL_HIGH();
	delay(5);
	if(SDA_READ())
	{
		SCL_LOW();
		return FALSE;
	}
	SCL_LOW();
	return TRUE;
}

/*
** 
** ������i2c ����Ӧ��λ
**
**
*/ 
void i2c_ack(void)
{
	SCL_LOW();
	SDA_LOW();
	delay(5);
	SCL_HIGH();
	delay(5);
	SCL_LOW();
	delay(5);
}

/*
** 
** ������i2c ��Ӧ��
**
**
*/ 
void i2c_no_ack(void)
{ 
	SCL_LOW();
	SDA_HIGH();
	delay(5);
	SCL_HIGH();
	delay(5);
	SCL_LOW();
	delay(5);
}

/*
** 
** ������i2c ����һ���ֽ�
**
**
*/ 
void i2c_send_byte(uint8_t byte) //���ݴӸ�λ����λ//
{
	uint8_t i = 8;
	while (i--)
	{
		SCL_LOW();
		delay(5);
		if (byte&0x80)
		{
			SDA_HIGH();
		}
		else 
		{
			SDA_LOW();
		}
		byte<<=1;
		SCL_HIGH();
		delay(5);
	}
	
	SCL_LOW();
}


/*
** 
** ������i2c ����һ���ֽ�
**
**
*/ 
uint8_t i2c_rece_byte(void)    //���ݴӸ�λ����λ//
{ 
	uint8_t i=8;
	uint8_t byte=0;

	SDA_HIGH();
	while (i--)
	{
		byte <<= 1;
		SCL_LOW();
		delay(5);
		SCL_HIGH();
		if (SDA_READ())
		{
			byte |= 0x01;
		}
	}
	SCL_LOW();
	return byte;
}


/////////////////////////////////////////////////////////////////////////

/*
** ��������ee_byte_write
** ������дһ���ֽڵ�EEPROM��ĳ����ַ
** ������
	dev_addr���豸��ַ
	ee_addr���豸EEPROM��ַ
	ch��Ҫд�������
**
*/ 
bool ee_byte_write(uint8_t dev_addr, uint16_t ee_addr, uint8_t ch)
{
	uint8_t addr = 0;
	
	addr = dev_addr<<1;
	
	if (!i2c_start()) 
		return FALSE;
	
	i2c_send_byte(addr);		//����������ַ+�ε�ַ 
	if (!i2c_wait_ack())
	{
		i2c_stop(); 
		return FALSE;
	}
	
	i2c_send_byte(ee_addr>>8);	//���ö��ڵ�ַ
	i2c_wait_ack();
	
	i2c_send_byte(ee_addr&0xFF);	//���ö��ڵ�ַ
	i2c_wait_ack();
	
	i2c_send_byte(ch);
	i2c_wait_ack();
	
	i2c_stop();
	
	delay(50000);//delay_1ms(5);
	return TRUE;
}

/*
** ��������ee_random_read
** ����: ��EEPROMĳ����ַ��һ���ֽڵ�����
** ������
	dev_addr���豸��ַ
	ee_addr���豸EEPROM��ַ
	ch��Ҫд�������
**
*/ 
uint8_t ee_random_read(uint8_t dev_addr, uint16_t ee_addr)
{
	uint8_t ch = 0;
	uint8_t addr_write = 0, addr_read = 0;
		
	addr_write = dev_addr<<1;
	addr_read = (dev_addr<<1) + 1;
	
	if (!i2c_start()) 
		return FALSE;
	
	i2c_send_byte(addr_write);		//����������ַ+�ε�ַ 
	if (!i2c_wait_ack())
	{
		i2c_stop(); 
		return FALSE;
	}

	i2c_send_byte(ee_addr>>8);  //���øߵ�ַ
	i2c_wait_ack();
	i2c_send_byte(ee_addr&0xFF);  //���õ͵�ַ
	i2c_wait_ack();
	
	i2c_start();
	i2c_send_byte(addr_read);
	i2c_wait_ack();
		
	ch = i2c_rece_byte();
	i2c_no_ack();
				
	i2c_stop();
	
	return ch;	
}


/*
** ��������ee_random_read
** ����: ��EEPROMĳ����ַ��ʼд��len�����ȵ�����data
** ������
	dev_addr���豸��ַ
	ee_addr���豸EEPROM��ַ
	data��Ҫд������
	len��д�����ݵĳ���
**
*/ 
bool ee_page_write(uint8_t dev_addr, uint16_t ee_addr, uint8_t *data, uint16_t len)
{
	uint8_t addr = 0;
	
	addr = dev_addr<<1;
	
	if (!i2c_start()) 
		return FALSE;
	
	i2c_send_byte(addr);		//����������ַ+�ε�ַ 
	if (!i2c_wait_ack())
	{
		i2c_stop(); 
		return FALSE;
	}
	
	i2c_send_byte(ee_addr>>8);	//���ö��ڵ�ַ
	i2c_wait_ack();
	
	i2c_send_byte(ee_addr&0xFF);	//���ö��ڵ�ַ
	i2c_wait_ack();
	
	while(len--)
	{
		i2c_send_byte(*data);
		i2c_wait_ack();
		data++;
	}
	i2c_stop();
	
	delay(100000);//delay_1ms(20);
	return TRUE;
}

/*
** ��������ee_sequential_read
** ����: ��EEPROMĳ����ַ��ʼ��ȡlen���ȵ�����
** ������
	dev_addr���豸��ַ
	ee_addr���豸EEPROM��ַ
	data������������
	len��Ҫ��ȡ�����ݳ���
**
*/ 
bool ee_sequential_read(uint8_t dev_addr, uint16_t ee_addr, uint8_t *data, uint16_t len)
{	
	uint8_t addr_write = 0, addr_read = 0;
		
	addr_write = dev_addr<<1;
	addr_read = (dev_addr<<1) + 1;
	
	if (!i2c_start()) 
		return FALSE;
	
	i2c_send_byte(addr_write);		//����������ַ+�ε�ַ 
	if (!i2c_wait_ack())
	{
		i2c_stop(); 
		return FALSE;
	}

	i2c_send_byte(ee_addr>>8);  //���øߵ�ַ
	i2c_wait_ack();
	i2c_send_byte(ee_addr&0xFF);  //���õ͵�ַ
	i2c_wait_ack();
	
	i2c_start();
	i2c_send_byte(addr_read);
	i2c_wait_ack();
	
	while(len)
	{
		*data = i2c_rece_byte();
		if(len == 1)
		{		
			i2c_no_ack();
		}
		else
		{
			i2c_ack();
		}
		data++;
		len--;
	}
	i2c_stop();
	
	return TRUE;
}


