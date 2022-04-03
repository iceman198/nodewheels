
const char MAIN_page[] PROGMEM = R"=====(
<html>
	<head>
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0">
		<title>DWheels Controler</title>
		<script type = "text/javascript">
			var ws;
			function socketConnect() {
				ws = new WebSocket("ws://192.168.4.1:8181");
				if ("WebSocket" in window) {
					ws.onopen = function() {
						// Web Socket is connected, send data using send()
						//ws.send("Message to send");
					};
					
					ws.onmessage = function (evt) { 
						var received_msg = evt.data;
						console.log("Message received: " + received_msg);
						if (received_msg.indexOf("VOLTAGE") >= -1) {
							document.getElementById("voltage").innerHTML = received_msg;
						}
						//alert("Message is received...");
					};
					
					ws.onclose = function() { 
						// websocket is closed.
						//alert("Connection is closed..."); 
					};
				} else {
					// The browser doesn't support WebSocket
					alert("WebSocket NOT supported by your Browser!");
				}
			}
			function socketDisconnect() {
				ws.close();
			}

			socketConnect();

			function socketSend() {
				ws.send('testing');
			}

			function goForward() {
				ws.send('goForward');
			}
			function goBackward() {
				ws.send('goBackward');
			}
			function goLeft() {
				ws.send('goLeft');
			}
			function goRight() {
				ws.send('goRight');
			}
			function goStop() {
				ws.send('goStop');
			}

			function speedFast() {
				ws.send('speedFast');
			}
			function speedMedium() {
				ws.send('speedMedium');
			}
			function speedSlow() {
				ws.send('speedSlow');
			}
		</script>
	</head>
	<body>
		<h1>CONTROLLER</h1>
		<button onclick="socketConnect()">Connect</button>
		<button onclick="socketDisconnect()">Disconnect</button>
		<button onclick="socketSend()">Send test</button>
		<hr>
		<table border=1>
			<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td><button style="width:100%;" onclick="goForward()">Forward</button></td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td></tr>
			<tr><td><button style="width:100%;" onclick="goLeft()">Left</button></td><td><button style="width:100%;" onclick="goStop()">STOP</button></td><td><button style="width:100%;" onclick="goRight()">Right</button></td></tr>
			<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td><button style="width:100%;" onclick="goBackward()">Backwards</button></td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td></tr>
		</table>
		<hr>
		<form action="">
			<input type="radio" onclick="speedSlow()" name="speed" value="slow" checked>Slow 
			<input type="radio" onclick="speedMedium()" name="speed" value="fast">Medium
			<input type="radio" onclick="speedFast()" name="speed" value="fast">Fast
		</form>
		<hr>
		<div id="voltage"></div>
	</body>
</html>
)=====";