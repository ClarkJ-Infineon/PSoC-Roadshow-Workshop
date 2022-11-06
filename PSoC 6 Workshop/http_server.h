/*
 * http_server.h
 *
 *      Author: jarvisc
 */

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "cy_http_server.h"
#include "cy_nw_helper.h"
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

void configure_http_server(void);


#endif /* HTTP_SERVER_H_ */
