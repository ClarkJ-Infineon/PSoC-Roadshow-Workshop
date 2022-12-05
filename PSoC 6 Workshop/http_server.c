
/*
 * http_server.c
 *
 *      Author: jarvisc
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#include <FreeRTOS.h>

#include "cy_wcm.h"
#include "cy_nw_helper.h"
#include "cy_http_server.h"
#include "cy_secure_sockets.h"

#include "html_web_page.h"


#define HTTP_HEADER_204                              "HTTP/1.1 204 No Content"
#define HTTP_REQUEST_HANDLE_SUCCESS                  (0)
#define MAX_SOCKETS                                  (4)
#define HTTP_REQUEST_HANDLE_ERROR                    (-1)
#define HTTP_PORT                                    (80u)
#define EVENT_STREAM_DATA                            "data: "
#define LFLF                                         "\n\n"
#define CHUNKED_CONTENT_LENGTH                       (0u)

cy_http_server_t          		http_ap_server;
cy_network_interface_t    		nw_interface;
cy_socket_sockaddr_t      		http_server_ip_address;
cy_http_response_stream_t*     	http_event_stream;

extern uint32_t cm4_led_value;
extern TaskHandle_t main_task_handle;


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
void start_http_server(void)
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


	cy_http_server_start(http_ap_server);
}


// Function to send Server Side Response Stream, updating web client page
void send_http_stream(const void* http_response)
{

	cy_http_server_response_stream_write_payload( http_event_stream, EVENT_STREAM_DATA, sizeof(EVENT_STREAM_DATA)-1 );
	cy_http_server_response_stream_write_payload( http_event_stream, http_response, strlen( (char *) http_response ) );
	cy_http_server_response_stream_write_payload( http_event_stream, LFLF, sizeof(LFLF)-1 );

}

