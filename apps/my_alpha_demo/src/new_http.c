

#include "new_common.h"
#include "new_http.h"
#include "new_pins.h"

/*
GET / HTTP/1.1
Host: 127.0.0.1
*/
/*
GET /test?a=5 HTTP/1.1
Host: 127.0.0.1
*/
/*
GET /test?a=Test%20with%20space HTTP/1.1
Host: 127.0.0.1
Connection: keep-alive
Cache-Control: max-age=0
*/
/*
GET /test?a=15&b=25 HTTP/1.1
Host: 127.0.0.1
Connection: keep-alive
*/

const char httpHeader[] = "HTTP/1.1 200 OK\nContent-type: " ;  // HTTP header
const char httpMimeTypeHTML[] = "text/html\n\n" ;              // HTML MIME type
const char httpMimeTypeText[] = "text/plain\n\n" ;           // TEXT MIME type
const unsigned char htmlHeader[] = "<!DOCTYPE html><html><body>" ;
const unsigned char htmlEnd[] = "</body></html>" ;

bool http_startsWith(const char *base, const char *substr) {
	while(*substr != 0) {
		if(*base != *substr)
			return false;
		if(*base == 0)
			return false;
		base++;
		substr++;
	}
	return true;
}
bool http_checkUrlBase(const char *base, const char *fileName) {
	while(*base != 0 && *base != '?' && *base != ' ') {
		if(*base != *fileName)
			return false;
		if(*base == 0)
			return false;
		base++;
		fileName++;
	}
	if(*fileName != 0)
		return false;
	return true;
}

void http_setup(char *o, const char *type){
	strcpy(o,httpHeader);
	strcat(o,type);
}
const char *http_checkArg(const char *p, const char *n) {
	while(1) {
		if(*n == 0 && (*p == 0 || *p == '='))
			return p;
		if(*p != *n)
			return 0;
		p++;
		n++;
	}
	return p;
}
void http_copyCarg(const char *at, char *to, int maxSize) {
	while(*at != 0 && *at != '&' && *at != ' ' && maxSize > 1) {
		*to = *at;
		to++;
		at++;
		maxSize--;
	}
	*to = 0;
}
bool http_getArg(const char *base, const char *name, char *o, int maxSize) {
	while(*base != '?') {
		if(*base == 0)
			return 0;
		base++;
	}
	base++;
	while(*base) {
		const char *at = http_checkArg(base,name);
		if(at) {
			at++;
			http_copyCarg(at,o,maxSize);
			return 1;
		}
		while(*base != '&') {
			if(*base == 0) {
				return 0;
			}
			base++;
		}
		base++;
	}
	return 0;
}

const char *htmlIndex = "<select name=\"cars\" id=\"cars\">\
<option value=\"1\">qqqqqq</option>\
<option value=\"2\">qqq</option>\
</select>";
//const char *htmlPinRoles = "<option value=\"1\">Relay</option>\
//<option value=\"2\">Relay_n</option>\
//<option value=\"3\">Button</option>\
//<option value=\"4\">Button_n</option>\
//<option value=\"5\">LED</option>\
//<option value=\"6\">LED_n</option>\
//</select>";
const char *htmlReturnToMenu = "<a href=\"index\">Return to menu</a>";;

const char *htmlPinRoleNames[] = {
	" ",
	"Rel",
	"Rel_n",
	"Btn",
	"Btn_n",
	"LED",
	"LED_n",
	"e",
	"e",
	"e",
};

void setupAllWB2SPinsAsButtons() {
		PIN_SetPinRoleForPinIndex(6,IOR_Button);
		PIN_SetPinChannelForPinIndex(6,1);

		PIN_SetPinRoleForPinIndex(7,IOR_Button);
		PIN_SetPinChannelForPinIndex(7,1);

		PIN_SetPinRoleForPinIndex(8,IOR_Button);
		PIN_SetPinChannelForPinIndex(8,1);

		PIN_SetPinRoleForPinIndex(23,IOR_Button);
		PIN_SetPinChannelForPinIndex(23,1);

		PIN_SetPinRoleForPinIndex(24,IOR_Button);
		PIN_SetPinChannelForPinIndex(24,1);

		PIN_SetPinRoleForPinIndex(26,IOR_Button);
		PIN_SetPinChannelForPinIndex(26,1);

		PIN_SetPinRoleForPinIndex(27,IOR_Button);
		PIN_SetPinChannelForPinIndex(27,1);
}



void HTTP_ProcessPacket(const char *recvbuf, char *outbuf) {
	int i, j;
	char tmpA[64];
	char tmpB[32];
	char tmpC[32];

	const char *urlStr = recvbuf + 5;
	if(http_startsWith(recvbuf,"GET")) {
		printf("HTTP request\n");
	} else {
		printf("Other request\n");
	}
	http_getArg(urlStr,"a",tmpA,sizeof(tmpA));
	http_getArg(urlStr,"b",tmpB,sizeof(tmpB));
	http_getArg(urlStr,"c",tmpC,sizeof(tmpC));
	if(http_checkUrlBase(urlStr,"about")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"About us page.");
		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"cfg_wifi_set")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"Please wait for module to reset...");
		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"cfg_wifi")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"<h1>OpenBK2731T</h1>");
		strcat(outbuf,"<form action=\"/cfg_wifi_set\">\
			  <label for=\"ssid\">SSID:</label><br>\
			  <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"\"><br>\
			  <label for=\"pass\">Pass:</label><br>\
			  <input type=\"text\" id=\"pass\" name=\"pass\" value=\"\"><br><br>\
			  <input type=\"submit\" value=\"Submit\">\
			</form> ");


		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"cfg")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"<h1>Test</h1>");
		strcat(outbuf,"<form action=\"cfg_pins\"><input type=\"submit\" value=\"Configure Module\"/></form>");
		strcat(outbuf,"<form action=\"cfg_wifi\"><input type=\"submit\" value=\"Configure WiFi\"/></form>");


		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"setWB2SInputs")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);

		setupAllWB2SPinsAsButtons();

		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,"Set all inputs for dbg .");
		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"setAllInputs")) {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		// it breaks UART pins as well, omg!
		for(i = 0; i < GPIO_MAX; i++) {
			PIN_SetPinRoleForPinIndex(i,IOR_Button);
			PIN_SetPinChannelForPinIndex(i,1);
		}
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,"Set all inputs for dbg .");
		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"cfg_pins")) {
		int iChanged = 0;
		int iChangedRequested = 0;

		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"<h1>Test</h1>");
		for(i = 0; i < GPIO_MAX; i++) {
			sprintf(tmpA, "%i",i);
			if(http_getArg(recvbuf,tmpA,tmpB,sizeof(tmpB))) {
				int role;
				int pr;

				iChangedRequested++;

				role = atoi(tmpB);

				pr = PIN_GetPinRoleForPinIndex(i);
				if(pr != role) {
					PIN_SetPinRoleForPinIndex(i,role);
					iChanged++;
				}
			}
			sprintf(tmpA, "r%i",i);
			if(http_getArg(recvbuf,tmpA,tmpB,sizeof(tmpB))) {
				int rel;
				int prevRel;

				iChangedRequested++;

				rel = atoi(tmpB);

				prevRel = PIN_GetPinChannelForPinIndex(i);
				if(prevRel != rel) {
					PIN_SetPinChannelForPinIndex(i,rel);
					iChanged++;
				}
			}
		}
		if(iChangedRequested>0) {
			sprintf(tmpA, "Pins update - %i reqs, %i changed!<br><br>",iChangedRequested,iChanged);
			strcat(outbuf,tmpA);
		}
	//	strcat(outbuf,"<button type=\"button\">Click Me!</button>");
		strcat(outbuf,"<form action=\"cfg_pins\">");
		for( i = 0; i < GPIO_MAX; i++) {
			int si, ch;
			si = PIN_GetPinRoleForPinIndex(i);
			ch = PIN_GetPinChannelForPinIndex(i);
			sprintf(tmpA, "P%i ",i);
			strcat(outbuf,tmpA);
			sprintf(tmpA, "<select name=\"%i\">",i);
			strcat(outbuf,tmpA);
			for(j = 0; j < IOR_Total_Options; j++) {
				if(j == si) {
					sprintf(tmpA, "<option value=\"%i\" selected>%s</option>",j,htmlPinRoleNames[j]);
				} else {
					sprintf(tmpA, "<option value=\"%i\">%s</option>",j,htmlPinRoleNames[j]);
				}
				strcat(outbuf,tmpA);
			}
			strcat(outbuf, "</select>");
			if(ch == 0) {
				tmpB[0] = 0;
			} else {
				sprintf(tmpB,"%i",ch);
			}
			sprintf(tmpA, "<input name=\"r%i\" type=\"text\" value=\"%s\"/>",i,tmpB);
			strcat(outbuf,tmpA);
			strcat(outbuf,"<br>");
		}
		strcat(outbuf,"<input type=\"submit\" value=\"Save\"/></form>");

		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else if(http_checkUrlBase(urlStr,"index")) {
		int relayFlags = 0;
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"<style>.r { background-color: red; } .g { background-color: green; }</style>");
		strcat(outbuf,"<h1>Test</h1>");
		if(http_getArg(urlStr,"tgl",tmpA,sizeof(tmpA))) {
			j = atoi(tmpA);
			sprintf(tmpA,"<h3>Toggled %i!</h3>",j);
			strcat(outbuf,tmpA);
			CHANNEL_Toggle(j);
		}

		for(i = 0; i < GPIO_MAX; i++) {
			int role = PIN_GetPinRoleForPinIndex(i);
			int ch = PIN_GetPinChannelForPinIndex(i);
			if(role == IOR_Relay || role == IOR_Relay_n || role == IOR_LED || role == IOR_LED_n) {
				BIT_SET(relayFlags,ch);
			}
		}
		for(i = 0; i < CHANNEL_MAX; i++) {
			if(BIT_CHECK(relayFlags,i)) {
				const char *c;
				if(CHANNEL_Check(i)) {
					c = "r";
				} else {
					c = "g";
				}
				strcat(outbuf,"<form action=\"index\">");
				sprintf(tmpA,"<input type=\"hidden\" name=\"tgl\" value=\"%i\">",i);
				strcat(outbuf,tmpA);
				sprintf(tmpA,"<input class=\"%s\" type=\"submit\" value=\"Toggle %i\"/></form>",c,i);
				strcat(outbuf,tmpA);
			}
		}
	//	strcat(outbuf,"<button type=\"button\">Click Me!</button>");
		strcat(outbuf,"<form action=\"cfg\"><input type=\"submit\" value=\"Config\"/></form>");
		strcat(outbuf,"<form action=\"about\"><input type=\"submit\" value=\"About\"/></form>");


		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	} else {
		http_setup(outbuf, httpMimeTypeHTML);
		strcat(outbuf,htmlHeader);
		strcat(outbuf,"Not found.");
		strcat(outbuf,htmlReturnToMenu);
		strcat(outbuf,htmlEnd);
	}
}