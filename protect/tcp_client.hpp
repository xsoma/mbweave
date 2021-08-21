#include <cstdint>
#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "winhttp.lib")

class tcp_client
{
protected:
	int last_error;
	SOCKET socket;
	bool encryption_using, initialized_from_socket;
	const std::vector<uint8_t> encryption_key;
	const size_t encryption_key_size;
public:
	__forceinline int get_last_error() const
	{
		return this->last_error;
	}
	__forceinline int is_encryption_using() const
	{
		return this->encryption_using;
	}
	__forceinline SOCKET get_socket() const
	{
		return this->socket;
	}
	__forceinline tcp_client(const char* ip, const u_short port, const std::vector<uint8_t>& encryption_key = {}) : encryption_key(encryption_key), encryption_key_size(encryption_key.size())
	{
		this->encryption_using = (this->encryption_key_size > 0);
		this->initialized_from_socket = false;
		WSADATA wsaData;
		if ((this->last_error = ::WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
			return;
		this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(port);
		addr.sin_addr.S_un.S_addr = ::inet_addr(ip);
		this->last_error = ::connect(this->socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
	}
	__forceinline tcp_client(const SOCKET socket, const std::vector<uint8_t>& encryption_key = {}) : socket(socket), encryption_key(encryption_key), encryption_key_size(encryption_key.size())
	{
		this->encryption_using = (this->encryption_key_size > 0);
		this->initialized_from_socket = true;
	}
	__forceinline ~tcp_client()
	{
		this->close();
	}
	__forceinline int send(const std::vector<uint8_t>& msg) const
	{
		if (this->encryption_using)
		{
			std::vector<uint8_t> encrypted_msg = msg;
			const size_t encrypted_msg_size = encrypted_msg.size();
			for (size_t i = 0; i < encrypted_msg_size; ++i)
				encrypted_msg[i] ^= this->encryption_key[i % this->encryption_key_size];
			return ::send(this->socket, reinterpret_cast<const char*>(encrypted_msg.data()), encrypted_msg.size(), 0);
		}
		return ::send(this->socket, reinterpret_cast<const char*>(msg.data()), msg.size(), 0);
	}
	__forceinline int send(const std::string& msg) const
	{
		return this->send(std::vector<uint8_t>(msg.begin(), msg.end()));
	}
	__forceinline std::vector<uint8_t> recv() const
	{
		std::vector<uint8_t> received;
		received.resize(65536);
		received.resize(::recv(this->socket, reinterpret_cast<char*>(&received[0]), received.size(), 0));
		if (this->encryption_using)
		{
			const size_t received_size = received.size();
			for (size_t i = 0; i < received_size; ++i)
				received[i] ^= this->encryption_key[i % this->encryption_key_size];
		}
		return received;
	}
	__forceinline std::string recv_str() const
	{
		const std::vector<uint8_t>& received = this->recv();
		return std::string(received.begin(), received.end());
	}
	__forceinline int close() const
	{
		const int last_error_a = ::closesocket(this->socket),
			last_error_b = 0;
		if (this->initialized_from_socket == false)
			::WSACleanup();
		return (last_error_a == 0) ? last_error_b : last_error_a;
	}
};
