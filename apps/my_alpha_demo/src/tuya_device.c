/*
 * @Author: yj 
 * @email: shiliu.yang@tuya.com
 * @LastEditors: yj 
 * @file name: tuya_device.c
 * @Description: template demo for SDK WiFi & BLE for BK7231T
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-01-23 16:33:00
 * @LastEditTime: 2021-01-27 17:00:00
 */

#define _TUYA_DEVICE_GLOBAL

/* Includes ------------------------------------------------------------------*/
#include "uni_log.h"
#include "tuya_iot_wifi_api.h"
#include "tuya_hal_system.h"
#include "tuya_iot_com_api.h"
#include "tuya_cloud_com_defs.h"
#include "gw_intf.h"
#include "gpio_test.h"
#include "tuya_gpio.h"
#include "tuya_key.h"
#include "tuya_led.h"
#include "wlan_ui_pub.h"

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

#define UDP_LISTENER_PORT 55000

/* Private includes ----------------------------------------------------------*/
#include "tuya_device.h"
#include "new_http.h"
#include "new_pins.h"

#include "../../beken378/func/key/multi_button.h"
#include "lwip/apps/mqtt.h"

static int cnt = 0;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* wifi 相关配置 */
#define WIFI_WORK_MODE_SEL          GWCM_OLD_PROD   //wifi 工作模式选择
#define WIFI_CONNECT_OVERTIME_S     180             //wifi 配网超时时间，单位：s

/* 配网按键相关宏,长按进入配网模式 */
#define WIFI_KEY_PIN                TY_GPIOA_14 //按键引脚 
#define WIFI_KEY_TIMER_MS           100         //轮询扫描按键所需的时间，一般默认为 100ms
#define WIFI_KEY_LONG_PRESS_MS      3000        //按键长按时间设置
#define WIFI_KEY_SEQ_PRESS_MS       400         //按键连按时间设置
#define WIFI_KEY_LOW_LEVEL_ENABLE   TRUE        //TRUE：按键按下为低，FALSE：按键按下为高

/* 配网指示灯相关宏 */
#define WIFI_LED_PIN                TY_GPIOA_26 //LED 引脚 
#define WIFI_LED_LOW_LEVEL_ENABLE   FALSE       //TRUE：低电平点亮 led，FALSE：高电平点亮 led
#define WIFI_LED_FAST_FLASH_MS      300         //快速闪烁时间 300ms 
#define WIFI_LED_LOW_FLASH_MS       500         //慢闪时间 500ms

/* Private variables ---------------------------------------------------------*/
LED_HANDLE wifi_led_handle; //定义 wifi led 句柄 

/* Private function prototypes -----------------------------------------------*/
VOID hw_report_all_dp_status(VOID);


#define SERVER_PORT            80 /*set up a tcp server,port at 20000*/

int my_fd = -1; 

int g_my_reconnect_mqtt_after_time = -1;


#define tcp_server_log(M, ...) os_printf("TCP", M, ##__VA_ARGS__)

int unw_recv(const int fd, void *buf, u32 nbytes)
{
    fd_set readfds, errfds;
    int ret = 0;   

    if( fd < 0 ) 
    {        
        return -1;//UNW_FAIL;
    } 

    FD_ZERO( &readfds );
    FD_ZERO( &errfds ); 
    FD_SET( fd, &readfds );
    FD_SET( fd, &errfds );

    ret = select( fd+1, &readfds, NULL, &errfds, NULL);
    os_printf("select ret:%d, %d, %d\r\n", ret, FD_ISSET( fd, &readfds ), FD_ISSET( fd, &errfds ));

    if(ret > 0 && FD_ISSET( fd, &readfds ))
        return recv(fd,buf,nbytes,0); 
    else
        return -1;//UNW_FAIL;
    
}

void tcp_client_thread( beken_thread_arg_t arg )
{
    OSStatus err = kNoErr;
    int fd = (int) arg;
    int len = 0;
    fd_set readfds, errfds, readfds2; 
    char *buf = NULL;
    char *reply = NULL;
	//char reply[8192];

    my_fd = fd;

    reply = (char*) os_malloc( 8192+2048 );
    buf = (char*) os_malloc( 1024 );
    ASSERT(buf);
    

    
    while ( 1 )
    {
           
        {
            len = recv( fd, buf, 1024, 0 );

            if ( len <= 0 )
            {
                os_printf( "TCP Client is disconnected, fd: %d", fd );
                goto exit;
            }
  
      PR_NOTICE( "TCP received string %s\n",buf );
		  
		  HTTP_ProcessPacket(buf, reply);

		///	strcpy(buf,"[WB2S example TCP reply!]");
			len = strlen(reply);
      PR_NOTICE( "TCP sending reply len %i\n",len );
            len = send( fd, reply, len, 0 );

            rtos_delay_milliseconds(10);
        }
    }
	
exit:
    if ( err != kNoErr ) 
		tcp_server_log( "TCP client thread exit with err: %d", err );
	
    if ( buf != NULL ) 
		os_free( buf );
    if ( reply != NULL ) 
		os_free( reply );
	
    close( fd );
    rtos_delete_thread( NULL );
}

volatile u8 test_flag = 0;
void close_tcp_client(void)
{
    os_printf("close_tcp_client:%d, %p\r\n", my_fd, rtos_get_current_thread());
    test_flag = 1;
    close( my_fd );
    my_fd = -1;
}

/* TCP server listener thread */
void tcp_server_thread( beken_thread_arg_t arg )
{
    (void)( arg );
    OSStatus err = kNoErr;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sockaddr_t_size = sizeof(client_addr);
    char client_ip_str[16];
    int tcp_listen_fd = -1, client_fd = -1;
    fd_set readfds;

    tcp_listen_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;/* Accept conenction request on all network interface */
    server_addr.sin_port = htons( SERVER_PORT );/* Server listen on port: 20000 */
    err = bind( tcp_listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
    
    err = listen( tcp_listen_fd, 0 );
    
    while ( 1 )
    {
        FD_ZERO( &readfds );
        FD_SET( tcp_listen_fd, &readfds );

        select( tcp_listen_fd + 1, &readfds, NULL, NULL, NULL);

        if ( FD_ISSET( tcp_listen_fd, &readfds ) )
        {
            client_fd = accept( tcp_listen_fd, (struct sockaddr *) &client_addr, &sockaddr_t_size );
            if ( client_fd >= 0 )
            {
                os_strcpy( client_ip_str, inet_ntoa( client_addr.sin_addr ) );
                tcp_server_log( "TCP Client %s:%d connected, fd: %d", client_ip_str, client_addr.sin_port, client_fd );
                if ( kNoErr
                     != rtos_create_thread( NULL, BEKEN_APPLICATION_PRIORITY, 
							                     "TCP Clients",
                                                 (beken_thread_function_t)tcp_client_thread,
                                                 0x800, 
                                                 (beken_thread_arg_t)client_fd ) ) 
                {
                    close( client_fd );
					client_fd = -1;
                }
            }
        }
    }
	
    if ( err != kNoErr ) 
		tcp_server_log( "Server listerner thread exit with err: %d", err );
	
    close( tcp_listen_fd );
    rtos_delete_thread( NULL );
}

void connect_to_wifi(const char *oob_ssid,const char *connect_key)
{
	/*network_InitTypeDef_adv_st	wNetConfigAdv;

	os_memset( &wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st) );
	
	os_strcpy((char*)wNetConfigAdv.ap_info.ssid, oob_ssid);
	hwaddr_aton("48:ee:0c:48:93:12", wNetConfigAdv.ap_info.bssid);
	wNetConfigAdv.ap_info.security = SECURITY_TYPE_WPA2_MIXED;
	wNetConfigAdv.ap_info.channel = 5;
	
	os_strcpy((char*)wNetConfigAdv.key, connect_key);
	wNetConfigAdv.key_len = os_strlen(connect_key);
	wNetConfigAdv.dhcp_mode = DHCP_CLIENT;
	wNetConfigAdv.wifi_retry_interval = 100;

	bk_wlan_start_sta_adv(&wNetConfigAdv);
	*/
    network_InitTypeDef_st network_cfg;
	
    os_memset(&network_cfg, 0x0, sizeof(network_InitTypeDef_st));

    os_strcpy((char *)network_cfg.wifi_ssid, oob_ssid);
    os_strcpy((char *)network_cfg.wifi_key, connect_key);

    network_cfg.wifi_mode = STATION;
    network_cfg.dhcp_mode = DHCP_CLIENT;
    network_cfg.wifi_retry_interval = 100;

    bk_printf("ssid:%s key:%s\r\n", network_cfg.wifi_ssid, network_cfg.wifi_key);
			
    bk_wlan_start(&network_cfg);
}



void demo_start_tcp()
{
    OSStatus err = kNoErr;

    err = rtos_create_thread( NULL, BEKEN_APPLICATION_PRIORITY, 
									"TCP_server", 
									(beken_thread_function_t)tcp_server_thread,
									0x800,
									(beken_thread_arg_t)0 );
    if(err != kNoErr)
    {
       os_printf("create \"TCP_server\" thread failed!\r\n");
    }
}
//reste is GPIO26

#define MY_RELAY              GPIO24
#define MY_LED_RED              GPIO7 

beken_timer_t led_timer;

#ifndef LWIP_MQTT_EXAMPLE_IPADDR_INIT
#if LWIP_IPV4
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(PP_HTONL(IPADDR_LOOPBACK))
#else
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT
#endif
#endif

static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT;
static mqtt_client_t* mqtt_client;

static const struct mqtt_connect_client_info_t mqtt_client_info =
{
  "test",
  "homeassistant", /* user */
  "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqssssasasasasa", /* pass */
  100,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
    PR_NOTICE("Publish result: %d\n", result);
  }
}
static void
mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_example_init(void);
void example_publish(mqtt_client_t *client, int channel, int iVal)
{
	char pub_topic[32];
	char pub_payload[128];
//  const char *pub_payload= "{\"temperature\": \"45.5\"}";
  err_t err;
  int myValue;
  u8_t qos = 2; /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0; /* No don't retain such crappy payload... */
  
  if(client==0)
	  return;
  if(mqtt_client_is_connected(client)==0) {
		 g_my_reconnect_mqtt_after_time = 5;
		return;
  }

 //  sprintf(pub_payload,"{\"temperature\": \"%i\"}",(int)(20+20*sin(cnt*0.01f)));
  myValue = CHANNEL_Check(channel);
   sprintf(pub_payload,"%i",myValue);
   
    PR_NOTICE("calling pub: \n");
	sprintf(pub_topic,"wb2s/%i/get",channel);
  err = mqtt_publish(client, pub_topic, pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, 0);
  if(err != ERR_OK) {
    PR_NOTICE("Publish err: %d\n", err);
	 if(err == ERR_CONN) {
		 
		// g_my_reconnect_mqtt_after_time = 5;

       // mqtt_example_init();

  //mqtt_client_connect(mqtt_client,
    //      &mqtt_ip, MQTT_PORT,
    //      mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
     //     &mqtt_client_info);
	 }
  }
}

int g_incoming_channel_mqtt = 0;
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	int iValue;
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  //PR_NOTICE("MQTT client in mqtt_incoming_data_cb\n");
  PR_NOTICE("MQTT client in mqtt_incoming_data_cb data is %s for ch %i\n",data,g_incoming_channel_mqtt);

  iValue = atoi(data);
  CHANNEL_Set(g_incoming_channel_mqtt,iValue);

 // PR_NOTICE(("MQTT client \"%s\" data cb: len %d, flags %d\n", client_info->client_id, (int)len, (int)flags));
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  //PR_NOTICE("MQTT client in mqtt_incoming_publish_cb\n");
  PR_NOTICE("MQTT client in mqtt_incoming_publish_cb topic %s\n",topic);
// TODO: better
  g_incoming_channel_mqtt = topic[5] - '0';
 // PR_NOTICE(("MQTT client \"%s\" publish cb: topic %s, len %d\n", client_info->client_id, topic, (int)tot_len));
}

static void
mqtt_request_cb(void *arg, err_t err)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

  PR_NOTICE(("MQTT client \"%s\" request cb: err %d\n", client_info->client_id, (int)err));
}
static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
  PR_NOTICE("Subscribe result: %i\n", result);
}
void example_do_connect(mqtt_client_t *client);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	int i;
	char tmp[32];
  err_t err = ERR_OK;
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  LWIP_UNUSED_ARG(client);

//  PR_NOTICE(("MQTT client < removed name > connection cb: status %d\n",  (int)status));
 // PR_NOTICE(("MQTT client \"%s\" connection cb: status %d\n", client_info->client_id, (int)status));

  if (status == MQTT_CONNECT_ACCEPTED) {
    PR_NOTICE("mqtt_connection_cb: Successfully connected\n");


  mqtt_set_inpub_callback(mqtt_client,
          mqtt_incoming_publish_cb,
          mqtt_incoming_data_cb,
          LWIP_CONST_CAST(void*, &mqtt_client_info));

	 /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */


    err = mqtt_sub_unsub(client,
            "wb2s/+/set", 1,
            mqtt_request_cb, LWIP_CONST_CAST(void*, client_info),
            1);
    if(err != ERR_OK) {
      PR_NOTICE("mqtt_subscribe return: %d\n", err);
    }


    //mqtt_sub_unsub(client,
    //        "topic_qos1", 1,
    //        mqtt_request_cb, LWIP_CONST_CAST(void*, client_info),
    //        1);
    //mqtt_sub_unsub(client,
    //        "topic_qos0", 0,
    //        mqtt_request_cb, LWIP_CONST_CAST(void*, client_info),
    //        1);
  } else {
    PR_NOTICE("mqtt_connection_cb: Disconnected, reason: %d\n", status);
    example_do_connect(client);

  }
}

void example_do_connect(mqtt_client_t *client)
{
  err_t err;



  	ipaddr_aton("192.168.0.113",&mqtt_ip);
	//ipaddr_aton("192.168.0.110",&mqtt_ip);
	//ipaddr_aton("192.168.0.114",&mqtt_ip);

  /* Initiate client and connect to server, if this fails immediately an error code is returned
     otherwise mqtt_connection_cb will be called with connection result after attempting
     to establish a connection with the server.
     For now MQTT version 3.1.1 is always used */

  mqtt_client_connect(mqtt_client,
          &mqtt_ip, MQTT_PORT,
          mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
          &mqtt_client_info);


  /* For now just print the result code if something goes wrong */
  if(err != ERR_OK) {
    PR_NOTICE("mqtt_connect return %d\n", err);
  }
}


void mqtt_example_init(void)
{
  mqtt_client = mqtt_client_new();

	

	example_do_connect(mqtt_client);
}

static void app_my_channel_toggle_callback(int channel, int iVal)
{
    PR_NOTICE("Channel has changed! Publishing change %i with %i \n",channel,iVal);
	example_publish(mqtt_client,channel,iVal);
}
int loopsWithDisconnected = 0;
static void app_led_timer_handler(void *data)
{
	if(mqtt_client != 0 && mqtt_client_is_connected(mqtt_client) == 0) {
		PR_NOTICE("Timer discovetrs disconnected mqtt %i\n",cnt);
		loopsWithDisconnected++;
		if(loopsWithDisconnected>10){ 
			example_do_connect(mqtt_client);
			loopsWithDisconnected = 0;
		}
	}
	//if(g_my_reconnect_mqtt_after_time>0)
	//{
	//	g_my_reconnect_mqtt_after_time--;
	//	if(g_my_reconnect_mqtt_after_time<=0){
	//		///mqtt_example_connect();
	//		g_my_reconnect_mqtt_after_time = -1;
	//	}
	//}
	cnt ++;
   // bk_gpio_output_reverse(MY_RELAY);
	if( cnt % 2 )
	{
	//	bk_gpio_output_reverse(MY_LED_RED);
		///example_publish(mqtt_client,0);
	}
	else
	{
		
	}

    PR_NOTICE("Timer is %i\n",cnt);
}
//void button_r0_short_press(void *param)
//{
//	PR_NOTICE("r0 key_short_press\r\n");
//}
//void button_r0_double_press(void *param)
//{
//	PR_NOTICE("r0 key_double_press\r\n");
//}
//void button_r0_long_press_hold(void *param)
//{
//	PR_NOTICE("r0 key_long_press_hold\r\n");
//}
//#define KEY_R0 GPIO6
//#define KEY_R1 GPIO26
//#define KEY_R2 GPIO8
//BUTTON_S button_r0;
//BUTTON_S button_r1;
//BUTTON_S button_r2;
//uint8_t button_r0_get_gpio_value(void)
//{
//	return bk_gpio_input(KEY_R0);
//}
//void button_r1_short_press(void *param)
//{
//	PR_NOTICE("r1 key_short_press\r\n");
//}
//void button_r1_double_press(void *param)
//{
//	PR_NOTICE("r1 key_double_press\r\n");
//}
//void button_r1_long_press_hold(void *param)
//{
//	PR_NOTICE("r1 key_long_press_hold\r\n");
//}
//uint8_t button_r1_get_gpio_value(void)
//{
//	return bk_gpio_input(KEY_R1);
//}
//void button_r2_short_press(void *param)
//{
//	PR_NOTICE("r2 key_short_press\r\n");
//}
//void button_r2_double_press(void *param)
//{
//	PR_NOTICE("r2 key_double_press\r\n");
//}
//void button_r2_long_press_hold(void *param)
//{
//	PR_NOTICE("r2 key_long_press_hold\r\n");
//}
//uint8_t button_r2_get_gpio_value(void)
//{
//	return bk_gpio_input(KEY_R2);
//}
void myInit()
{

    OSStatus err;
	
	PIN_Init();

	CHANNEL_SetChangeCallback(app_my_channel_toggle_callback);
	//key_configure();

	//{
	//	bk_gpio_config_input_pup(KEY_R0);
	//	button_init(&button_r0, button_r0_get_gpio_value, 0);
	//	button_attach(&button_r0, SINGLE_CLICK,     button_r0_short_press);
	//	button_attach(&button_r0, DOUBLE_CLICK,     button_r0_double_press);
	//	button_attach(&button_r0, LONG_PRESS_HOLD,  button_r0_long_press_hold);
	//	button_start(&button_r0);
	//}
	//{
	//	bk_gpio_config_input_pup(KEY_R1);
	//	button_init(&button_r1, button_r1_get_gpio_value, 0);
	//	button_attach(&button_r1, SINGLE_CLICK,     button_r1_short_press);
	//	button_attach(&button_r1, DOUBLE_CLICK,     button_r1_double_press);
	//	button_attach(&button_r1, LONG_PRESS_HOLD,  button_r1_long_press_hold);
	//	button_start(&button_r1);
	//}
	//{
	//	bk_gpio_config_input_pup(KEY_R2);
	//	button_init(&button_r2, button_r2_get_gpio_value, 0);
	//	button_attach(&button_r2, SINGLE_CLICK,     button_r2_short_press);
	//	button_attach(&button_r2, DOUBLE_CLICK,     button_r2_double_press);
	//	button_attach(&button_r2, LONG_PRESS_HOLD,  button_r2_long_press_hold);
	//	button_start(&button_r2);
	//}
	//

    bk_gpio_config_output(MY_RELAY);
    bk_gpio_output(MY_RELAY, 0);
	
    bk_gpio_config_output(MY_LED_RED);
    bk_gpio_output(MY_LED_RED, 0);

    err = rtos_init_timer(&led_timer,
                          1 * 1000,
                          app_led_timer_handler,
                          (void *)0);
    ASSERT(kNoErr == err);

    err = rtos_start_timer(&led_timer);
    ASSERT(kNoErr == err);
}


/* Private functions ---------------------------------------------------------*/
/**
 * @Function: wifi_state_led_reminder
 * @Description: WiFi led指示灯，根据当前 WiFi 状态，做出不同提示 
 * @Input: cur_stat：当前 WiFi 状态 
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_state_led_reminder(IN CONST GW_WIFI_NW_STAT_E cur_stat)
{
    switch (cur_stat)
    {
        case STAT_LOW_POWER:    //wifi 连接超时，进入低功耗模式
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭提示灯
        break;

        case STAT_UNPROVISION: //SamrtConfig 配网模式，等待连接
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_FAST_FLASH_MS, 0xffff); //led 快闪
        break;

        case STAT_AP_STA_UNCFG: //ap 配网模式，等待连接
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_LOW_FLASH_MS, 0xffff); //led 慢闪
        break;

        case STAT_AP_STA_DISC:
        case STAT_STA_DISC:     //SamrtConfig/ap 正在连接中
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭 led 
        break;

        case STAT_CLOUD_CONN:
        case STAT_AP_CLOUD_CONN: //连接到涂鸦云
            tuya_set_led_light_type(wifi_led_handle, OL_HIGH, 0, 0); //led 常量
        break;

        default:
        break;
    }
}

/**
 * @Function: wifi_key_process
 * @Description: 按键回调函数
 * @Input: port：触发引脚,type：按键触发类型,cnt:触发次数
 * @Output: none
 * @Return: none
 * @Others: 长按触发配网模式
 */
STATIC VOID wifi_key_process(TY_GPIO_PORT_E port,PUSH_KEY_TYPE_E type,INT_T cnt)
{
    PR_DEBUG("port:%d,type:%d,cnt:%d",port,type,cnt);
    OPERATE_RET op_ret = OPRT_OK;
    UCHAR_T ucConnectMode = 0;

    if (port = WIFI_KEY_PIN) {
        if (LONG_KEY == type) { //press long enter linking network
            PR_NOTICE("key long press");
            /* 手动移除设备 */
            tuya_iot_wf_gw_unactive();
        } else if (NORMAL_KEY == type) {
            PR_NOTICE("key normal press");
        } else {
            PR_NOTICE("key type is no deal");
        }
    }

    return;
}

/**
 * @Function: wifi_config_init
 * @Description: 初始化 WiFi 相关设备，按键，led指示灯
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_config_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* LED 相关初始化 */ 
    op_ret = tuya_create_led_handle(WIFI_LED_PIN, TRUE, &wifi_led_handle);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }
    tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭 LED

    /* 按键相关初始化 */
    KEY_USER_DEF_S key_def;

    op_ret = key_init(NULL, 0, WIFI_KEY_TIMER_MS);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }

    /* 初始化 key 相关参数 */
    memset(&key_def, 0, SIZEOF(key_def));
    key_def.port = WIFI_KEY_PIN;                            //按键引脚
    key_def.long_key_time = WIFI_KEY_LONG_PRESS_MS;         //长按时间配置
    key_def.low_level_detect = WIFI_KEY_LOW_LEVEL_ENABLE;   //TRUE:低电平算按下，FALSE：高电平算按下
    key_def.lp_tp = LP_ONCE_TRIG;   //
    key_def.call_back = wifi_key_process;                   //按键按下后回调函数
    key_def.seq_key_detect_time = WIFI_KEY_SEQ_PRESS_MS;    //连按间隔时间配置

    /* 注册按键 */
    op_ret = reg_proc_key(&key_def);
    if (op_ret != OPRT_OK) {
        PR_ERR("reg_proc_key err:%d", op_ret);
    }

    return;
}

/**
 * @Function: hw_report_all_dp_status
 * @Description: 上报所有 dp 点
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
VOID hw_report_all_dp_status(VOID)
{
    //report all dp status
}

/**
 * @Function:gpio_test 
 * @Description: gpio测试
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    return gpio_test_all(in, out);
}

/**
 * @Function: mf_user_callback
 * @Description: 授权回调函数
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 清空flash中存储的数据
 */
VOID mf_user_callback(VOID)
{
    hw_reset_flash_data();
    return;
}

/**
 * @Function: prod_test
 * @Description: 扫描到产测热点，进入回调函数，主要是按键、指示灯、继电器功能测试
 * @Input: flag:授权标识；rssi:信号强度
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID prod_test(BOOL_T flag, SCHAR_T rssi)
{
    if (flag == FALSE || rssi < -60) 
    {
        PR_ERR("Prod test failed... flag:%d, rssi:%d", flag, rssi);
        return;
    }
    PR_NOTICE("flag:%d rssi:%d", flag, rssi);

}

/**
 * @Function: app_init
 * @Description: 设备初始化，设置工作模式
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 无
 */
// NOTE: this is externally called from tuya_mainc
VOID app_init(VOID)
{
    app_cfg_set(WIFI_WORK_MODE_SEL, prod_test);

    /* 设置配网超时时间，未配网超时后退出配网模式 */
    tuya_iot_wf_timeout_set(WIFI_CONNECT_OVERTIME_S);

    /* WiFi 按键，led 初始化 */
    wifi_config_init();
}

/**
 * @Function: pre_device_init
 * @Description: 设备信息(SDK信息、版本号、固件标识等)打印、重启原因和打印等级设置
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID pre_device_init(VOID)
{
    PR_DEBUG("%s",tuya_iot_get_sdk_info());
    PR_DEBUG("%s:%s",APP_BIN_NAME,DEV_SW_VERSION);
    PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);
    PR_NOTICE("Hello Tuya World!");
    PR_NOTICE("system reset reason:[%s]",tuya_hal_system_get_rst_info());
    /* 打印等级设置 */
    SetLogManageAttr(TY_LOG_LEVEL_DEBUG);
}

/**
 * @Function: status_changed_cb
 * @Description: network status changed callback
 * @Input: status: current status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID status_changed_cb(IN CONST GW_STATUS_E status)
{
    PR_NOTICE("status_changed_cb is status:%d",status);

    if(GW_NORMAL == status) {
        hw_report_all_dp_status();
    }else if(GW_RESET == status) {
        PR_NOTICE("status is GW_RESET");
    }
}

/**
 * @Function: upgrade_notify_cb
 * @Description: firmware download finish result callback
 * @Input: fw: firmware info
 * @Input: download_result: 0 means download succes. other means fail
 * @Input: pri_data: private data
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    PR_DEBUG("download  Finish");
    PR_DEBUG("download_result:%d", download_result);
}

/**
 * @Function: get_file_data_cb
 * @Description: firmware download content process callback
 * @Input: fw: firmware info
 * @Input: total_len: firmware total size
 * @Input: offset: offset of this download package
 * @Input: data && len: this download package
 * @Input: pri_data: private data
 * @Output: remain_len: the size left to process in next cb
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset, \
                                     IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d", offset, len);

    return OPRT_OK;
}

/**
 * @Function: gw_ug_inform_cb
 * @Description: gateway ota firmware available nofity callback
 * @Input: fw: firmware info
 * @Output: none
 * @Return: int:
 * @Others: 
 */
INT_T gw_ug_inform_cb(IN CONST FW_UG_S *fw)
{
    PR_DEBUG("Rev GW Upgrade Info");
    PR_DEBUG("fw->fw_url:%s", fw->fw_url);
    PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    PR_DEBUG("fw->file_size:%d", fw->file_size);

    return tuya_iot_upgrade_gw(fw, get_file_data_cb, upgrade_notify_cb, NULL);
}

/**
 * @Function: hw_reset_flash_data
 * @Description: hardware reset, erase user data from flash
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
VOID hw_reset_flash_data(VOID)
{
    return;
}

/**
 * @Function: gw_reset_cb
 * @Description: gateway restart callback, app remove the device 
 * @Input: type:gateway reset type
 * @Output: none
 * @Return: none
 * @Others: reset factory clear flash data
 */
VOID gw_reset_cb(IN CONST GW_RESET_TYPE_E type)
{
    PR_DEBUG("gw_reset_cb type:%d",type);
    if(GW_REMOTE_RESET_FACTORY != type) {
        PR_DEBUG("type is GW_REMOTE_RESET_FACTORY");
        return;
    }

    hw_reset_flash_data();
}

/**
 * @Function: dev_obj_dp_cb
 * @Description: obj dp info cmd callback, tuya cloud dp(data point) received
 * @Input: dp:obj dp info
 * @Output: none
 * @Return: none
 * @Others: app send data by dpid  control device stat
 */
VOID dev_obj_dp_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    PR_DEBUG("dp->cid:%s dp->dps_cnt:%d",dp->cid,dp->dps_cnt);
    UCHAR_T i = 0;

    for(i = 0;i < dp->dps_cnt;i++) {
        //deal_dp_proc(&(dp->dps[i]));
        dev_report_dp_json_async(get_gw_cntl()->gw_if.id, dp->dps, dp->dps_cnt);
    }
}

/**
 * @Function: dev_raw_dp_cb
 * @Description: raw dp info cmd callback, tuya cloud dp(data point) received (hex data)
 * @Input: dp: raw dp info
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID dev_raw_dp_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    PR_DEBUG("raw data dpid:%d",dp->dpid);
    PR_DEBUG("recv len:%d",dp->len);
#if 1 
    INT_T i = 0;
    for(i = 0;i < dp->len;i++) {
        PR_DEBUG_RAW("%02X ",dp->data[i]);
    }
#endif
    PR_DEBUG_RAW("\n");
    PR_DEBUG("end");
    return;
}

/**
 * @Function: dev_dp_query_cb
 * @Description: dp info query callback, cloud or app actively query device info
 * @Input: dp_qry: query info
 * @Output: none
 * @Return: none
 * @Others: none
 */
STATIC VOID dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry) 
{
    PR_NOTICE("Recv DP Query Cmd");

    hw_report_all_dp_status();
}

/**
 * @Function: wf_nw_status_cb
 * @Description: tuya-sdk network state check callback
 * @Input: stat: curr network status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID wf_nw_status_cb(IN CONST GW_WIFI_NW_STAT_E stat)
{
    PR_NOTICE("wf_nw_status_cb,wifi_status:%d", stat);
    wifi_state_led_reminder(stat);

    if(stat == STAT_AP_STA_CONN || stat >= STAT_STA_CONN) {
        hw_report_all_dp_status();
    }
}

/**
 * @Function: device_init
 * @Description: device initialization process 
 * @Input: none
 * @Output: none
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
OPERATE_RET device_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

#ifdef MY_TUYA_STILL_ENABLED
    TY_IOT_CBS_S wf_cbs = {
        status_changed_cb,\ 
        gw_ug_inform_cb,\   
        gw_reset_cb,\
        dev_obj_dp_cb,\
        dev_raw_dp_cb,\
        dev_dp_query_cb,\
        NULL,
    };

    op_ret = tuya_iot_wf_soc_dev_init_param(WIFI_WORK_MODE_SEL, WF_START_SMART_FIRST, &wf_cbs, NULL, PRODECT_ID, DEV_SW_VERSION);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_wf_soc_dev_init_param error,err_num:%d",op_ret);
        return op_ret;
    }

    op_ret = tuya_iot_reg_get_wf_nw_stat_cb(wf_nw_status_cb);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb is error,err_num:%d",op_ret);
        return op_ret;
    }
#endif
	
	myInit();

	connect_to_wifi("ssid","pass");
	//demo_start_upd();
	demo_start_tcp();
#if 0
	// https://www.elektroda.pl/rtvforum/topic3804553.html
	// SmartSwitch Tuya WL-SW01_16 16A
	PIN_SetPinRoleForPinIndex(7, IOR_Relay);
	PIN_SetPinChannelForPinIndex(7, 1);

	PIN_SetPinRoleForPinIndex(26, IOR_Button);
	PIN_SetPinChannelForPinIndex(26, 1);
#else
	// https://www.elektroda.pl/rtvforum/topic3822484.html
	//  WiFi Tuya SmartLife 4CH 10A

	PIN_SetPinRoleForPinIndex(7, IOR_Button);
	PIN_SetPinChannelForPinIndex(7, 1);
	PIN_SetPinRoleForPinIndex(8, IOR_Button);
	PIN_SetPinChannelForPinIndex(8, 2);
	PIN_SetPinRoleForPinIndex(9, IOR_Button);
	PIN_SetPinChannelForPinIndex(9, 3);
	PIN_SetPinRoleForPinIndex(1, IOR_Button);
	PIN_SetPinChannelForPinIndex(1, 4);

	PIN_SetPinRoleForPinIndex(14, IOR_Relay);
	PIN_SetPinChannelForPinIndex(14, 1);
	PIN_SetPinRoleForPinIndex(6, IOR_Relay);
	PIN_SetPinChannelForPinIndex(6, 2);
	PIN_SetPinRoleForPinIndex(24, IOR_Relay);
	PIN_SetPinChannelForPinIndex(24, 3);
	PIN_SetPinRoleForPinIndex(26, IOR_Relay);
	PIN_SetPinChannelForPinIndex(26, 4);

#endif

	mqtt_example_init();

    return op_ret;
}
