import asyncio
import signal
import ssl
import threading
import websockets
from sys import argv


url = argv[1]  # 1st argument is wss:// URL for game server
certificate = argv[2]  # 2nd argument is path to custom certificate required for localhost self-signed PEM

# Enables TLS features
tls_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
tls_context.keylog_filename = "keylog.pem"  # Wireshark will need that for debugging
tls_context.load_verify_locations(certificate)  # localhost requires self-signed PEM cert

console_lock = asyncio.Lock()  # To not write a received message and read a sending message simultaneously
input_required = threading.Event()  # To notify the task sending message that it should prompt for message into stdin


async def send_messages(connection: websockets.WebSocketClientProtocol):
    """Wait for the console to be available then read input message from stdin and send it to server."""

    while connection.open:  # Waits for next Ctrl+C until connection has been closed
        try:
            input_required.wait()  # Waits for Ctrl+C

            async with console_lock:  # Waits for the console to be available before reading stdin
                rptl_message = input("Send: ")

            await connection.send(rptl_message)

            input_required.clear()
        except websockets.ConnectionClosedError as closed:  # Connection will end up closed
            print(f"Connection was closed: {closed.reason}")


async def receive_messages(connection: websockets.WebSocketClientProtocol):
    """Listen for next RPTL message and wait for the console to be available then displays it to stdout."""

    while connection.open:  # Waits for next RPTL message until connection has been closed
        try:
            rptl_message = await connection.recv()

            async with console_lock:  # Waits for the console to be available before writing to stdout
                print(f"Recv: {rptl_message}")
        except websockets.ConnectionClosedError as closed:  # Connection will end up closed
            print(f"Connection was closed: {closed.reason}")


async def rptl_connection(connection: websockets.WebSocketClientProtocol):
    """Listen for RPTL message on given connection and displays them to stdout.
    At SIGINT, messages no longer displayed until RPTL message has been written to stdin and sent to server."""

    receiving = asyncio.create_task(receive_messages(connection))
    sending = asyncio.create_task(send_messages(connection))

    await asyncio.gather(receiving, sending)


async def websocket_connection(host_url: str, security_context: ssl.SSLContext):
    # Wait for WSS client connection to be made
    connection = await websockets.connect(uri=host_url, ssl=security_context)

    print(f"Connected to {host_url}.")

    # Once connection is made, starts RPTL protocol until disconnection
    await rptl_connection(connection)

signal.signal(signal.SIGINT, lambda _, __: input_required.set())  # When Ctrl+C is triggered, prompt for RPTL message
asyncio.run(websocket_connection(url, tls_context))