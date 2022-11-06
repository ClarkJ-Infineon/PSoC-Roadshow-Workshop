/*
 * html_web_page.h
 *
 *      Author: jarvisc
 */

#ifndef HTML_WEB_PAGE_H_
#define HTML_WEB_PAGE_H_

/* HTML Device Data Page - LED Control  */
#define SOFTAP_DEVICE_DATA \
        "<!DOCTYPE html> " \
        "<html>" \
        "<head><title>Wi-Fi Web Server Demo LED Control</title></head>" \
        "<body>" \
            "<h1 style=\"text-align: left\" > PSoC 6 Technical Workshop </h1>" \
            "<br><br>" \
            "<p>Click to enable or disable the LED</p>" \
            "<button type=\"button\" onclick=\"enable_led()\" id=\"enable_btn\">    LED On    </button> " \
            "<button type=\"button\" onclick=\"disable_led()\" id=\"disable_btn\">    LED Off    </button> " \
            "<br><br>" \
            "<br><br>" \
            "<div id=\"device_data\" value=\"100\"></div>" \
            "<script>" \
				"function enable_led() { " \
					" var xhttp = new XMLHttpRequest(); "\
					" xhttp.onreadystatechange = function() { "\
						"   if (this.readyState === 4 && this.status == 200) { " \
							"   } "\
						"}; "\
						"xhttp.open(\"POST\", \"/\", true); "\
						"xhttp.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\"); "\
						"xhttp.send(\"Enable\");"\
				"} "\
				"function disable_led() { " \
					"  var xhttp = new XMLHttpRequest(); " \
					"  xhttp.onreadystatechange = function() { " \
						"    if (this.readyState === 4 && this.status == 200) { " \
						"    } " \
						"  }; " \
						"xhttp.open(\"POST\", \"/\", true); " \
						"xhttp.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\"); "\
						"xhttp.send(\"Disable\"); " \
				"} "\
				"if(typeof(EventSource) !== \"undefined\") {" \
					"var source = new EventSource(\"/events\");" \
					"source.onmessage = function(event) {" \
						"document.getElementById(\"device_data\").innerHTML = event.data;" \
						"  };" \
				"} else {" \
					"document.getElementById(\"device_data\").innerHTML = \"Sorry, your browser does not support server-sent events...\";" \
				"}" \
        "</script>" \
        "</body>" \
        "</html>"

#endif /* HTML_WEB_PAGE_H_ */

