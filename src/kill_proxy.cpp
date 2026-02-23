#include <cstdint>
#include <thread>

#include "net/client.hpp"
#include "net/conn.hpp"
#include "net/server.hpp"

int main(int argc, char* argv[])
{
	auto server = net::client("localhost", 7777);
	auto proxy = net::server("localhost", 8888);

	proxy.bind();
	proxy.listen(1);

	auto server_conn = server.connect();
	auto proxy_conn = proxy.accept();

	auto handle_loop = [](net::conn& src, net::conn& dst) {
		for (;;) {
			src.handle([&dst] (uint8_t id, std::span<uint8_t> payload) {
				dst.send_packet(id, payload);
			});
		}
	};

	std::thread server_thread(handle_loop, std::ref(server_conn), std::ref(proxy_conn));
	std::thread proxy_thread(handle_loop, std::ref(proxy_conn), std::ref(server_conn));

	while (1) {
	}

	return 0;
}