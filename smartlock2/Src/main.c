
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include<string.h>
#include "OLED.h"
#include "keyboard.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define MAIN_MENU 0
#define INSERT_MENU 1
#define TIPS_MENU 2

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

char rxdata[1];                     //接收当个数据
char mydata[64];                   //保存接收连续数据
int year=19,month=7,day=14,hour=10,min=17,sec=0;
char code[16];
char mypassword[6]="123456";         //原密码
char password[6];                    //保存登陆的密码
char cmd[64];                        //保存命令
int pos = 0;       
int flags=0;        //标志位
uint16_t ret=0;
char tipsbuf[16];                         
int keyflag=0;              
char keyvalue;             //保存输入的按键值
char admin[5]="admin";       //系统默认管理身份
char code[16];               // 保存输出信息
char tipsbuf[16];            //
char datebuf[16];            //保存日期
char timebuf[16];            //保存时间
char datetimebuf[16];  

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)       //按键输入
{
	static int pre_ticks=0;

	int ticks=HAL_GetTick();
  if(ticks-pre_ticks <300 ){
        return;
    }
    pre_ticks = ticks;
    
		keyvalue = Get_KeyNum(GPIO_Pin);
    if(keyvalue >='0'&& keyvalue <='9'){
			  OLED_CLS();
        flags =INSERT_MENU;
        code[pos] = '*';
        password[pos] = keyvalue;
        pos++;
    }
		
    if(keyvalue=='E'){
			  OLED_CLS();
			  if(strlen(password) != 6 )
				{
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
					flags=TIPS_MENU;
					strcpy(tipsbuf,"6 numbers Required");
					memset(code,0,sizeof(code));
          memset(password,0,sizeof(password));
          pos = 0;
				}else{
					ret = strncmp(mypassword,password,6);
          if(ret==0){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
						
						HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
            flags=TIPS_MENU;
            strcpy(tipsbuf,"lock success");
						memset(code,0,sizeof(code));
            memset(password,0,sizeof(password));
            pos = 0;
          }else {
						HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
					  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
            flags = TIPS_MENU;
            strcpy(tipsbuf,"lock failed");
					  memset(code,0,sizeof(code));
            memset(password,0,sizeof(password));
            pos = 0;
        }
			}
    }
		
    if(keyvalue == 'Q'){
			  OLED_CLS();
			  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
			  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
        flags = MAIN_MENU;
			  memset(code,0,sizeof(code));
        memset(password,0,sizeof(password));
       pos = 0;
   }
}

void reset(void)     //清空接收数据的数组
{
	memset(mydata,0,sizeof(mydata));
	pos=0;	
}

void rightlight(void)   //设置绿灯亮
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
}

void  wronglight(void)   //设置红灯亮
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
}

char buf[16];
void SetDateTime(char *ptchar,int i)    //设置时间
{	
	int temp=0;
	char datetime[20];
	int myresult[6];
	memset(datetime,0,sizeof(datetime));
  ptchar=strstr(mydata,"datetime=");
	ptchar=ptchar+strlen("datetime=");
	while(*ptchar !=',' && *ptchar!=';')
  {
			datetime[i]=*ptchar;
			ptchar++;
			i++;
	}
	int m= 0;
	for (int j = 0; j < strlen(datetime); j++)
	{
		if (datetime[j] != '-')
		{
			temp = temp * 10 + datetime[j] - '0';
		}else
		{
			myresult[m] = temp;
			temp = 0;
			m++;
		}
	}
	reset();  
}

char insertpass[16];                  //保存新输入的密码
char resetpass[6];                    //保存新设置的密码
char config[5];                       //保存配置用户名
void SetPassWord(char *ptchar,int i)   //设置密码
{
	    char config[5];
	    flags = TIPS_MENU;
			memset(config,0,sizeof(config));
			ptchar=strstr(mydata,"id=");
		  ptchar=ptchar+strlen("id=");
		  while(*ptchar !=',' && *ptchar!=';')
		  {
			  config[i]=*ptchar;
			  ptchar++;
			  i++;
		  }
			if(strncmp(config,admin,5)==0)  //如果用户名正确，就能设置密码
			{
				i=0;
			  memset(insertpass,0,sizeof(insertpass));
		    ptchar=strstr(mydata,"password=");
		    ptchar=ptchar+strlen("pwssword=");
		    while(*ptchar !=',' && *ptchar!=';')
		    {
			     insertpass[i]=*ptchar;
			     ptchar++;
			     i++;
		    } 
			  if(strlen(insertpass)==6)
			  {
				  i=0;
				   memset(mypassword,0,sizeof(mypassword));
				   strcpy(mypassword,insertpass);
				   strcpy(tipsbuf,"pswd has set!");
				   reset();
           rightlight();				 
			   }else{
				   strcpy(tipsbuf,"6 num required!");
				   reset();	
				   wronglight();
			   }
	     }else{
		     strcpy(tipsbuf,"id wrong!");
	       memset(mydata,0,sizeof(mydata));
		     pos=0;
         wronglight();
	     }	
}

void Unlock(char *ptchar,int i)   //解锁
{
	 flags = TIPS_MENU;
	 memset(insertpass,0,sizeof(insertpass));
	 ptchar=strstr(mydata,"keypass=");
	 ptchar=ptchar+strlen("keypass=");
	 while(*ptchar !=',' && *ptchar!=';')
		{
			 insertpass[i]=*ptchar;
			 ptchar++;
			 i++;
		}
		if(strncmp(insertpass,mypassword,6)==0)
		{
				rightlight();
			  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
				strcpy(tipsbuf,"lock success");
		}else{
				strcpy(tipsbuf,"lock failed");
				wronglight();
		}
		reset();
}

void RestPassWord(char *ptchar,int i)   //重设密码
{
	/*先验证密码是否正确*/
	flags = TIPS_MENU;
	memset(insertpass,0,sizeof(insertpass));
	ptchar=strstr(mydata,"keypass=");
	ptchar=ptchar+strlen("keypass=");
	while(*ptchar !=',' && *ptchar!=';')
	{
			insertpass[i]=*ptchar;
			ptchar++;
			i++;
	}
	if(strncmp(insertpass,mypassword,strlen(mypassword))==0)
	{
			i=0;
			memset(resetpass,0,sizeof(resetpass));
		  ptchar=strstr(mydata,"newpass=");
		  ptchar=ptchar+strlen("newpass=");
		  while(*ptchar !=',' && *ptchar!=';')
		  {
			   resetpass[i]=*ptchar;
			   ptchar++;
			   i++;
		  }
			if(strlen(resetpass)==6)
			{
				 memset(mypassword,0,sizeof(mypassword));
				 strcpy(mypassword,resetpass);
				 HAL_UART_Transmit(&huart4,mypassword,strlen(mypassword),100);
				 strcpy(tipsbuf,"pswd has changed!");	
				 rightlight();
			}else{
				 wronglight();
				 strcpy(tipsbuf,"6 num required!");		
			}
	}else{
			wronglight();
			strcpy(tipsbuf,"wrong passwd!");
		  } 
  reset();			 
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)    //串口中断接收数据
{
	
	int i=0;
	char *ptchar;
	mydata[pos]=rxdata[0];
	pos++;
	if(rxdata[0]==';')
	{
		  
		/*设置时间*/
		if(strstr(mydata,"setdatetime")!=NULL)
		{
			//memset(time,0,sizeof(time));
			 SetDateTime(ptchar,i);
		}
		/*设置初始密码*/
		 else if(strstr(mydata,"setpassword")!=NULL)   
		 { 
			 SetPassWord(ptchar,i);
		 }else if(strstr(mydata,"resetpass")!=NULL )   //修改密码
		  {
			  RestPassWord(ptchar,i);
	    }else if(strstr(mydata,"unlock")!=NULL)  //解锁
		   {
			  Unlock(ptchar,i);
			 }else if(strstr(mydata,"lock")!=NULL)
		    {
			    wronglight();
			    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
			    reset();	
		    }
   }
	HAL_UART_Receive_IT(&huart4,rxdata,1);
}

void main_menu_flash(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
	HAL_Delay(1000);
	sec++;
    if(sec==60){
        sec=0;
			   min++;
		}			
     if(min==60){  
        min=0;
        hour++;			 
		 }
     if(hour==12)
		 {
			 hour=0;
		 }
	sprintf(datebuf,"%d-%d-%d",year,month,day);
	sprintf(timebuf,"%d:%d:%d",hour,min,sec);
	OLED_ShowStr(0,0,(uint8_t *)datebuf,CODE8X16);
	OLED_ShowStr(0,16,(uint8_t *)timebuf,CODE8X16);
	OLED_ShowStr(0,32,(uint8_t *)"smart lock!",CODE8X16);
	fflash();
}

void insert_menu_flash(void)
{ 
	OLED_ShowStr(0,0,(uint8_t *)"please ins psd:",CODE8X16);
	OLED_ShowStr(0,16,(uint8_t *)code,CODE8X16);
	fflash();
}

void tips_menu_flah(void)
{
	
	OLED_CLS();
	OLED_ShowStr(0,0,(uint8_t *)tipsbuf,CODE8X16);
	//HAL_Delay(2*1000);
	fflash();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart4,rxdata,1);
	
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
	
  OLED_Init();
	OLED_CLS_FLASH();
	OLED_CLS();
	Key_Bord_Init();
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
	//HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  switch(flags)
	{
		case MAIN_MENU:
		     main_menu_flash();break;
		case INSERT_MENU:
		     insert_menu_flash();break;
		case TIPS_MENU:
			   tips_menu_flah();
		     HAL_Delay(3*1000);
		     OLED_CLS();
         flags = MAIN_MENU;
         break;
		default:main_menu_flash();break;
	}
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* UART4 init function */
static void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
