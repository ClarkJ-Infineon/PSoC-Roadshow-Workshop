
/****************************************************************************************************************************************
* Section 3 - Step 2
* Reference code for allocating and initialized an IPC Message Queue (for the CM0 main.c) 
****************************************************************************************************************************************/
// Variable declaration for message queue and payload
cyhal_ipc_t cm0_msg_queue;
uint32_t cm0_led_value = CYBSP_LED_STATE_ON;

// Pointer creation and shared memory allocatino for queue pool and handle
void* queue_pool;
cyhal_ipc_queue_t* queue_handle;
CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool,1UL,sizeof(cm0_led_value));
CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_handle);

// Configuration of Message Queue objecy
queue_handle->channel_num = CYHAL_IPC_CHAN_0;
queue_handle->queue_num = 1UL;
queue_handle->queue_pool = queue_pool;
queue_handle->num_items = 1UL;
queue_handle->item_size = sizeof(cm0_led_value);

//Initialization of Message Queue
cyhal_ipc_queue_init(&cm0_msg_queue, queue_handle);



/****************************************************************************************************************************************
* Section 3 - Step 6
* Reference code for retrieving IPC Message Queue handle (for the CM4 main.c) 
****************************************************************************************************************************************/
// Retrieve handle from shared memory
cyhal_ipc_t cm4_msg_queue;
cyhal_ipc_queue_get_handle(&cm4_msg_queue, CYHAL_IPC_CHAN_0, 1UL);



/****************************************************************************************************************************************
* Section 5 - Step 4
* Reference code for initialize serial flash and enabling XIP for Wi-Fi firmware
****************************************************************************************************************************************/
// Header files for serial flash library and QSPI Configurator generated code
#include "cycfg_qspi_memslot.h"

// Init QSPI and enable XIP to get the Wi-Fi firmware from the QSPI NOR flash
cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
							  CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
							  CYBSP_QSPI_SCK, CYBSP_QSPI_SS, 50000000lu);
cy_serial_flash_qspi_enable_xip(true);



/****************************************************************************************************************************************
* Section 6 - Step 7
* Reference code for HTTP Server - derived from Web Server code example
* Includes two functions:
*   softap_resource_handler(...)
*   configure_http_server()
****************************************************************************************************************************************/
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
		}
		else if(!strncmp((char *)http_message_body->data, "Disable", 7))
		{
			// TODO Code when LED Turned Off
		}

		// TODO Notify Main task to update LED
		
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

}



/****************************************************************************************************************************************
* Section 7 - Step 1
* Reference code for adding support for server side event handling
****************************************************************************************************************************************/
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



/****************************************************************************************************************************************
* Section 7 - Step 2
* Reference code for registering the Server Side Event handler (within the configure_http_server function)
****************************************************************************************************************************************/
// Function calls to register server side event handler inside configure_http_server() function
cy_resource_dynamic_data_t dynamic_sse_resource;
dynamic_sse_resource.resource_handler = process_sse_handler;
dynamic_sse_resource.arg = NULL;
cy_http_server_register_resource( http_ap_server,
	(uint8_t*) "/events",
	(uint8_t*)"text/event-stream",
	CY_RAW_DYNAMIC_URL_CONTENT,
	&dynamic_sse_resource);



/****************************************************************************************************************************************
* Section 7 - Step 3
* Reference code for registering the Server Side Event handler (within the configure_http_server function)
****************************************************************************************************************************************/
// Write payload stream to web client, updating the LED status
cy_http_server_response_stream_write_payload( http_event_stream, EVENT_STREAM_DATA, sizeof(EVENT_STREAM_DATA)-1 );
cy_http_server_response_stream_write_payload( http_event_stream, http_response, sizeof(http_response)-1 );
cy_http_server_response_stream_write_payload( http_event_stream, LFLF, sizeof(LFLF)-1 );
