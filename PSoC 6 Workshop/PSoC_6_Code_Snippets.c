
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
* Section 3 - Step 4
* Reference code for 'getting' queue message (for the CM0 main.c)
****************************************************************************************************************************************/
// Wait for queue message to be recieved and get message from queue
cyhal_ipc_queue_get(&cm0_msg_queue, &cm0_led_value, CYHAL_IPC_NEVER_TIMEOUT);



/****************************************************************************************************************************************
* Section 3 - Step 5
* Reference code for initializing and driving a GPIO (for the CM0 main.c)
* Init function goes above for loop
* Write function goes within for loop, after queue get function
****************************************************************************************************************************************/
// Initialize GPIO using HAL APIs
cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, cm0_led_value);

// Drive value out to GPIO
cyhal_gpio_write(CYBSP_USER_LED, cm0_led_value);



/****************************************************************************************************************************************
* Section 3 - Step 6
* Reference code for retrieving IPC Message Queue handle (for the CM4 main.c) 
****************************************************************************************************************************************/
// Retrieve handle from shared memory
cyhal_ipc_t cm4_msg_queue;
cyhal_ipc_queue_get_handle(&cm4_msg_queue, CYHAL_IPC_CHAN_0, 1UL);



/****************************************************************************************************************************************
* Section 3 - Step 7
* Reference code for 'putting' queue message (for the CM4 main.c)
****************************************************************************************************************************************/
// Put message into queue, waiting if queue is full
cyhal_ipc_queue_put(&cm4_msg_queue, &cm4_led_value, CYHAL_IPC_NEVER_TIMEOUT);



/****************************************************************************************************************************************
* Section 5 - Step 5
* Include files required for Wi-Fi Access Point - Code Snippet 8
****************************************************************************************************************************************/
#include "cy_wcm.h"
#include "cy_nw_helper.h"



/****************************************************************************************************************************************
* Section 5 - Step 6
* Code to start Wi-Fi Access Point (calling function included in Code Snippet)
****************************************************************************************************************************************/
// Function prototype to be added at top of main.c
void snippet_wcm_ap_get_client();

// Create and Start Access Point
printf("Starting Wi-Fi Access Point\n");
snippet_wcm_ap_get_client();



/****************************************************************************************************************************************
* Section 6 - Step 5
* Include files required for HTTP Server - Derived from Wi-Fi Server Code Example
****************************************************************************************************************************************/
#include "http_server.h"



/****************************************************************************************************************************************
* Section 6 - Step 6
* Code to start HTTP Server (callin function derived from Wi-Fi Server Code Example)
****************************************************************************************************************************************/
// Initialize and Start HTTP Server
printf("Start HTTP Server\n");
start_http_server();




/****************************************************************************************************************************************
* Section 7 - Step 1
* Update value of LED variable based on POST command, then using Task Notify to update main task
****************************************************************************************************************************************/
// Code when LED Turned On
cm4_led_value = CYBSP_LED_STATE_ON;

// Code when LED Turned Off
cm4_led_value = CYBSP_LED_STATE_OFF;

// Notify Main task to update LED
xTaskNotifyGive(main_task_handle);



/****************************************************************************************************************************************
* Section 7 - Step 2
* Wait for Task Notify from HTTP Server task
****************************************************************************************************************************************/
// Take Task Notification (waiting max delay)
ulTaskNotifyTake(pdTRUE, portMAX_DELAY);



/****************************************************************************************************************************************
* Section 8 - Step 1
* Reference code for adding handler function for server side event handling
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
* Section 8 - Step 2
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
* Section 8 - Step 3
* Character array to hold SSE message payload
* Function to be pasted in differt section of the main function (follow lab guide)
****************************************************************************************************************************************/

char http_response[64] = {0}; // character array for hold SSE message

sprintf( http_response, "The LED is turned ON." );  // if cm4_led_value == CYBSP_LED_STATE_ON

sprintf( http_response, "The LED is turned OFF." ); // else condition

// Send message payload to SSE stream function (included in http_server.c)
send_http_stream(&http_response);



