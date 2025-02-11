import os
import asyncio
import logging
import signal
from contextlib import asynccontextmanager
from typing import Dict, Set
import socket

logging.basicConfig(level=logging.INFO,
                   format='%(asctime)s - %(levelname)s - %(message)s')

class ChallengeServer:
    def __init__(self, host: str = '0.0.0.0'):
        self.host = host
        self.port = int(os.getenv('PORT', 9000))
        self.max_clients = int(os.getenv('MAX_CLIENTS', 100))
        self.used_ports: Set[int] = set()
        self.base_challenge_port = 10000
        self.max_challenge_port = 11000
        self.active_challenges: Dict[int, asyncio.subprocess.Process] = {}
        self.server = None
        self.tasks = set()

    async def get_available_port(self) -> int:
        for port in range(self.base_challenge_port, self.max_challenge_port):
            if port not in self.used_ports:
                try:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.bind(('127.0.0.1', port))
                    sock.close()
                    self.used_ports.add(port)
                    return port
                except OSError:
                    continue
        raise RuntimeError("No available ports")

    async def release_port(self, port: int):
        self.used_ports.discard(port)
        if port in self.active_challenges:
            del self.active_challenges[port]

    @asynccontextmanager
    async def challenge_process(self):
        challenge_port = await self.get_available_port()
        TIME_LIMIT = os.getenv('TIME_LIMIT', 60)
        try:
            MAX_MEM = int(os.getenv('MAX_MEM', '50')) * 1024 * 1024
        except ValueError:
            MAX_MEM = (50 * 1024 * 1024)
        MAX_CPU = os.getenv('MAX_CPU', 100)
        
        try:
            command = [
             '-R', '/srv:/', '--disable_clone_newnet', '--user', '999', '--group', '999', '--disable_proc', 
            '-Mo', '--env', f"PORT={challenge_port}", '--time_limit', f"{TIME_LIMIT}", '--rlimit_cpu', f"{MAX_CPU}", '--', '/app/chall'
            ]
            ## removed '--rlimit_mem', f"{MAX_MEM}" due to k8s permission issue [E][2025-02-04T20:37:10+0000][1] containSetLimits():181 setrlimit64(0, RLIMIT_MEMLOCK, 5368709120): Operation not permitted
            ## Confgiured it in k8s deployment file
            process = await asyncio.create_subprocess_exec(
                '/jail/nsjail',
                *command,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
                env={'PORT': str(challenge_port)}
            )
            self.active_challenges[challenge_port] = process
            await asyncio.sleep(0.1)
            yield challenge_port
        finally:
            if process.returncode is None:
                try:
                    process.terminate()
                    await process.wait()
                except:
                    pass
            await self.release_port(challenge_port)

    async def forward_data(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        try:
            while True:
                data = await reader.read(8192)
                if not data:
                    break
                writer.write(data)
                await writer.drain()
        except Exception as e:
            logging.error(f"Error in data forwarding: {e}")
        finally:
            try:
                writer.close()
                await writer.wait_closed()
            except:
                pass

    async def handle_client(self, client_reader: asyncio.StreamReader, client_writer: asyncio.StreamWriter):
        client_addr = client_writer.get_extra_info('peername')
        logging.info(f"New connection from {client_addr}")

        async with self.challenge_process() as challenge_port:
            try:
                binary_reader, binary_writer = await asyncio.open_connection(
                    'localhost', challenge_port
                )

                forward_tasks = [
                    asyncio.create_task(self.forward_data(client_reader, binary_writer)),
                    asyncio.create_task(self.forward_data(binary_reader, client_writer))
                ]
                
                for task in forward_tasks:
                    self.tasks.add(task)
                    task.add_done_callback(self.tasks.discard)

                await asyncio.wait(
                    forward_tasks,
                    return_when=asyncio.FIRST_COMPLETED
                )

                for task in forward_tasks:
                    if not task.done():
                        task.cancel()
                        try:
                            await task
                        except asyncio.CancelledError:
                            pass

            except Exception as e:
                logging.error(f"Error handling client {client_addr}: {e}")
            finally:
                try:
                    client_writer.close()
                    await client_writer.wait_closed()
                except:
                    pass
                logging.info(f"Connection closed for {client_addr}")

    async def cleanup(self):
        logging.info("Starting cleanup...")
        
        for task in self.tasks:
            task.cancel()
        
        if self.tasks:
            await asyncio.gather(*self.tasks, return_exceptions=True)
        
        if self.server:
            self.server.close()
            await self.server.wait_closed()
        
        for process in self.active_challenges.values():
            try:
                process.terminate()
                await process.wait()
            except:
                pass
        
        logging.info("Cleanup completed")

    async def serve_forever(self):
        self.server = await asyncio.start_server(
            self.handle_client,
            self.host,
            self.port
        )

        async with self.server:
            logging.info(f"Server running on {self.host}:{self.port} with max clients: {self.max_clients}")
            try:
                await self.server.serve_forever()
            except asyncio.CancelledError:
                logging.info("Server shutdown requested")
                await self.cleanup()

async def main():
    server = ChallengeServer()
    
    loop = asyncio.get_running_loop()
    for sig in (signal.SIGTERM, signal.SIGINT):
        loop.add_signal_handler(sig, lambda: asyncio.create_task(server.cleanup()))
    
    try:
        await server.serve_forever()
    except KeyboardInterrupt:
        logging.info("Keyboard interrupt received")
        await server.cleanup()

if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logging.info("Shutting down...")
