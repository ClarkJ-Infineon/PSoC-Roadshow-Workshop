<<<<<<< Updated upstream
/******************************************************************************
* File Name:   main.c
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

#include "cy_serial_flash_qspi.h"
#include "cycfg_qspi_memslot.h"

#include "cy_wcm.h"
#include "cy_nw_helper.h"

#include "cy_http_server.h"
#include "http_server.h"

/* RTOS related macros. */
#define MAIN_TASK_STACK_SIZE        (10 * 1024)
#define MAIN_TASK_PRIORITY          (6)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void main_task(void *arg);
void snippet_wcm_ap_get_client();
void configure_http_server(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
TaskHandle_t main_task_handle;
volatile int uxTopUsedPriority;
uint32_t cm4_led_value = CYBSP_LED_STATE_OFF;


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU and serves as a template for a basic
* FreeRTOS application.  This function initializes the device, create a FreeRTOS
* task, and starts the FreeRTOS scheduler.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
	cy_rslt_t result;

	/* This enables RTOS aware debugging in OpenOCD */
	uxTopUsedPriority = configMAX_PRIORITIES - 1;

	/* Initialize the device and board peripherals */
	result = cybsp_init() ;
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	/* Enable global interrupts */
	__enable_irq();

	// TODO Initialize Serial Flash
	// Init QSPI and enable XIP to get the Wi-Fi firmware from the QSPI NOR flash
	cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
								  CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
								  CYBSP_QSPI_SCK, CYBSP_QSPI_SS, 50000000lu);
	cy_serial_flash_qspi_enable_xip(true);


	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
	printf("\x1b[2J\x1b[;H");
	printf("Hello PSoC 6 Technical Workshop\n");

	xTaskCreate(main_task, "Main Task", MAIN_TASK_STACK_SIZE , NULL, MAIN_TASK_PRIORITY, &main_task_handle);

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/* Should never get here */
	CY_ASSERT(0);
}


/*******************************************************************************
* Function Name: main_task
********************************************************************************
* Summary:
* This is the main FreeRTOS task.  This task will be used to within the
* workshop to configure and start the WiFi Access Point and HTTP Server.
* Within the for loop, the LED status will be monitor and updated as needed
* based on FreeRTOS task notifications.
*
* Parameters:
*  arg: task argument
*
* Return:
*  None
*
*******************************************************************************/
void main_task(void *arg)
{
	(void) arg;

	char http_response[64] = {0};

	printf("This is the main task\n");

	// TODO Get IPC Message Queue Handle
	// Retrieve handle from shared memory
	cyhal_ipc_t cm4_msg_queue;
	cyhal_ipc_queue_get_handle(&cm4_msg_queue, CYHAL_IPC_CHAN_0, 1UL);


	// TODO Create Access Point
	printf("Starting Access Point\n");
	snippet_wcm_ap_get_client();


	// TODO Create HTTP Server
	printf("Starting HTTP Server\n");
	configure_http_server();
	cy_http_server_start(http_ap_server);


	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		// Toggle LED variable
    	if (cm4_led_value == CYBSP_LED_STATE_ON) {
    		sprintf( http_response, "The LED is turned ON.");

    	} else {
    		sprintf( http_response, "The LED is turned OFF.");


    	}

    	cy_http_server_response_stream_write_payload( http_event_stream, EVENT_STREAM_DATA, sizeof(EVENT_STREAM_DATA)-1 );
    	cy_http_server_response_stream_write_payload( http_event_stream, http_response, sizeof(http_response)-1 );
    	cy_http_server_response_stream_write_payload( http_event_stream, LFLF, sizeof(LFLF)-1 );


    	cyhal_ipc_queue_put(&cm4_msg_queue, &cm4_led_value, CYHAL_IPC_NEVER_TIMEOUT);
		vTaskDelay(100);
	}
}

#define WIFI_SSID_AP                      "WorkshopSSID"
#define WIFI_KEY_AP                       "mypassword"

static cy_wcm_ip_setting_t ap_ip_settings;

static void ap_eve_callback(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{
    char ip4_str[16];
    cy_nw_ip_address_t nw_ipv4;

    printf("######### Received event changed from wcm, event = %d #######\n", event);
    switch(event)
    {
        case CY_WCM_EVENT_DISCONNECTED:
        {
            printf("Network is down! \n");
            break;
        }
        case CY_WCM_EVENT_RECONNECTED:
        {
            printf("Network is up again! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTING:
        {
            printf("Connecting to AP ... \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTED:
        {
            printf("Connected to AP and network is up !! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECT_FAILED:
        {
            printf("Connection to AP Failed ! \n");
            break;
        }
        case CY_WCM_EVENT_IP_CHANGED:
        {
            cy_wcm_ip_address_t ip_addr;
            cy_wcm_get_ip_addr(CY_WCM_INTERFACE_TYPE_STA, &ip_addr);

            if(event_data->ip_addr.version == CY_WCM_IP_VER_V4)
            {
                nw_ipv4.ip.v4 = event_data->ip_addr.ip.v4;
                cy_nw_ntoa(&nw_ipv4, ip4_str);
                printf("IPV4 address: %s\n", ip4_str);
            }
            break;
        }
        case CY_WCM_EVENT_STA_JOINED_SOFTAP:
        {
            printf("mac address of the STA which joined = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            break;
        }
        case CY_WCM_EVENT_STA_LEFT_SOFTAP:
        {
            printf("mac address of the STA which left = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            break;
        }
        default:
        {
            printf("Invalid event \n");
            break;
        }
    }
}

void snippet_wcm_ap_get_client()
{
    cy_rslt_t result;
    char ip6_str[40];
    char ip4_str[16];
    char *ip = "192.168.0.2";
    char *netmask = "255.255.255.0";
    char *gateway = "192.168.0.2";
    cy_nw_ip_address_t nw_ipv6;
    cy_nw_ip_address_t nw_ipv4;

    cy_wcm_config_t config;
    cy_wcm_ap_config_t ap_conf;
    cy_wcm_ip_address_t ipv6_addr;

    /* Initialize the Wi-Fi device, Wi-Fi transport, and lwIP network stack.*/
    config.interface = CY_WCM_INTERFACE_TYPE_AP;
    result = cy_wcm_init(&config);
    if(result != CY_RSLT_SUCCESS)
    {
        printf("\ncy_wcm_init failed...!\n");
        return;
    }

    /* AP settings */
    cy_wcm_set_ap_ip_setting(&ap_ip_settings, ip, netmask, gateway, CY_WCM_IP_VER_V4);

    memset(&ap_conf, 0, sizeof(cy_wcm_ap_config_t));
    ap_conf.channel = 1;
    memcpy(ap_conf.ap_credentials.SSID, WIFI_SSID_AP, strlen(WIFI_SSID_AP) + 1);
    memcpy(ap_conf.ap_credentials.password, WIFI_KEY_AP, strlen(WIFI_KEY_AP) + 1);
    ap_conf.ap_credentials.security = CY_WCM_SECURITY_WPA2_AES_PSK;

    ap_conf.ip_settings.ip_address = ap_ip_settings.ip_address;
    ap_conf.ip_settings.netmask = ap_ip_settings.netmask;
    ap_conf.ip_settings.gateway = ap_ip_settings.gateway;

    printf("configured ip address of the AP = %u \n", (uint8_t)ap_conf.ip_settings.ip_address.ip.v4);
    nw_ipv4.ip.v4 = ap_conf.ip_settings.ip_address.ip.v4;
    cy_nw_ntoa(&nw_ipv4, ip4_str);
    printf("IPV4 address: %s\n", ip4_str);

    /* Start AP */
    result = cy_wcm_start_ap(&ap_conf);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_start_ap failed....! \n");
        return;
    }

    /* Register AP event callback */
    result = cy_wcm_register_event_callback(&ap_eve_callback);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_register_event_callback failed....! \n");
        return;
    }

    /* Link Local IPV6 AP address for AP */
    result = cy_wcm_get_ipv6_addr(CY_WCM_INTERFACE_TYPE_AP, CY_WCM_IPV6_LINK_LOCAL, &ipv6_addr);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_get_ipv6_addr failed....! \n");
        return;
    }

    nw_ipv6.ip.v6[0] = ipv6_addr.ip.v6[0];
    nw_ipv6.ip.v6[1] = ipv6_addr.ip.v6[1];
    nw_ipv6.ip.v6[2] = ipv6_addr.ip.v6[2];
    nw_ipv6.ip.v6[3] = ipv6_addr.ip.v6[3];
    cy_nw_ntoa_ipv6(&nw_ipv6, ip6_str);
    printf("IPV6 address: %s\n", ip6_str);

    return;

}

// Dynamic Event handler for HTTP POST and GET requests
int32_t softap_resource_handler(const char *url_path,
                                 const char *url_parameters,
                                 cy_http_response_stream_t *stream,
                                 void *arg,
                                 cy_http_message_body_t *http_message_body)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int32_t status = HTTP_REQUEST_HANDLE_SUCCESS;

    switch (http_message_body->request_type)
    {
    case CY_HTTP_REQUEST_GET:
    	/* Send the data of the device */
    	result = cy_http_server_response_stream_write_payload(stream, SOFTAP_DEVICE_DATA, sizeof(SOFTAP_DEVICE_DATA) - 1);
    	break;

    case CY_HTTP_REQUEST_POST:

		/* Compare the input from client to increase or decrease pwm value. */
		if(!strncmp((char *)http_message_body->data, "Enable", 6))
		{
			// TODO Code when LED Turned On
			cm4_led_value = CYBSP_LED_STATE_ON;
		}
		else if(!strncmp((char *)http_message_body->data, "Disable", 7))
		{
			// TODO Code when LED Turned Off
			cm4_led_value = CYBSP_LED_STATE_OFF;
		}

		// TODO Notify Main task to update LED
		xTaskNotifyGive(main_task_handle);

    	/* Send the HTTP response. */
    	result = cy_http_server_response_stream_write_payload(stream, HTTP_HEADER_204, sizeof(HTTP_HEADER_204) - 1);
    	break;

    default:
        break;
    }

    if (CY_RSLT_SUCCESS != result)
    {
        status = HTTP_REQUEST_HANDLE_ERROR;
    }

    return status;
}

// TODO Add Server Side Event Handler function here...
// Server Side Event handler
int32_t process_sse_handler( const char* url_path, const char* url_parameters,
                                   cy_http_response_stream_t* stream, void* arg,
                                   cy_http_message_body_t* http_message_body )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Assign the incoming stream to http_event_stream pointer */
    http_event_stream = stream;

    /* Enable chunked transfer encoding on the HTTP stream */
    result = cy_http_server_response_stream_enable_chunked_transfer( http_event_stream );

    result = cy_http_server_response_stream_write_header( http_event_stream, CY_HTTP_200_TYPE,
                                                CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED,
                                                MIME_TYPE_TEXT_EVENT_STREAM );
    return result;
}


// Function to configure HTTP server and register POST/GET handler
void configure_http_server(void)
{
	cy_wcm_ip_address_t ip_addr;

	/* IP address of SoftAp. */
	cy_wcm_get_ip_addr(CY_WCM_INTERFACE_TYPE_AP, &ip_addr);
	http_server_ip_address.ip_address.ip.v4 = ip_addr.ip.v4;
	http_server_ip_address.ip_address.version = CY_SOCKET_IP_VER_V4;

	/* Add IP address information to network interface object. */
	nw_interface.object = (void *)&http_server_ip_address;
	nw_interface.type = CY_NW_INF_TYPE_WIFI;

	/* Initialize secure socket library. */
	cy_http_server_network_init();

	/* Allocate memory needed for secure HTTP server. */
	cy_http_server_create(&nw_interface, HTTP_PORT, MAX_SOCKETS, NULL, &http_ap_server);

	/* Configure dynamic and register resource handler. */
	cy_resource_dynamic_data_t http_get_post_resource;
	http_get_post_resource.resource_handler = softap_resource_handler;
	http_get_post_resource.arg = NULL;
	cy_http_server_register_resource(http_ap_server,
			(uint8_t *)"/",
			(uint8_t *)"text/html",
			CY_DYNAMIC_URL_CONTENT,
			&http_get_post_resource);

	// TODO Register the Server Side Event Handler here...
	// Function calls to register server side event handler inside configure_http_server() function
	cy_resource_dynamic_data_t dynamic_sse_resource;
	dynamic_sse_resource.resource_handler = process_sse_handler;
	dynamic_sse_resource.arg = NULL;
	cy_http_server_register_resource( http_ap_server,
		(uint8_t*) "/events",
		(uint8_t*)"text/event-stream",
		CY_RAW_DYNAMIC_URL_CONTENT,
		&dynamic_sse_resource);

}


/* [] END OF FILE */
=======
/******************************************************************************
* File Name:   main.c
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

// Header file for serial flash driver
#include "cy_serial_flash_qspi.h"

// Header files for serial flash library and QSPI Configurator generated code
#include "cycfg_qspi_memslot.h"

#include "cy_wcm.h"
#include "cy_nw_helper.h"

#include "http_server.h"


/* RTOS related macros. */
#define MAIN_TASK_STACK_SIZE        (10 * 1024)
#define MAIN_TASK_PRIORITY          (6)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void main_task(void *arg);
void snippet_wcm_ap_get_client();

/*******************************************************************************
* Global Variables
*******************************************************************************/
TaskHandle_t main_task_handle;
volatile int uxTopUsedPriority;
uint32_t cm4_led_value = CYBSP_LED_STATE_ON;


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU and serves as a template for a basic
* FreeRTOS application.  This function initializes the device, create a FreeRTOS
* task, and starts the FreeRTOS scheduler.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
	cy_rslt_t result;

	/* This enables RTOS aware debugging in OpenOCD */
	uxTopUsedPriority = configMAX_PRIORITIES - 1;

	/* Initialize the device and board peripherals */
	result = cybsp_init() ;
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	/* Enable global interrupts */
	__enable_irq();

	// Init QSPI and enable XIP to get the Wi-Fi firmware from the QSPI NOR flash
	cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
								  CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
								  CYBSP_QSPI_SCK, CYBSP_QSPI_SS, 50000000lu);
	cy_serial_flash_qspi_enable_xip(true);


	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
	printf("\x1b[2J\x1b[;H");
	printf("Hello PSoC 6 Technical Workshop\n");

	xTaskCreate(main_task, "Main Task", MAIN_TASK_STACK_SIZE , NULL, MAIN_TASK_PRIORITY, &main_task_handle);

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/* Should never get here */
	CY_ASSERT(0);
}


/*******************************************************************************
* Function Name: main_task
********************************************************************************
* Summary:
* This is the main FreeRTOS task.  This task will be used to within the
* workshop to configure and start the WiFi Access Point and HTTP Server.
* Within the for loop, the LED status will be monitor and updated as needed
* based on FreeRTOS task notifications.
*
* Parameters:
*  arg: task argument
*
* Return:
*  None
*
*******************************************************************************/
void main_task(void *arg)
{
	(void) arg;
	char http_response[64] = {0}; // character array for hold SSE message

	printf("This is the main task\n");

	// Retrieve handle from shared memory
	cyhal_ipc_t cm4_msg_queue;
	cyhal_ipc_queue_get_handle(&cm4_msg_queue, CYHAL_IPC_CHAN_0, 1UL);

	// Create and Start Access Point
	printf("Starting Wi-Fi Access Point\n");
	snippet_wcm_ap_get_client();

	// Configure and Start HTTP Server
	printf("Starting HTTP Server\n");
	start_http_server();

	for (;;)
	{
		// Take Task Notification (waiting max delay)
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		// Toggle LED variable
    	if (cm4_led_value == CYBSP_LED_STATE_ON) {
    		sprintf( http_response, "The LED is turned ON." );  // if cm4_led_value == CYBSP_LED_STATE_ON


    	} else {
    		sprintf( http_response, "The LED is turned OFF." ); // else condition


    	}

    	// Put message into queue, waiting if queue is full
    	cyhal_ipc_queue_put(&cm4_msg_queue, &cm4_led_value, CYHAL_IPC_NEVER_TIMEOUT);

    	// Send message payload to SSE stream function (included in http_server.c)
    	send_http_stream(&http_response);

		vTaskDelay(100);
	}
}


#define WIFI_SSID_AP                      "PSoC-SSID"
#define WIFI_KEY_AP                       "mypassword"

static cy_wcm_ip_setting_t ap_ip_settings;

static void ap_eve_callback(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{
    char ip4_str[16];
    cy_nw_ip_address_t nw_ipv4;

    printf("######### Received event changed from wcm, event = %d #######\n", event);
    switch(event)
    {
        case CY_WCM_EVENT_DISCONNECTED:
        {
            printf("Network is down! \n");
            break;
        }
        case CY_WCM_EVENT_RECONNECTED:
        {
            printf("Network is up again! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTING:
        {
            printf("Connecting to AP ... \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTED:
        {
            printf("Connected to AP and network is up !! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECT_FAILED:
        {
            printf("Connection to AP Failed ! \n");
            break;
        }
        case CY_WCM_EVENT_IP_CHANGED:
        {
            cy_wcm_ip_address_t ip_addr;
            cy_wcm_get_ip_addr(CY_WCM_INTERFACE_TYPE_STA, &ip_addr);

            if(event_data->ip_addr.version == CY_WCM_IP_VER_V4)
            {
                nw_ipv4.ip.v4 = event_data->ip_addr.ip.v4;
                cy_nw_ntoa(&nw_ipv4, ip4_str);
                printf("IPV4 address: %s\n", ip4_str);
            }
            break;
        }
        case CY_WCM_EVENT_STA_JOINED_SOFTAP:
        {
            printf("mac address of the STA which joined = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            break;
        }
        case CY_WCM_EVENT_STA_LEFT_SOFTAP:
        {
            printf("mac address of the STA which left = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            break;
        }
        default:
        {
            printf("Invalid event \n");
            break;
        }
    }
}

void snippet_wcm_ap_get_client()
{
    cy_rslt_t result;
    char ip6_str[40];
    char ip4_str[16];
    char *ip = "192.168.0.2";
    char *netmask = "255.255.255.0";
    char *gateway = "192.168.0.2";
    cy_nw_ip_address_t nw_ipv6;
    cy_nw_ip_address_t nw_ipv4;

    cy_wcm_config_t config;
    cy_wcm_ap_config_t ap_conf;
    cy_wcm_ip_address_t ipv6_addr;

    /* Initialize the Wi-Fi device, Wi-Fi transport, and lwIP network stack.*/
    config.interface = CY_WCM_INTERFACE_TYPE_AP;
    result = cy_wcm_init(&config);
    if(result != CY_RSLT_SUCCESS)
    {
        printf("\ncy_wcm_init failed...!\n");
        return;
    }

    /* AP settings */
    cy_wcm_set_ap_ip_setting(&ap_ip_settings, ip, netmask, gateway, CY_WCM_IP_VER_V4);

    memset(&ap_conf, 0, sizeof(cy_wcm_ap_config_t));
    ap_conf.channel = 1;
    memcpy(ap_conf.ap_credentials.SSID, WIFI_SSID_AP, strlen(WIFI_SSID_AP) + 1);
    memcpy(ap_conf.ap_credentials.password, WIFI_KEY_AP, strlen(WIFI_KEY_AP) + 1);
    ap_conf.ap_credentials.security = CY_WCM_SECURITY_WPA2_AES_PSK;

    ap_conf.ip_settings.ip_address = ap_ip_settings.ip_address;
    ap_conf.ip_settings.netmask = ap_ip_settings.netmask;
    ap_conf.ip_settings.gateway = ap_ip_settings.gateway;

    printf("configured ip address of the AP = %u \n", (uint8_t)ap_conf.ip_settings.ip_address.ip.v4);
    nw_ipv4.ip.v4 = ap_conf.ip_settings.ip_address.ip.v4;
    cy_nw_ntoa(&nw_ipv4, ip4_str);
    printf("IPV4 address: %s\n", ip4_str);

    /* Start AP */
    result = cy_wcm_start_ap(&ap_conf);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_start_ap failed....! \n");
        return;
    }

    /* Register AP event callback */
    result = cy_wcm_register_event_callback(&ap_eve_callback);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_register_event_callback failed....! \n");
        return;
    }

    /* Link Local IPV6 AP address for AP */
    result = cy_wcm_get_ipv6_addr(CY_WCM_INTERFACE_TYPE_AP, CY_WCM_IPV6_LINK_LOCAL, &ipv6_addr);
    if( result != CY_RSLT_SUCCESS )
    {
        printf("\ncy_wcm_get_ipv6_addr failed....! \n");
        return;
    }

    nw_ipv6.ip.v6[0] = ipv6_addr.ip.v6[0];
    nw_ipv6.ip.v6[1] = ipv6_addr.ip.v6[1];
    nw_ipv6.ip.v6[2] = ipv6_addr.ip.v6[2];
    nw_ipv6.ip.v6[3] = ipv6_addr.ip.v6[3];
    cy_nw_ntoa_ipv6(&nw_ipv6, ip6_str);
    printf("IPV6 address: %s\n", ip6_str);

    return;

}


/* [] END OF FILE */
>>>>>>> Stashed changes
